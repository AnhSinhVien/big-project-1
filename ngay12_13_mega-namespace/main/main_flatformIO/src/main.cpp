#include <Arduino.h>
#include <TimerOne.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);  

//******************** BIẾN HÀM MENU ***********************//
// biến chọn chế độ menu
volatile bool value_chooseMenu = 0;
volatile bool value_selectChoose = 0;
volatile bool chooseChanged = false;  // Cờ để báo rằng menu đã thay đổi
uint8_t chooseStatus = 0;
uint8_t statusWork = 0;

// cho phép đọc dữ liệu auto
bool allow_reading_auto = 0;
bool allow_reading_manual = 0;

// kí tự mũi tên
uint8_t muiTen[] = { B10000, B11000, B11100, B11110, B11110, B11100, B11000, B10000 };

// chân chọn chế độ
#define pin_choose 19
#define pin_select 18

//******************** BIẾN HÀM ĐIỀU KHIỂN ***********************//
// BIẾN AUTO
// Đi sang Trái: 8,7
// Đi sang Phải: 5,4
// Đi thẳng    : 6

// Chân cảm biến HC-SR04
const int trigPin = 2;
const int echoPin = 3;
unsigned long duration;                // biến đo thời gian
int distance;                          // biến lưu khoảng cách
long timer_ngat = 100000;              // 100ms

// Chân cảm biến hồng ngoại
const byte sensor_hn[5] = {8,7,6,5,4};
byte TT_sensor_hn[5] = {0};
const int che_do_chan_sensorHN[5] = {-2,-1,0,1,2};
int kt_che_do_chan_sensorHN[5] = {0};

// vị trí bàn
byte vi_tri = 0;
byte chong_nhieu1 = 0;

// nhận thông tin lên esp32
String receivedData = "";
int receivedDataNumber = 0;

// BIẾN MANUAL
//*******************************chú thích**************************
//  f = 'tiến', b = 'lùi' ,
//  l = 'quay trái',    r = 'quay phải'
//******************************************************************
char read_manual;
// Chân điều khiển
#define pinDkBanhA 12         // Bánh trái
#define pinDkBanhB 11         // Bánh phải
// Chân đảo bánh
#define pinDaoBanhA 10
#define pinDaoBanhB 9
byte daoChieuBanh = 0;
// chân đảo bánh thủ công
#define pinDaoBanhTC 25
byte last_DaoBanhTC = 1;
byte before_DaoBanhTC = 1;
// Chân đọc tốc độ
#define pinReadSpeedBanhA A2
#define pinReadSpeedBanhB A3
//ĐỌC CÁC BÁNH
long int readSpeedBanhA = 0;
long int readSpeedBanhB = 0;
// BĂM XUNG CÁC BÁNH
int outSpeedBanhA, outSpeedBanhB;
const int xung_max = 50;
// Chân còi
#define coi 44


//-------------------- HÀM CON --------------------------//
// HÀM MENU
void chooseMenu();
void selectChoose();
void menuSelect();
void che_do_da_chon();

// HÀM CHUYỂN ĐỘNG
void modeStop();
void moveForward();
void moveBack();
void moveLeft();
void moveRight();
void readSpeedPOT_HG();
void Dao_Banh_thu_cong();     // hàm đảo bánh thủ công
void daoBanhQuay();           // hàm đảo quay chế độ manual
void daoBanhQuayAUTO();       // hàm đảo quay khi ở chế độ auto
void lcd_dao_banh();

// HÀM AUTO
void Xung_trig();
void read_data();
void modeAuto();

// HÀM MANUAL
void modeManual();



//-------------------- SETUP --------------------------//
void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.createChar(0, muiTen);

  // khai báo chân dùng ngắt
  pinMode(pin_select, INPUT_PULLUP);
  pinMode(pin_choose,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pin_select),selectChoose,FALLING);    // ngắt lựa chọn
  attachInterrupt(digitalPinToInterrupt(pin_choose),chooseMenu,FALLING);    // ngắt chọn

  // BIẾN ĐIỀU KHIỂN AUTO
  for (int i = 0; i < 5; i++)
  {
    pinMode(sensor_hn[i], INPUT);
  }

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Cấu hình timer để tạo ngắt mỗi 100ms
  Timer1.initialize(timer_ngat);  // 100ms
  Timer1.attachInterrupt(Xung_trig); // Gọi hàm ngắt triggerUltrasonic

  // Chân điều khiển tốc độ
  pinMode(pinDkBanhA, OUTPUT);
  pinMode(pinDkBanhB, OUTPUT);
  digitalWrite(pinDkBanhA, 0);
  digitalWrite(pinDkBanhB, 0);

  // Chân đảo bánh
  pinMode(pinDaoBanhA, OUTPUT);
  pinMode(pinDaoBanhB, OUTPUT);
  pinMode(pinDaoBanhTC, INPUT_PULLUP);
  digitalWrite(pinDaoBanhA, 1);     // mặc định của đảo bánh
  digitalWrite(pinDaoBanhA, 1);

}

