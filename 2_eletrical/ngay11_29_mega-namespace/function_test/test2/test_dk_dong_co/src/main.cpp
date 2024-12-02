#include <Arduino.h>
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

// khai báo test
const int pinDK[4] = {40, 41, 42, 43};      // lên, xuống, trái, phải
byte dk_manual = 0;



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
void daoBanhQuay();           // hàm đảo quay chế độ manual
void daoBanhQuayAUTO();       // hàm đảo quay khi ở chế độ auto
void Dao_Banh_thu_cong();     // hàm đảo bánh thủ công

// HÀM AUTO
void Xung_trig();
void read_data();
void modeAuto();

// HÀM MANUAL
void modeManual();

// hàm test
void test_dk_manual();
void lcd_dao_banh();



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

  // Chân điều khiển tốc độ
  pinMode(pinDkBanhA, OUTPUT);
  pinMode(pinDkBanhB, OUTPUT);
  digitalWrite(pinDkBanhA, 0);
  digitalWrite(pinDkBanhB, 0);
  // Chân đảo bánh
  pinMode(pinDaoBanhA, OUTPUT);
  pinMode(pinDaoBanhB, OUTPUT);
  //KHAI BÁO CHÂN ĐỌC TÍN HIỆU
  pinMode(pinReadSpeedBanhA, INPUT);
  pinMode(pinReadSpeedBanhB, INPUT);
  pinMode(pinDaoBanhTC, INPUT_PULLUP);

  // biến test
  for (int i = 0; i < 4; i++)
  {
    pinMode(pinDK[i], INPUT_PULLUP);
  }
  

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



void modeAuto(){
  allow_reading_auto = 1;
  
  // ĐỂ ĐẢM BẢO LÀ DÙ CÓ KẾT THÚC CHẾ ĐỘ MANUAL Ở CHẾ ĐỘ LÙI THÌ SANG AUTO VẪN ĐI THẲNG
  daoBanhQuayAUTO();

  // trong trường hợp tại vị trí start và cần đợi lệnh

}


//  CHẾ ĐỘ MANUAL
void modeManual(){
  allow_reading_manual = 1;
  test_dk_manual();
  switch (read_manual)
  {
    case 'f': daoBanhQuay(); daoChieuBanh = 0; moveForward(); break;
    case 'l': daoBanhQuay(); daoChieuBanh = 0; moveLeft(); break;
    case 'r': daoBanhQuay(); daoChieuBanh = 0; moveRight(); break;
    case 'b': daoBanhQuay(); daoChieuBanh = 1; moveBack(); break;
    default: modeStop(); break;
  }
}

// Đọc dữ liệu speed từ triết áp
void readSpeedPOT_HG(){
  readSpeedBanhA = analogRead(pinReadSpeedBanhA);
  readSpeedBanhB = analogRead(pinReadSpeedBanhB);
  outSpeedBanhA = (readSpeedBanhA*50)/1023;
  outSpeedBanhB = (readSpeedBanhB*50)/1023;
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
    digitalWrite(pinDaoBanhA,1);
    digitalWrite(pinDaoBanhB,1);
    delay(70);
    digitalWrite(pinDaoBanhA,0);
    digitalWrite(pinDaoBanhB,0);
  }
  if((read_manual == 'f'|| read_manual == 'l'|| read_manual == 'r') && daoChieuBanh == 1){
    digitalWrite(pinDaoBanhA,1);
    digitalWrite(pinDaoBanhB,1);
    delay(70);
    digitalWrite(pinDaoBanhA,0);
    digitalWrite(pinDaoBanhB,0);
  }
  lcd_dao_banh();
}


// Hàm đảo quay chế độ auto
void daoBanhQuayAUTO(){
  if (daoChieuBanh == 1)
  {
    digitalWrite(pinDaoBanhA,1);
    digitalWrite(pinDaoBanhB,1);
    delay(70);
    digitalWrite(pinDaoBanhA,0);
    digitalWrite(pinDaoBanhB,0);
    daoChieuBanh = 0;
  }
  lcd_dao_banh();
}


// hàm test phím
void test_dk_manual(){
  for (int i = 0; i < 4; i++)
  {
    if(digitalRead(pinDK[i]) == 0){
      dk_manual = i+1;
    }
  }
  switch (dk_manual)
  {
    case 1: read_manual = 'f'; break;
    case 2: read_manual = 'b'; break;
    case 3: read_manual = 'l'; break;
    case 4: read_manual = 'r'; break;
    default: read_manual = 's'; break;
  }
  
}

void lcd_dao_banh(){
  lcd.setCursor(0, 3);
  lcd.print("che do quay: ");
  lcd.print(daoChieuBanh);    // tiến: 0      lùi: 1
}

// Hàm đảo bánh thủ công
void Dao_Banh_thu_cong(){
  last_DaoBanhTC = digitalRead(pinDaoBanhTC);
  if (last_DaoBanhTC == 0 && before_DaoBanhTC == 1)
  {
    digitalWrite(pinDaoBanhA,1);
    digitalWrite(pinDaoBanhB,1);
    delay(100);
    digitalWrite(pinDaoBanhA,0);
    digitalWrite(pinDaoBanhB,0);
  }
  before_DaoBanhTC = last_DaoBanhTC;
}