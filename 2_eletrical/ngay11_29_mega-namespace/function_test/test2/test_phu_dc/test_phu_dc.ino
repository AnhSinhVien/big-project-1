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



//-------------------- SETUP --------------------------//
void setup() {
  Serial.begin(9600);

  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.createChar(0, muiTen);

  // Chân điều khiển tốc độ
  pinMode(pinDkBanhA, OUTPUT);
  pinMode(pinDkBanhB, OUTPUT);
  // Chân đảo bánh
  pinMode(pinDaoBanhA, OUTPUT);
  pinMode(pinDaoBanhB, OUTPUT);
  //KHAI BÁO CHÂN ĐỌC TÍN HIỆU
  pinMode(pinReadSpeedBanhA, INPUT);
  pinMode(pinReadSpeedBanhB, INPUT);

}

//-------------------- LOOP --------------------------//
void loop() {
  readSpeedPOT_HG();
}

void readSpeedPOT_HG(){
  readSpeedBanhA = analogRead(pinReadSpeedBanhA);
  readSpeedBanhB = analogRead(pinReadSpeedBanhB);
  outSpeedBanhA = (readSpeedBanhA * xung_max) / 1023;
  outSpeedBanhB = (readSpeedBanhB * xung_max) / 1023;
  
  // Hiển thị tốc độ bánh A
  lcd.setCursor(0, 3);
  lcd.print("SPEEDA:");
  lcd.setCursor(7, 3);
  if (outSpeedBanhA < 10) lcd.print("0");  // Thêm số 0 nếu dưới 10
  lcd.print(outSpeedBanhA);
  
  // Hiển thị tốc độ bánh B
  lcd.setCursor(10, 3);
  lcd.print("SPEEDB:");
  lcd.setCursor(17, 3);
  if (outSpeedBanhB < 10) lcd.print("0");  // Thêm số 0 nếu dưới 10
  lcd.print(outSpeedBanhB);
}