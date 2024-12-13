#ifndef INIT_H
#define INIT_H

// khai báo thư viện
#include <Arduino.h>
#include <TimerOne.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


// khởi tạo các chân
// chân chọn chế độ LCD
#define pin_choose 19
#define pin_select 18

// Chân relay khoá
#define relay 22

// Chân cảm biến HC-SR04
#define trigPin  7
#define echoPin  6

// Chân cảm biến hồng ngoại
const byte sensor_hn[5] = {12,11,10,9,8};

//manual
// Chân điều khiển
// Chân đảo bánh
#define pinDaoBanhA 4
#define pinDaoBanhB 3
// chân đảo bánh thủ công
#define pinDaoBanhTC 25
// Chân đọc tốc độ
// #define pinReadSpeedBanhA A2
// #define pinReadSpeedBanhB A3
// Chân còi
#define coi 44

// extern void PWM_16Bit_SetupFrequency(int timer, unsigned int frequency);
// extern void PWM_16Bit_Frequency(uint16_t* regPin, unsigned long percentPWM);

namespace main
{   
    namespace lcd
    {
        extern LiquidCrystal_I2C lcd;
        extern bool allow_reading_auto;
        extern bool allow_reading_manual;
        extern volatile bool value_chooseMenu;

        void setup();
        void chooseMenu();
        void selectChoose();
        //void menuSelect();
        void che_do_da_chon();
        
    } // namespace lcd

    namespace PWM
    {
        void setup(int timer, unsigned int frequency);
        void PWM_16Bit_Frequency(volatile uint16_t* regPin, unsigned long percentPWM);
    } // namespace PWM
    

    namespace manual
    {
        extern uint16_t outSpeedBanhA;
        extern uint16_t outSpeedBanhB;
        extern volatile uint16_t* pinDkBanhA;
        extern volatile uint16_t* pinDkBanhB;
        void setup();
        void readSpeedPOT_HG();
        void modeManual();
        void daoBanhQuay();
        void daoBanhQuayAUTO();
        void lcd_dao_banh();
        void modeStop();
        void moveForward();
        void moveBack();
        void moveLeft();
        void moveRight();
    } // namespace manual

    namespace HCSR04
    {
        typedef struct 
        {
            uint32_t duration;      // biến đo thời gian
            uint16_t distance;      // biến lưu khoảng cách
        }HC_SR04;

        extern HC_SR04 sensorData;
        void setup();
        void Xung_trig();
        void calculateDistance();
    } // namespace HCSR04
    
    namespace autoNoPID
    {
        void setup();
        void modeAuto();
    } // namespace autoNoPID
    

    namespace config
    {
        void setup();
    } // namespace config 
    
}


#endif