//-------------------- LOOP --------------------------//
void loop() {
  readSpeedPOT_HG();
  che_do_da_chon();
}
//-------------------- END LOOP --------------------------//


//-------------------- HÀM CON --------------------------//

//******************** HÀM CON MENU ***********************//
// NÚT CHỌN MENU
void chooseMenu(){
  value_chooseMenu = !value_chooseMenu;
  chooseChanged = true;
}

// NÚT LỰA CHỌN
void selectChoose(){
  value_selectChoose = !value_selectChoose;
  chooseChanged = true;
}

// GIAO DIỆN LỰA CHỌN
void menuSelect(){
  if(value_chooseMenu == 1)
  {
    allow_reading_auto = 0;
    allow_reading_manual = 0;
    if(value_selectChoose == 0)
    {
      lcd.setCursor(0, 0);
      lcd.write(0);
      lcd.print("AUTO");
      lcd.print("        ");
      lcd.setCursor(0, 1);
      lcd.print(" MANUAL");
      lcd.print("      ");
      if (value_chooseMenu == 0)
      {
        chooseStatus = 0;
      }
    }
    if(value_selectChoose == 1)
    {
      lcd.setCursor(0, 0);
      lcd.print(" AUTO");
      lcd.print("        ");
      lcd.setCursor(0, 1);
      lcd.write(0);
      lcd.print("MANUAL");
      lcd.print("      ");
      if (value_chooseMenu == 0)
      {
        chooseStatus = 1;
      }
    } 
    Dao_Banh_thu_cong();
  }


}

// CHẾ ĐỘ LỰA CHỌN HOẠT ĐỘNG
void che_do_da_chon(){
  menuSelect();
  
  // kết quả
  if (value_chooseMenu == 0 && chooseStatus == 0)
  {
    if(chooseChanged == true){
      lcd.clear();
      lcd.setCursor(7, 0);
      lcd.print("AUTO");
    }
    // hành động 1: AUTO
    modeAuto();
  }

  if (value_chooseMenu == 0 && chooseStatus == 1)
  {
    if(chooseChanged == true){
      lcd.clear();
      lcd.setCursor(7, 0);
      lcd.print("MANUAL");
    }
    // hành động 2: MANUAL 
    modeManual();
  }
  chooseChanged = false;
  // Serial.print(value_chooseMenu);
  // Serial.print(" - ");
  // Serial.println(chooseStatus);
}


//******************** HÀM CON MENU ***********************//
// HÀM CHUYỂN ĐỘNG
// ĐI THẲNG
void moveForward(){
  lcd.setCursor(3,1);
  lcd.print("          ");
  lcd.setCursor(3,1);
  lcd.print("Di thang");
  analogWrite(pinDkBanhA, outSpeedBanhA);
  analogWrite(pinDkBanhB, outSpeedBanhB);
}
// ĐI LÙI
void moveBack(){
  lcd.setCursor(3,1);
  lcd.print("          ");
  lcd.setCursor(3,1);
  lcd.print("Di lui");
  analogWrite(pinDkBanhA, outSpeedBanhA);
  analogWrite(pinDkBanhB, outSpeedBanhB);
}
// SANG TRÁI
void moveLeft(){
  lcd.setCursor(3,1);
  lcd.print("          ");
  lcd.setCursor(3,1);
  lcd.print("Sang trai");
  analogWrite(pinDkBanhA, outSpeedBanhA - outSpeedBanhA);
  analogWrite(pinDkBanhB, outSpeedBanhB);
}
// SANG PHẢI
void moveRight(){
  lcd.setCursor(3,1);
  lcd.print("          ");
  lcd.setCursor(3,1);
  lcd.print("Sang phai");
  analogWrite(pinDkBanhA, outSpeedBanhA);
  analogWrite(pinDkBanhB, outSpeedBanhB - outSpeedBanhB);
}
// STOP
void modeStop(){
  lcd.setCursor(3,1);
  lcd.print("          ");
  lcd.setCursor(3,1);
  lcd.print("Stop");
  analogWrite(pinDkBanhA, outSpeedBanhA - outSpeedBanhA);
  analogWrite(pinDkBanhB, outSpeedBanhB - outSpeedBanhB);
}


