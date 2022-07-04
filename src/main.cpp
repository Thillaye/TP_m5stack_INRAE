#include <Arduino.h>
#include <M5Core2.h>
#include "UNIT_ENV.h"

#include "opm.h"

//#define INTERNAL_BUTTON
//#define EXTERNAL_BUTTON

SHT3X sht30;
QMP6988 qmp6988;

int last_value = 0;
int cur_value = 0;

float tmp = 0.0;
float hum = 0.0;
float pressure = 0.0;

unsigned long ts;
unsigned long last_update;


void setup()
{
  M5.begin();
  
  #ifdef EXTERNAL_BUTTON
  pinMode(32, INPUT);
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
      M5.Lcd.clear();
      M5.Lcd.setCursor(60, 110);
      M5.Lcd.setTextColor(YELLOW);
      M5.Lcd.print("A Button pressed");
    }
	if (M5.BtnB.wasPressed())
    {
      M5.Lcd.clear();
      M5.Lcd.setCursor(60, 110);
      M5.Lcd.setTextColor(RED);
      M5.Lcd.print("B Button pressed");
    }
	if (M5.BtnC.wasPressed())
    {
      M5.Lcd.clear();
      M5.Lcd.setCursor(60, 110);
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.print("C Button pressed");
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
	M5.Lcd.drawString(F("TEMPERATURE"),30,30,2);
	M5.Lcd.drawString(F("°C"),250,20,2);
	M5.Lcd.drawString(F("HUMIDITY"),30,100,2);
	M5.Lcd.drawString(F("%"),250,100,2);
	M5.Lcd.drawString(F("Pressure"),30,157,2);
	M5.Lcd.drawString(F("Pa"),250,160,2);

	M5.Lcd.drawFloat(tmp, 1, 140, 20, 6);
	M5.Lcd.drawFloat(hum, 1, 140, 90, 6);
	M5.Lcd.drawFloat(pressure, 1, 140, 155, 4);
  }
}