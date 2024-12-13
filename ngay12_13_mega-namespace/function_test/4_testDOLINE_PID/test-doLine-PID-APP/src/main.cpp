#include <Arduino.h>
#include <TimerOne.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "PWM16bitSetupFreq.h"

// khai báo pin cảm biến
// Chân cảm biến hồng ngoại
/**
   * Trái -------------------------- Phải
   * |                                  |
   * | OUT5 | OUT4 | OUT3 | OUT2 | OUT1 |
   *
   * Khoảng cách phát hiện Line ĐEN (~1cm)
   * Có Line - HIGH - Bit 0
   * Ko Line - LOW  - Bit 1
*/
const byte sensor_hn[5] = {8,9,10,11,12};
// Chân cảm biến hồng ngoại
byte TT_sensor_hn[5] = {0};
const int che_do_chan_sensorHN[5] = {-2,-1,0,1,2};
int kt_che_do_chan_sensorHN[5] = {0};

// các mức tốc độ
#define FAST 85   //!
#define NORMAL 80 //!
#define SLOW 75   //!
#define STOP 65 

// Các mức tốc độ pid
#define BASE_SPEED 77  // Tốc độ cơ sở
#define MAX_SPEED 85   // Tốc độ tối đa
#define MIN_SPEED 70   // Tốc độ tối thiểu

// biến tốc độ điều chỉnh
volatile uint16_t* pinDkBanhA = &OCR3A;       // Bánh trái (chân 5)
volatile uint16_t* pinDkBanhB = &OCR3B;       // Bánh phải (chân 3)
uint16_t speedA = BASE_SPEED, speedB = BASE_SPEED;

// PID điều khiển hướng
float KP = 1, KI = 0.000007, KD = 23;  // Hệ số PID
struct PIDcar
{
  int P = 0, I = 0, D = 0;
  float Kp = KP, Ki = KI, Kd = KD;  
  float PID_value;
  int errorPrev; // Lưu giá trị "error" trước đó
};

PIDcar car;


// vị trí bàn
byte vi_tri = 0;
byte chong_nhieu1 = 0;

// nhận thông tin lên app
struct dataBluetooth
{
  float dataKp;
  float dataKi;
  float dataKd;
};
dataBluetooth readData;
String data;

// Lưu thời điểm phát hiện xe vừa ra ngoài line
unsigned long capPoint;
unsigned long capPoint2;
#define TIME_OUT 3000

// biến dùng khi ngoài line
bool dung = 0;

/**
* Cho biết hướng lệch hiện tại của xe khi xe quá vạch ra ngoài
* Lệch Phải mức 2 : +2, +3
* Lệch Phải mức 1 : +1
* Ngay giữa line  :  0
* Lệch Trái mức 1 : -1
* Lệch Trái mức 2 : -2, -3
*/
int8_t direction;


// cảm biến siêu âm
// Chân cảm biến HC-SR04
#define trigPin  7
#define echoPin  6
typedef struct 
{
  uint32_t duration;      // biến đo thời gian
  uint16_t distance;      // biến lưu khoảng cách
}HC_SR04;
HC_SR04 sensorData;
uint32_t timer_ngat = 100000;   // 100ms


// LCD
LiquidCrystal_I2C lcd(0x27,20,4);

// relay khoá
#define RELAY 22

// khai báo hàm
void readDataBluetooth();
void Xung_trig();
void calculateDistance();
void Stop();
void DiTien();
void CustomHuong(uint8_t PWM_A, uint8_t PWM_B);
void PID_Control(int8_t errorNow);
void mainCode();

void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);
  
  // Động cơ
  pinMode(5, OUTPUT);  // Chân 6 là chân OC3A (PWM)
  pinMode(2, OUTPUT);  // Chân 8 là chân OC3C (PWM)
  pinMode(RELAY, OUTPUT);  // Chân 8 là chân OC3C (PWM)
  PWM_16Bit_SetupFrequency(3, 2000);
  digitalWrite(RELAY, 0);

  // hồng ngoại
  for (int i = 0; i < 5; i++)
  {
      pinMode(sensor_hn[i], INPUT);
  }

  // LCD
  lcd.init();                      // initialize the lcd 
  lcd.backlight();

  // siêu âm
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
            
  // Cấu hình timer để tạo ngắt mỗi 100ms
  Timer1.initialize(timer_ngat);  // 100ms
  Timer1.attachInterrupt(Xung_trig); // Gọi hàm ngắt triggerUltrasonic
  Stop();
  Serial.print("A: "); Serial.print(speedA);
  Serial.print(" - ");
  Serial.print("B: "); Serial.println(speedB);
}

