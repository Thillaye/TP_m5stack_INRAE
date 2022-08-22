#include <Arduino.h>
#include <M5Core2.h>
#include "opm.h"
#include "plus.h"
#include "minus.h"

#define INTERNAL_BUTTON
//#define EXTERNAL_BUTTON
#define VIBRATOR

#ifdef VIBRATOR
#define VIBRATOR_PIN 32
#define VIBRATOR_PWM_FREQ 10000
#define VIBRATOR_PWM_CHANNEL 0
#define VIBRATOR_PWM_RESOLUTION 10
#endif

int last_value = 0;
int cur_value = 0;
int vibrator = 500;

void vibratorSetup() {
    ledcSetup(VIBRATOR_PWM_CHANNEL, VIBRATOR_PWM_FREQ, VIBRATOR_PWM_RESOLUTION);
    ledcAttachPin(VIBRATOR_PIN, VIBRATOR_PWM_CHANNEL);
}

void vibratorSet(uint32_t duty) {
    ledcWrite(VIBRATOR_PWM_CHANNEL, duty);
}

void setup()
{
  M5.begin();
  
  #ifdef EXTERNAL_BUTTON
  pinMode(32, INPUT);
  #endif

  #ifdef VIBRATOR
    vibratorSetup();
    vibratorSet(0);
  #endif

  M5.Lcd.drawJpg(opm, sizeof(opm), 75, 30);
  delay(500);
  M5.Lcd.clear();
  
  #ifdef VIBRATOR
  M5.Lcd.drawJpg(minus,sizeof(minus),38,210,0,0,0,0,JPEG_DIV_8);

	M5.Lcd.drawJpg(plus,sizeof(plus),250,210,0,0,0,0,JPEG_DIV_8);

  M5.Lcd.fillRoundRect(128,210,60,28,3,DARKGREY);
  M5.Lcd.fillRoundRect(0,0,320,180,3,BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.drawCentreString("STOP",160,215,2);
  #endif
  
 /* M5.Lcd.setCursor(90, 110);
  M5.Lcd.setTextSize(2);
  M5.Lcd.print("Hello World !"); */

  
  
}

void loop()
{
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
    
}