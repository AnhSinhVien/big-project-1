#include "init.h"
#include "HardwareSerial.h"
#include "GlobalsSpeed.h"

namespace main
{
    namespace PWM
    {
        unsigned int freq;
        void setup(int timer, unsigned int frequency){
            unsigned long top_value = (16000000/(8*frequency)) - 1;
            freq = top_value;
            if(timer == 1){
                // RESET lại 2 thanh ghi 
                TCCR1A = 0;
                TCCR1B = 0;

                // Cấu hình Fast PWM với TOP là ICR1
                TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << COM1C1) | (1 << WGM11);    // Clear OCnA, OCnB, OCnC on Compare Match
                TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11);  // Prescaler = 8

                ICR1 = top_value;  // Đặt TOP để đạt tần số PWM mong muốn
            }
            else if(timer == 3){
                // RESET lại 2 thanh ghi 
                TCCR3A = 0;
                TCCR3B = 0;

                // Cấu hình Fast PWM với TOP là ICR3
                //TCCR3A |= (1 << COM3A1) | (1 << COM3B1) | (1 << COM3C1) | (1 << WGM31);    // Clear OCnA, OCnB, OCnC on Compare Match
                TCCR3A |= (1 << COM3A1) | (1 << COM3B1) | (1 << WGM31);
                TCCR3B |= (1 << WGM33) | (1 << WGM32) | (1 << CS31);  // Prescaler = 8

                ICR3 = top_value;  // Đặt TOP để đạt tần số PWM mong muốn
            }
            else if(timer == 4){
                // RESET lại 2 thanh ghi 
                TCCR4A = 0;
                TCCR4B = 0;

                // Cấu hình Fast PWM với TOP là ICR3
                TCCR4A |= (1 << COM4A1) | (1 << COM4B1) | (1 << COM4C1) | (1 << WGM41);    // Clear OCnA, OCnB, OCnC on Compare Match
                TCCR4B |= (1 << WGM43) | (1 << WGM42) | (1 << CS41);  // Prescaler = 8

                ICR4 = top_value;  // Đặt TOP để đạt tần số PWM mong muốn
            }
            else if(timer == 5){
                // RESET lại 2 thanh ghi 
                TCCR5A = 0;
                TCCR5B = 0;

                // Cấu hình Fast PWM với TOP là ICR3
                TCCR5A |= (1 << COM5A1) | (1 << COM5B1) | (1 << COM5C1) | (1 << WGM51);    // Clear OCnA, OCnB, OCnC on Compare Match
                TCCR4B |= (1 << WGM53) | (1 << WGM52) | (1 << CS51);  // Prescaler = 8

                ICR5 = top_value;  // Đặt TOP để đạt tần số PWM mong muốn
            }
            else{
                return;
            }
        }
        


        void PWM_16Bit_Frequency(volatile uint16_t* regPin, uint32_t percentPWM){
            if(percentPWM > 255){
                *regPin = freq;
            }
            else if(percentPWM < 0){
                *regPin = 0;
            }
            else{
                unsigned int scaleFreq = (percentPWM * freq)/255;
                *regPin = scaleFreq;
            //Serial.println(scaleFreq);
            }

        
        }
       
        //
    } // namespace PWM
    
} // namespace main