// CHẾ ĐỘ AUTO
// SỬ DỤNG NGẮT TIMER CHO CB SIÊU ÂM
void Xung_trig(){
  digitalWrite(trigPin, 0);     // tắt chân trig
  delayMicroseconds(2);
  digitalWrite(trigPin, 1);     // phát xung từ chân trig
  delayMicroseconds(5);         // xung có độ dài 5 microSeconds
  digitalWrite(trigPin, 0);     // tắt chân trig
}

// ĐỌC DỮ LIỆU TỪ ESP32 BÀNG UART2
void read_data(){
  if ((Serial2.available()) && allow_reading_auto == 1) {
    receivedData = Serial2.readStringUntil('\n');                  // Đọc toàn bộ chuỗi đến khi gặp dấu ngắt dòng
    receivedDataNumber = receivedData.toInt();                    // Chuyển chuỗi thành số nguyên
  }
}

void modeAuto(){
  allow_reading_auto = 1;
  
  // ĐỂ ĐẢM BẢO LÀ DÙ CÓ KẾT THÚC CHẾ ĐỘ MANUAL Ở CHẾ ĐỘ LÙI THÌ SANG AUTO VẪN ĐI THẲNG
  daoBanhQuayAUTO();

  /* Tính toán thời gian */
  // Đo độ rộng xung HIGH ở chân echo. 
  duration = pulseIn(echoPin,HIGH);  
  // Tính khoảng cách đến vật.
  duration = pulseIn(echoPin, HIGH);
  distance = int(duration/2/29.412);
  //Serial.print(distance);
  //Serial.print("cm - ");


  // trong trường hợp tại vị trí start và cần đợi lệnh
  while (receivedDataNumber == 0 && vi_tri == 0 && value_chooseMenu == 0)
  {
    read_data();
    lcd.setCursor(0,0);
    lcd.print("                ");
    lcd.setCursor(0,0);
    lcd.print("doi lenh");

    modeStop();
  }
  

  // Biến kiểm tra đến vạch
  byte dem_vach = 0;
  // Điều kiện khoảng cách
  if(distance >= 7){
    // Đọc cảm biến hồng ngoại
    for (int i = 0; i < 5; i++)
    {
      TT_sensor_hn[i] = digitalRead(sensor_hn[i]);
      // kt xem đang ở line nào
      if(TT_sensor_hn[i] == 0){
        // kt chế độ chân line và gán giá trị
        kt_che_do_chan_sensorHN[i] = che_do_chan_sensorHN[i];
      }
      else{
        kt_che_do_chan_sensorHN[i] = 0;
        dem_vach++;
      }
    }
    // tính tổng và tìm ra đường đi
    int sum = kt_che_do_chan_sensorHN[0]+kt_che_do_chan_sensorHN[1]+kt_che_do_chan_sensorHN[2]+kt_che_do_chan_sensorHN[3]+kt_che_do_chan_sensorHN[4];
    //Serial.println(sum);

    // vị trí bàn
    if (dem_vach == 5 && chong_nhieu1 == 0)
    {
      vi_tri++;
      chong_nhieu1 = 1;
      if (vi_tri == 7)
      {
        vi_tri = 0;
      }
      Serial2.println(String(vi_tri));
  
    }
    if (dem_vach != 5)
    {
      chong_nhieu1 = 0;
    }
    //Serial.print("bàn: ");
    //Serial.println(vi_tri);
    //Serial.print(" - ");

    // hướng di chuyển
    if (sum>0)
    {
      //Serial.println("Sang trái");
      moveLeft();
    }
    else if (sum<0)
    {
      //Serial.println("Sang phải");
      moveRight();
    }
    else
    {
      //Serial.println("Đi thẳng");
      moveForward();
    }
    
    delay(7);
  }
  else{
    //Serial.println("Stop");
    modeStop();
  }

  // in ra màn hình LCD16x2
  lcd.setCursor(0,0);
  lcd.print(distance);
  lcd.print("cm - ");
  lcd.setCursor(7,0);
  lcd.print("ban: ");
  lcd.print(vi_tri);

  // Đến vị trí bàn được gửi
  int dem_time_dung_tai_table = 1000;
  if(receivedDataNumber != 0){
    while (receivedDataNumber == vi_tri && (dem_time_dung_tai_table--) !=0 )
    {
      lcd.setCursor(0,0);
      lcd.print(dem_time_dung_tai_table);
      lcd.print("s");

      modeStop();

      delay(3);
      // trở về quầy
      if(dem_time_dung_tai_table<=0){
        receivedDataNumber = 0;
      }
      
    }
  }
}


