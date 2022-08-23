#include <Arduino.h>
#include <M5Core2.h>
#include "opm.h"
#include "plus.h"
#include "minus.h"
#include "UNIT_ENV.h"
#include <time.h>
#define INTERNAL_BUTTON
//#define EXTERNAL_BUTTON
//#define VIBRATOR

#ifdef VIBRATOR
#define VIBRATOR_PIN 32
#define VIBRATOR_PWM_FREQ 10000
#define VIBRATOR_PWM_CHANNEL 0
#define VIBRATOR_PWM_RESOLUTION 10
#endif

struct sensorData_t{
 // float timestamp;
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
RTC_TimeTypeDef TimeStruct;
RTC_DateTypeDef DateStruct;
sensorData_t data;

float tmp = 0.0;
float hum = 0.0;
float pressure = 0.0;
unsigned long ts;
unsigned long last_update;

#ifdef vibrator
void vibratorSetup() {
    ledcSetup(VIBRATOR_PWM_CHANNEL, VIBRATOR_PWM_FREQ, VIBRATOR_PWM_RESOLUTION);
    ledcAttachPin(VIBRATOR_PIN, VIBRATOR_PWM_CHANNEL);
}

void vibratorSet(uint32_t duty) {
    ledcWrite(VIBRATOR_PWM_CHANNEL, duty);
}
#endif

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

  TimeStruct.Hours   = 16;    //Set the specific time of the real-time clock structure.
  TimeStruct.Minutes = 31;
  TimeStruct.Seconds = 50;
  M5.Rtc.SetTime(&TimeStruct);    //Writes the set time to the real-time clock.
  DateStruct.WeekDay = 3;
  DateStruct.Month = 8;
  DateStruct.Date = 24;
  DateStruct.Year = 2022;
  M5.Rtc.SetDate(&DateStruct);
  
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
    M5.Rtc.GetTime(&TimeStruct);  
    M5.Lcd.drawString(F("TEMPERATURE"),30,25,2);
    M5.Lcd.drawString(F("°C"),250,15,2);
    M5.Lcd.drawString(F("HUMIDITY"),30,85,2);
    M5.Lcd.drawString(F("%"),250,75,2);
    M5.Lcd.drawString(F("Pressure"),30,132,2);
    M5.Lcd.drawString(F("Pa"),250,135,2);

    M5.Lcd.drawFloat(tmp, 1, 140, 15, 6);
    M5.Lcd.drawFloat(hum, 1, 140, 75, 6);
    M5.Lcd.drawFloat(pressure, 1, 140, 130, 4);
    M5.Lcd.setCursor(30, 175);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("Time: %02d : %02d : %02d",TimeStruct.Hours, TimeStruct.Minutes, TimeStruct.Seconds);
    M5.Lcd.setTextSize(1);

    data.temperature = tmp;
    data.humidity = hum;
    data.pressure = pressure;
    
    M5.Rtc.GetTime(&TimeStruct);
    M5.Rtc.GetDate(&DateStruct); 
    struct tm currTime;
    currTime.tm_year = DateStruct.Year - 1900;
    currTime.tm_mday = DateStruct.Date;
    currTime.tm_mon  = DateStruct.Month -1 ;
    currTime.tm_hour = TimeStruct.Hours - 2;
    currTime.tm_min  = TimeStruct.Minutes;
    currTime.tm_sec  = TimeStruct.Seconds;
    data.timestamp = mktime(&currTime);
    Serial.println(data.timestamp);


  }
}