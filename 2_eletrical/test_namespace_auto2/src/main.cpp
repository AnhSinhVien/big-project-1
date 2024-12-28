#include "GlobalsSpeed.h"
#include "init.h"

// Khởi tạo các biến extern 
volatile uint16_t* pinDkBanhA = &OCR3A;     // Bánh trái (chân 5) 
volatile uint16_t* pinDkBanhB = &OCR3B;     // Bánh phải (chân 2)

uint16_t outSpeedBanhA = 0; 
uint16_t outSpeedBanhB = 0;

float KP = 1, KI = 0.0005, KD = 25;  // Hệ số PID



//-------------------- SETUP --------------------------//
void setup() {
  // khơi tạo serial
  Serial.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  main::config::setup();

}

//-------------------- LOOP --------------------------//
void loop() {
  //main::manual::readSpeedPOT_HG();
  main::lcd::che_do_da_chon();
  //main::manual::modeManual();
  //main::autoNoPID::modeAuto();
  // main::autoPID::modeAuto();
}
//-------------------- END LOOP --------------------------//

/******************** CHÚ THÍCH TOÀN CHƯƠNG TRÌNH ********************/
/*
ĐÃ HOÀN THIỆN:
  - CHỌN 2 CHẾ ĐỘ AUTO, MANUAL (QUA WIFI: WEBSITE, BLUETOOTH: APP)
  - ĐIỀU KHIỂN TỐC ĐỘ BẰNG CHIẾT ÁP
  - CÓ THỂ ĐẢO CHIỀU
  - HIỂN THỊ CÁC THỨ CẦN QUA LCD2004
*/