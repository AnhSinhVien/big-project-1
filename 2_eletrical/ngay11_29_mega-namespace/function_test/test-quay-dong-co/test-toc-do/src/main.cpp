#include <Arduino.h>

#include "PWM16bitSetupFreq.h"

#define pinRead A0

int xungPWM = 65;
int prevXungPWM = -1;  // Lưu giá trị xungPWM trước đó
int i = 5;

void setup() {
  Serial.begin(9600);
  pinMode(5, OUTPUT);  // Chân 6 là chân OC3A (PWM)
  pinMode(2, OUTPUT);  // Chân 8 là chân OC3C (PWM)
  pinMode(13, OUTPUT);  // Chân 8 là chân OC3C (PWM)
  PWM_16Bit_SetupFrequency(3, 2000);
  digitalWrite(13, 0);
  delay(1000);
}

void loop() {
  //---test 1---//
  // unsigned long readAnalog = analogRead(pinRead);
  // int xungPWM = (readAnalog*255)/1023;
  // // Kiểm tra nếu xungPWM có sự thay đổi
  // if (xungPWM != prevXungPWM) {
  //   prevXungPWM = xungPWM;  // Cập nhật giá trị trước đó
  //   Serial.println(xungPWM);  // In giá trị mới ra màn hình
  // }

  //---test 2---//
  // xungPWM += i;
  // if (xungPWM >= 110 || xungPWM < 65) {
  //   delay(3000);
  //   i = -i;
  // }
  // Serial.println(xungPWM);
  // PWM_16Bit_Frequency(&OCR3A, xungPWM);
  // PWM_16Bit_Frequency(&OCR3B, xungPWM);
  // delay(200);

  //---test 3---//
  int xungPWM = 65;
  Serial.println(xungPWM);
  PWM_16Bit_Frequency(&OCR3A, xungPWM);
  PWM_16Bit_Frequency(&OCR3B, xungPWM);
  delay(3000);

  xungPWM = 90;
  Serial.println(xungPWM);
  PWM_16Bit_Frequency(&OCR3A, xungPWM);
  PWM_16Bit_Frequency(&OCR3B, xungPWM);
  delay(3000);

  xungPWM = 65;
  Serial.println(xungPWM);
  PWM_16Bit_Frequency(&OCR3A, xungPWM);
  PWM_16Bit_Frequency(&OCR3B, xungPWM);
  delay(3000);

  xungPWM = 100;
  Serial.println(xungPWM);
  PWM_16Bit_Frequency(&OCR3A, xungPWM);
  PWM_16Bit_Frequency(&OCR3B, xungPWM);
  delay(3000);

  xungPWM = 65;
  Serial.println(xungPWM);
  PWM_16Bit_Frequency(&OCR3A, xungPWM);
  PWM_16Bit_Frequency(&OCR3B, xungPWM);
  delay(3000);

  xungPWM = 110;
  Serial.println(xungPWM);
  PWM_16Bit_Frequency(&OCR3A, xungPWM);
  PWM_16Bit_Frequency(&OCR3B, xungPWM);
  delay(3000);

}
