#include "PWM16bitSetupFreq.h"

//#define pinRead A0

int xungPWM = 0;
int i = 10;

void setup() {
  Serial.begin(9600);
  pinMode(5, OUTPUT);  // Chân 6 là chân OC3A (PWM)
  pinMode(3, OUTPUT);  // Chân 7 là chân OC3B (PWM)
  pinMode(2, OUTPUT);  // Chân 8 là chân OC3C (PWM)
  PWM_16Bit_SetupFrequency(3, 2000);
  xungPWM = 0;
  digitalWrite(2, 0);
  delay(200);
}

void loop() {
  // unsigned long readAnalog = analogRead(pinRead);
  // int xungPWM = (readAnalog*255)/1023;
  xungPWM += i;
  if (xungPWM >= 130) {
    xungPWM =130;
  }
  PWM_16Bit_Frequency(&OCR3A, xungPWM);
  //PWM_16Bit_Frequency(&OCR3B, xungPWM);
  PWM_16Bit_Frequency(&OCR3C, xungPWM);
  Serial.println(xungPWM);
  // if (xungPWM >= 250) {
  //  i = -10;
  // }
  // if (xungPWM <= 0) {
  //   i = 10;
  // }
  
  delay(100);

}
