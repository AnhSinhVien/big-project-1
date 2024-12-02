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
const byte sensor_hn[5] = {12,11,10,9,8};
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
#define BASE_SPEED 75  // Tốc độ cơ sở
#define MAX_SPEED 85   // Tốc độ tối đa
#define MIN_SPEED 65   // Tốc độ tối thiểu

// biến tốc độ điều chỉnh
volatile uint16_t* pinDkBanhA = &OCR3A;       // Bánh trái (chân 5)
volatile uint16_t* pinDkBanhB = &OCR3B;       // Bánh phải (chân 3)
uint16_t speedA = BASE_SPEED, speedB = BASE_SPEED;

// PID điều khiển hướng
float Kp = 5.0, Ki = 0.0000007, Kd = 49.0;  // Hệ số PID
float previous_error = 0;
float integral = 0;

// vị trí bàn
byte vi_tri = 0;
byte chong_nhieu1 = 0;

// nhận thông tin lên esp32
String receivedData = "";
int receivedDataNumber = 0;

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
void Xung_trig();
void calculateDistance();
void Stop();
void DiTien();
void CustomHuong(uint8_t PWM_A, uint8_t PWM_B);
void PID_Control(int error);

void setup() {
  Serial.begin(9600);
  
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
  delay(1000);
}

void loop() {
  calculateDistance();
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
        direction = 0;
        Stop();
      }
      else
      {
        switch (direction)
        {
        case 3: // Lệch phải nhiều ... xoay phải vừa
          CustomHuong(SLOW, FAST);
          break;
        case 2: // Lệch phải nhiều ... xoay phải nhanh
          CustomHuong(STOP, NORMAL);
          break;
        case 1: // Lệch phải ít ... xoay phải chậm
          CustomHuong(SLOW, NORMAL);
          break;
        case 0: // Giữa ... cứ đi thẳng tiếp
          DiTien();
          break;
        case -1: // Lệch phải nhiều ... xoay trái chậm
          CustomHuong(NORMAL, SLOW);
          break;
        case -2: // Lệch phải nhiều ... xoay trái nhanh
          CustomHuong(NORMAL, STOP);
          break;
        case -3: // Lệch phải ít ... xoay trái vừa
          CustomHuong(FAST, SLOW);
          break;
        }
      }
    }
    // khi xe trong đường line
    else
    {
      if (dung == 0)
      {
        capPoint = millis();
        direction = sum;
        // Điều khiển robot theo PID
        PID_Control(sum);
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
  Serial.print("B: "); Serial.println(speedB);
}


//-------HÀM CON-------//

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

void PID_Control(int error) {
  // Tính toán PID
  float proportional = error;
  integral += error;
  float derivative = error - previous_error;

  float correction = Kp * proportional + Ki * integral + Kd * derivative;

  // Cập nhật tốc độ bánh xe dựa trên correction
  speedA = constrain(BASE_SPEED + correction, MIN_SPEED, MAX_SPEED);
  speedB = constrain(BASE_SPEED - correction, MIN_SPEED, MAX_SPEED);

  // Gửi tốc độ đến động cơ
  PWM_16Bit_Frequency(pinDkBanhA, speedA);
  PWM_16Bit_Frequency(pinDkBanhB, speedB);

  previous_error = error;
}