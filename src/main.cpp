#include <Arduino.h>
#include <M5Core2.h>
#include "opm.h"

#define INTERNAL_BUTTON

void setup()
{
  M5.begin();

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
}