#include <Arduino.h>
#include <M5Core2.h>
#include "opm.h"

#define INTERNAL_BUTTON
#define EXTERNAL_BUTTON
int last_value = 0;
  int cur_value = 0;
void setup()
{
  M5.begin();
  
  #ifdef EXTERNAL_BUTTON
  pinMode(32, INPUT);
  #endif

  M5.Lcd.drawJpg(opm, sizeof(opm), 75, 30);
  delay(500);
  M5.Lcd.clear();
  
  M5.Lcd.setCursor(90, 110);
  M5.Lcd.setTextSize(2);
  M5.Lcd.print("Hello World !");
  
}

void loop()
{
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
}