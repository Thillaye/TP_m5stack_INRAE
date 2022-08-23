#include <M5Core2.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "opm.h"
#include "plus.h"
#include "minus.h"
#include "UNIT_ENV.h"

#define INTERNAL_BUTTON
//#define EXTERNAL_BUTTON
#define VIBRATOR

#ifdef VIBRATOR
#define VIBRATOR_PIN 32
#define VIBRATOR_PWM_FREQ 10000
#define VIBRATOR_PWM_CHANNEL 0
#define VIBRATOR_PWM_RESOLUTION 10
#endif

struct sensorData_t{
  float temperature;
  float humidity;
  int pressure;
  unsigned long timestamp;
};

int last_value = 0;
int cur_value = 0;
int vibrator = 500;
SHT3X sht30;
QMP6988 qmp6988;
sensorData_t data;

float tmp = 0.0;
float hum = 0.0;
float pressure = 0.0;
unsigned long ts;
unsigned long last_update;
byte raw[30];

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8]={ 0};
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]={ 0x3d ,0x1a, 0x14, 0x2f, 0x40, 0xd5, 0xa4, 0x7c };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0x16, 0xAE, 0xDC, 0x29, 0x80, 0x22, 0xE9, 0x30, 0x33, 0x8D, 0x24, 0xFE, 0x82, 0x0E, 0x42, 0x3A };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static uint8_t mydata[] = "Hello, world!";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

// Pin mapping
// https://docs.m5stack.com/en/module/lora868
const lmic_pinmap lmic_pins = {
  .nss = 33,                       
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 26,                       
  .dio = {36, 35, LMIC_UNUSED_PIN}, 
};

void printHex2(unsigned v) {
  v &= 0xff;
  if (v < 16)
    Serial.print('0');
  Serial.print(v, HEX);
}

void do_send(osjob_t* j){
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
      Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
      // Prepare upstream data transmission at the next possible time.
      LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
      Serial.println(F("Packet queued"));
  }
  // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch(ev) {
      case EV_SCAN_TIMEOUT:
          Serial.println(F("EV_SCAN_TIMEOUT"));
          break;
      case EV_BEACON_FOUND:
          Serial.println(F("EV_BEACON_FOUND"));
          break;
      case EV_BEACON_MISSED:
          Serial.println(F("EV_BEACON_MISSED"));
          break;
      case EV_BEACON_TRACKED:
          Serial.println(F("EV_BEACON_TRACKED"));
          break;
      case EV_JOINING:
          Serial.println(F("EV_JOINING"));
          break;
      case EV_JOINED:
          Serial.println(F("EV_JOINED"));
          {
            u4_t netid = 0;
            devaddr_t devaddr = 0;
            u1_t nwkKey[16];
            u1_t artKey[16];
            LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
            Serial.print("netid: ");
            Serial.println(netid, DEC);
            Serial.print("devaddr: ");
            Serial.println(devaddr, HEX);
            Serial.print("AppSKey: ");
            for (size_t i=0; i<sizeof(artKey); ++i) {
              if (i != 0)
                Serial.print("-");
              printHex2(artKey[i]);
            }
            Serial.println("");
            Serial.print("NwkSKey: ");
            for (size_t i=0; i<sizeof(nwkKey); ++i) {
                    if (i != 0)
                            Serial.print("-");
                    printHex2(nwkKey[i]);
            }
            Serial.println();
          }
          // Disable link check validation (automatically enabled
          // during join, but because slow data rates change max TX
   // size, we don't use it in this example.
          LMIC_setLinkCheckMode(0);
          break;
      /*
      || This event is defined but not used in the code. No
      || point in wasting codespace on it.
      ||
      || case EV_RFU1:
      ||     Serial.println(F("EV_RFU1"));
      ||     break;
      */
      case EV_JOIN_FAILED:
          Serial.println(F("EV_JOIN_FAILED"));
          break;
      case EV_REJOIN_FAILED:
          Serial.println(F("EV_REJOIN_FAILED"));
          break;
      case EV_TXCOMPLETE:
          Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
          if (LMIC.txrxFlags & TXRX_ACK)
            Serial.println(F("Received ack"));
          if (LMIC.dataLen) {
            Serial.print(F("Received "));
            Serial.print(LMIC.dataLen);
            Serial.println(F(" bytes of payload"));
          }
          // Schedule next transmission
          os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
          break;
      case EV_LOST_TSYNC:
          Serial.println(F("EV_LOST_TSYNC"));
          break;
      case EV_RESET:
          Serial.println(F("EV_RESET"));
          break;
      case EV_RXCOMPLETE:
          // data received in ping slot
          Serial.println(F("EV_RXCOMPLETE"));
          break;
      case EV_LINK_DEAD:
          Serial.println(F("EV_LINK_DEAD"));
          break;
      case EV_LINK_ALIVE:
          Serial.println(F("EV_LINK_ALIVE"));
          break;
      /*
      || This event is defined but not used in the code. No
      || point in wasting codespace on it.
      ||
      || case EV_SCAN_FOUND:
      ||    Serial.println(F("EV_SCAN_FOUND"));
      ||    break;
      */
      case EV_TXSTART:
          Serial.println(F("EV_TXSTART"));
          break;
      case EV_TXCANCELED:
          Serial.println(F("EV_TXCANCELED"));
          break;
      case EV_RXSTART:
          /* do not print anything -- it wrecks timing */
          break;
      case EV_JOIN_TXCOMPLETE:
          Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
          break;
       default:
          Serial.print(F("Unknown event: "));
          Serial.println((unsigned) ev);
          break;
  }
}

