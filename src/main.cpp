#include <Arduino.h>
#include <M5Core2.h>
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

void setup()
{
  M5.begin(true, false, true, false);
  
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

void loop()
{
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
}