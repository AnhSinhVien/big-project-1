#include "init.h"
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
  //main::lcd::che_do_da_chon();
  main::manual::modeManual();
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