void loop() {
  readDataBluetooth();
  if ((readData.dataKp + readData.dataKi + readData.dataKd) != 0)
  {
    car.Kp = readData.dataKp;
    car.Ki = readData.dataKi;
    car.Kd = readData.dataKd;
  }
    // Serial.print("PID: "); 
    // Serial.print(KP,7); // In với 8 chữ số thập phân
    // Serial.print(", ");
    // Serial.print(KI,7);
    // Serial.print(", ");
    // Serial.print(KD,7);
    // Serial.print(" - ");
  if (data == "a")
  {
    Serial.print("PID: "); 
    Serial.print(car.Kp,7); // In với 8 chữ số thập phân
    Serial.print(", ");
    Serial.print(car.Ki,7);
    Serial.print(", ");
    Serial.print(car.Kd,7);
    Serial.print(" - ");
    mainCode();
  }
  else if(data == "s")
  {
    Stop();
    car.P = 0;
    car.I = 0;
    car.D = 0;
  }
}


//-------HÀM CON-------//
//-------HÀM ĐỌC DU LIỆU-------//
void readDataBluetooth(){
  if (Serial3.available()) {
    // Đọc dữ liệu từ HC-05
    data = Serial3.readString();
    Serial.print("data: ");
    Serial.println(data); // In với 8 chữ số thập phân

    // Hiển thị dữ liệu lên Serial Monitor
    float value1, value2, value3;
    int spaceIndex1 = data.indexOf(' '); // Vị trí khoảng trắng đầu tiên
    int spaceIndex2 = data.indexOf(' ', spaceIndex1 + 1); // Vị trí khoảng trắng thứ hai
    
    // Chuyển đổi các giá trị từ chuỗi sang float
    value1 = data.substring(0, spaceIndex1).toFloat();
    value2 = data.substring(spaceIndex1 + 1, spaceIndex2).toFloat();
    value3 = data.substring(spaceIndex2 + 1).toFloat();

    if ((value1 + value2 +value3) != 0)
    {
      readData.dataKp = value1;
      readData.dataKi = value2;
      readData.dataKd = value3;
    }
    
    // hiển thị thông số pid lên lcd
    lcd.setCursor(0,0);
    lcd.print("Kp: ");    lcd.print(readData.dataKp);
    lcd.setCursor(0,1);
    lcd.print("Ki: ");    lcd.print(readData.dataKi);
    lcd.setCursor(0,2);
    lcd.print("Kd: ");    lcd.print(readData.dataKd);
    // In kết quả
    // Serial.println("Các giá trị float:");
    // Serial.println(readData.dataKp,7); // In với 8 chữ số thập phân
    // Serial.println(readData.dataKi,7);
    // Serial.println(readData.dataKd,7);
  }
}

//-----HÀM CB SIÊU ÂM-----//
void Xung_trig(){
  digitalWrite(trigPin, 0);     // tắt chân trig
  delayMicroseconds(2);
  digitalWrite(trigPin, 1);     // phát xung từ chân trig
  delayMicroseconds(5);         // xung có độ dài 5 microSeconds
  digitalWrite(trigPin, 0);     // tắt chân trig
}

void calculateDistance(){
  /* Tính toán thời gian */
  // Đo độ rộng xung HIGH ở chân echo. 
  sensorData.duration = pulseIn(echoPin,HIGH);  
  // Tính khoảng cách đến vật.
  sensorData.distance = int(sensorData.duration/2/29.412);
  Serial.print("K/c: ");
  Serial.print(sensorData.distance);
  Serial.print("cm - ");
}

//-----HÀM AUTO-----//
// xe dừng 
void Stop(){
  speedA = STOP;
  speedB = STOP;
  PWM_16Bit_Frequency(pinDkBanhA, speedA);
  PWM_16Bit_Frequency(pinDkBanhB, speedB);
}

// xe tiến
void DiTien(){
  speedA = SLOW;      // tốc độ bánh A
  speedB = SLOW;      // tóc độ bánh B
  PWM_16Bit_Frequency(pinDkBanhA, speedA);
  PWM_16Bit_Frequency(pinDkBanhB, speedB);
}

// chuyển hướng trái phải
void CustomHuong(uint8_t PWM_A, uint8_t PWM_B){
  speedA = PWM_A;      // tốc độ bánh A
  speedB = PWM_B;      // tóc độ bánh B
  PWM_16Bit_Frequency(pinDkBanhA, speedA);
  PWM_16Bit_Frequency(pinDkBanhB, speedB);
}

