#include <Arduino.h>
#include <M5Core2.h>
#include "opm.h"

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
  // put your main code here, to run repeatedly:
}