//  CHẾ ĐỘ MANUAL
void modeManual(){
  allow_reading_manual = 1;
  if ((Serial3.available()) && allow_reading_manual == 1)
  {
    read_manual = Serial3.read();
    switch (read_manual)
    {
      case 'f': daoBanhQuay(); daoChieuBanh = 0; moveForward(); break;
      case 'l': daoBanhQuay(); daoChieuBanh = 0; moveLeft(); break;
      case 'r': daoBanhQuay(); daoChieuBanh = 0; moveRight(); break;
      case 'b': daoBanhQuay(); daoChieuBanh = 1; moveBack(); break;
      default: modeStop(); break;
    }
  }
}

// Đọc dữ liệu speed từ triết áp
void readSpeedPOT_HG(){
  readSpeedBanhA = analogRead(pinReadSpeedBanhA);
  readSpeedBanhB = analogRead(pinReadSpeedBanhB);
  outSpeedBanhA = (readSpeedBanhA*xung_max)/1023;
  outSpeedBanhB = (readSpeedBanhB*xung_max)/1023;
  // Hiển thị tốc độ bánh A
  lcd.setCursor(0, 2);
  lcd.print("SPEEDA:");
  lcd.setCursor(7, 2);
  if (outSpeedBanhA < 10) lcd.print("0");  // Thêm số 0 nếu dưới 10
  lcd.print(outSpeedBanhA);
  // Hiển thị tốc độ bánh B
  lcd.setCursor(10, 2);
  lcd.print("SPEEDB:");
  lcd.setCursor(17, 2);
  if (outSpeedBanhB < 10) lcd.print("0");  // Thêm số 0 nếu dưới 10
  lcd.print(outSpeedBanhB);
}


// Hàm đảo bánh
void daoBanhQuay(){
  if (read_manual == 'b' && daoChieuBanh == 0)
  {
    digitalWrite(pinDaoBanhA,0);
    digitalWrite(pinDaoBanhB,0);
    delay(70);
    digitalWrite(pinDaoBanhA,1);
    digitalWrite(pinDaoBanhB,1);
  }
  if((read_manual == 'f'|| read_manual == 'l'|| read_manual == 'r') && daoChieuBanh == 1){
    digitalWrite(pinDaoBanhA,0);
    digitalWrite(pinDaoBanhB,0);
    delay(70);
    digitalWrite(pinDaoBanhA,1);
    digitalWrite(pinDaoBanhB,1);
  }
  lcd_dao_banh();
}


// Hàm đảo quay chế độ auto
void daoBanhQuayAUTO(){
  if (daoChieuBanh == 1)
  {
    digitalWrite(pinDaoBanhA,0);
    digitalWrite(pinDaoBanhB,0);
    delay(70);
    digitalWrite(pinDaoBanhA,1);
    digitalWrite(pinDaoBanhB,1);
    daoChieuBanh = 0;
  }
  lcd_dao_banh();
}

// Hàm đảo bánh thủ công
void Dao_Banh_thu_cong(){
  last_DaoBanhTC = digitalRead(pinDaoBanhTC);
  if (last_DaoBanhTC == 0 && before_DaoBanhTC == 1)
  {
    digitalWrite(pinDaoBanhA,0);
    digitalWrite(pinDaoBanhB,0);
    delay(70);
    digitalWrite(pinDaoBanhA,1);
    digitalWrite(pinDaoBanhB,1);
  }
  before_DaoBanhTC = last_DaoBanhTC;
}

// hiển thị lên lcd chế độ chiều quay
void lcd_dao_banh(){
  lcd.setCursor(0, 3);
  lcd.print("che do quay: ");
  if (daoChieuBanh == 0)
  {
    lcd.setCursor(13, 3);
    lcd.print("Tien");
  }
  else{
    lcd.setCursor(13, 3);
    lcd.print("lui ");
  }
  
}




/******************** CHÚ THÍCH TOÀN CHƯƠNG TRÌNH ********************/
/*
ĐÃ HOÀN THIỆN:
  - CHỌN 2 CHẾ ĐỘ AUTO, MANUAL (QUA WIFI: WEBSITE, BLUETOOTH: APP)
  - ĐIỀU KHIỂN TỐC ĐỘ BẰNG CHIẾT ÁP
  - CÓ THỂ ĐẢO CHIỀU
  - HIỂN THỊ CÁC THỨ CẦN QUA LCD2004
*/