void PID_Control(int8_t errorNow) {
  // Tính toán PID
  car.P = errorNow;
  car.I += errorNow;
  car.D = errorNow - car.errorPrev;

  // Tính toán giá trị PID
  car.PID_value = (car.Kp * car.P) + (car.Ki * car.I) + (car.Kd * car.D);

  // Cập nhật tốc độ bánh xe dựa trên correction
  speedA = constrain(BASE_SPEED + car.PID_value, MIN_SPEED, MAX_SPEED);
  speedB = constrain(BASE_SPEED - car.PID_value, MIN_SPEED, MAX_SPEED);

  // Gửi tốc độ đến động cơ
  PWM_16Bit_Frequency(pinDkBanhA, speedA);
  PWM_16Bit_Frequency(pinDkBanhB, speedB);

  car.errorPrev = errorNow;
}

void mainCode(){
  calculateDistance();
  // sensorData.distance = 13;
  // Biến kiểm tra đến vạch
  byte dem_vach = 0;
  // Điều kiện khoảng cách
  if(sensorData.distance >= 7)
  {
    // Đọc cảm biến hồng ngoại
    for (int i = 0; i < 5; i++)
    {
      TT_sensor_hn[i] = digitalRead(sensor_hn[i]);
      // kt xem mắt nào có ở line
      if(TT_sensor_hn[i] == 0)
      {
        // kt chế độ chân line và gán giá trị
        kt_che_do_chan_sensorHN[i] = che_do_chan_sensorHN[i];
      }
      else
      {
        kt_che_do_chan_sensorHN[i] = 0;
        dem_vach++;
      }
    }
    // tính tổng và tìm ra đường đi
    int sum = kt_che_do_chan_sensorHN[0]+kt_che_do_chan_sensorHN[1]+kt_che_do_chan_sensorHN[2]+kt_che_do_chan_sensorHN[3]+kt_che_do_chan_sensorHN[4];
    //Serial.println(sum);
                
    // vị trí bàn
    if (dem_vach == 0 )
    {
      if (chong_nhieu1 == 0)
      {
        vi_tri++;
        chong_nhieu1 = 1;
        if (vi_tri == 7)
        {
          vi_tri = 0;
        }
      }
      // khi bị nhấc lên khỏi line
      if (millis() - capPoint2 >= TIME_OUT)
      {
        dung = 1;
        Stop();
      }
      //Serial2.println(String(vi_tri));
      Serial.print("table: ");
      Serial.print(vi_tri);  
      Serial.print(" - ");   
    }
    if (dem_vach != 0)
    {
      capPoint2 = millis();
      chong_nhieu1 = 0;
      dung = 0;
    }

    // biến 'direction' sẽ gán giá trị cuối cùng khi xe rời vạch nhằm tìm lại đường
    // ngoài đường line 
    if (dem_vach == 5)
    {
      if (millis() - capPoint >= TIME_OUT)
      {
        //direction = 0;
        Stop();
      }
    }
    // khi xe trong đường line
    else
    {
      if (dung == 0)
      {
        capPoint = millis();
        // Điều khiển robot theo PID
        switch (sum)
        {
        /* ------------------------ 11110 ------------------------ */
        case 2:
          direction = 3;
          break;
        /* ------------------------ 11100 ------------------------ */
        case 3:
          direction = 2;
          break;
        /* ------------------------ 11101 ------------------------ */
        case 1:
          direction = 1;
          break;
        /* ------------------------ 11011 ------------------------ */
        case 0:
          direction = 0;
          break;
        /* ------------------------ 10111 ------------------------ */
        case -1:
          direction = -1;
          break;
        /* ------------------------ 00111 ------------------------ */
        case -3:
          direction = -2;
          break;
        /* ------------------------ 01111 ------------------------ */
        case -2:
          direction = -3;
          break;
        /* ------------------------ Các trường hợp khác ------------------------ */
        default:
          break;
        }
        PID_Control(direction);
      }
      
    }
  }
  else
  {
    //Serial.println("Stop");
    Stop();
  }

  // in ra màn hình LCD16x2
  lcd.setCursor(0,0);
  lcd.print(sensorData.distance);
  lcd.println("cm - ");
  // lcd.setCursor(7,0);
  // lcd.print("ban: ");
  // lcd.print(vi_tri);

  // hiển thị tốc độ 
  // Bánh A
  lcd.setCursor(0,1);
  lcd.print("A: ");
  lcd.setCursor(3, 1);
  if (speedA < 10) lcd.print("0");  // Thêm số 0 nếu dưới 10
  lcd.print(speedA);
  //Bánh B
  lcd.setCursor(8,1);
  lcd.print("B: ");
  lcd.setCursor(11, 1);
  if (speedB < 10) lcd.print("0");  // Thêm số 0 nếu dưới 10
  lcd.print(speedB);

  Serial.print("A: "); Serial.print(speedA);
  Serial.print(" - ");
  Serial.print("B: "); Serial.print(speedB);
  Serial.print(" - ");Serial.println(car.PID_value);
  
}