void vibratorSetup() {
  ledcSetup(VIBRATOR_PWM_CHANNEL, VIBRATOR_PWM_FREQ, VIBRATOR_PWM_RESOLUTION);
  ledcAttachPin(VIBRATOR_PIN, VIBRATOR_PWM_CHANNEL);
}

void vibratorSet(uint32_t duty) {
  ledcWrite(VIBRATOR_PWM_CHANNEL, duty);
}

void encodestruct(struct sensorData_t data,byte bytes[2]) {
  signed short temp = data.temperature *100;
  bytes[0] = (temp>>8) & 0xFF;
  bytes[1] = temp  & 0xFF;
  signed short hum = data.humidity *100;
  bytes[2] = (hum>>8) & 0xFF;
  bytes[3] = hum  & 0xFF;
  signed pres = data.pressure ;
  Serial.println(pres);
  bytes[4] = (pres>>16) & 0xFF;
  bytes[5] = (pres>>8) & 0xFF;
  bytes[6] = pres  & 0xFF;
  unsigned long ts = data.timestamp;
  bytes[7] = (ts>>8) & 0xFF;
  bytes[8] = ts  & 0xFF;
};

void setup() {
  M5.begin(true, false, true, false);

  // LMIC init
  os_init();

  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  // Start job (sending automatically starts OTAA too)
  do_send(&sendjob);

  #ifdef EXTERNAL_BUTTON
  pinMode(32, INPUT);
  #endif

  #ifdef VIBRATOR
  vibratorSetup();
  vibratorSet(0);
  #endif

  Wire.begin(14,13);
  qmp6988.init();

  M5.Lcd.drawJpg(opm, sizeof(opm), 75, 30);
  delay(500);
  M5.Lcd.clear();
    
 /* M5.Lcd.setCursor(90, 110);
  M5.Lcd.setTextSize(2);
  M5.Lcd.print("Hello World !"); */
}

void loop() {
  ts = millis();
  M5.update();

  #ifdef INTERNAL_BUTTON
	if (M5.BtnA.wasPressed())
	{
		Serial.println("A button pressed");
    #ifdef VIBRATOR
		if (vibrator >=20)
		{
			vibrator -= 20;
			vibratorSet(vibrator);
		}
		#endif
	}
	if (M5.BtnB.wasPressed())
	{
		Serial.println("B Button pressed");
		#ifdef VIBRATOR
		vibrator = 500;
		vibratorSet(0);
		#endif
	}
	if (M5.BtnC.wasPressed())
	{
		Serial.println("C Button pressed");
    #ifdef VIBRATOR
		if (vibrator <=980)
		{
			vibrator += 20;
			vibratorSet(vibrator);
		}
		#endif
	}
  #endif

  #ifdef EXTERNAL_BUTTON
  cur_value = digitalRead(32); // read the value of BUTTON.  读取22
  if(cur_value != last_value){
    if(cur_value==0){
      M5.Lcd.clear();
      M5.Lcd.setTextColor(RED);
      M5.Lcd.setCursor(0,45); M5.Lcd.print("Button red"); // display the status
      M5.Lcd.setCursor(0,85); M5.Lcd.print("press");
    }
    else{
      M5.Lcd.clear();
      M5.Lcd.setTextColor(RED);
      M5.Lcd.setCursor(0,45); M5.Lcd.print("Button red"); // display the status
      M5.Lcd.setCursor(0,85); M5.Lcd.print("release");
    }
    last_value = cur_value;
  }
  #endif
  if (ts > last_update + 2000) {
    last_update = ts;
    pressure = qmp6988.calcPressure();
    if(sht30.get()==0){
      tmp = sht30.cTemp;
      hum = sht30.humidity;
    }else{
      tmp=0,hum=0;
      }
    #ifdef VIBRATOR
    M5.Lcd.drawJpg(minus,sizeof(minus),38,210,0,0,0,0,JPEG_DIV_8);

    M5.Lcd.drawJpg(plus,sizeof(plus),250,210,0,0,0,0,JPEG_DIV_8);

    M5.Lcd.fillRoundRect(128,210,60,28,3,DARKGREY);
    M5.Lcd.fillRoundRect(0,0,320,180,3,BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.drawCentreString("STOP",160,215,2);
    #endif

    M5.Lcd.drawString(F("TEMPERATURE"),30,25,2);
    M5.Lcd.drawString(F("°C"),250,15,2);
    M5.Lcd.drawString(F("HUMIDITY"),30,85,2);
    M5.Lcd.drawString(F("%"),250,75,2);
    M5.Lcd.drawString(F("Pressure"),30,132,2);
    M5.Lcd.drawString(F("Pa"),250,135,2);
    M5.Lcd.drawString(F("Ts"),30,167,2);
    M5.Lcd.drawString(F("ms"),250,165,2);

    M5.Lcd.drawFloat(tmp, 1, 140, 15, 6);
    M5.Lcd.drawFloat(hum, 1, 140, 75, 6);
    M5.Lcd.drawFloat(pressure, 1, 140, 130, 4);
    M5.Lcd.drawFloat(last_update, 1, 140, 165, 2);

    data.temperature = tmp;
    data.humidity = hum;
    data.pressure = pressure;
    data.timestamp = last_update;
    encodestruct(data,raw);
  }
os_runloop_once();
}