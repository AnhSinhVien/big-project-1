#include "init.h"
#include "GlobalsSpeed.h"

namespace main
{
    namespace HCSR04
    {
        HC_SR04 sensorData;

        uint32_t timer_ngat = 100000;   // 100ms

        // hàm setup
        void setup(){
            pinMode(trigPin, OUTPUT);
            pinMode(echoPin, INPUT);
            
            // Cấu hình timer để tạo ngắt mỗi 100ms
            Timer1.initialize(timer_ngat);  // 100ms
            Timer1.attachInterrupt(Xung_trig); // Gọi hàm ngắt triggerUltrasonic
        }

        // SỬ DỤNG NGẮT TIMER CHO CB SIÊU ÂM
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
            //Serial.print(distance);
            //Serial.print("cm - ");
        }

        
    } // namespace HCSR04
    
} // namespace main
