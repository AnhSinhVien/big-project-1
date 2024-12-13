#include "init.h"
#include "GlobalsSpeed.h"

namespace main
{
    namespace config
    {
        void setup(){
            pinMode(2, OUTPUT);            //OCR3B
            pinMode(5, OUTPUT);            //OCR3A

            // Chân điều khiển tốc độ
            OCR3A = 0;
            OCR3B = 0;
            outSpeedBanhA = 0;
            outSpeedBanhB = 0;

            // Cảm biến dò line
            for (int i = 0; i < 5; i++)
            {
                pinMode(sensor_hn[i], INPUT);
            }
            

            // khai báo LCD menu
            main::lcd::setup();
            // khai báo manual
            main::manual::setup();
            //khởi tạo HC_SR04
            main::HCSR04::setup();
            // khởi tao xung
            main::PWM::setup(3,2000);
            // khởi tạo auto no PID
            //main::autoNoPID::setup();
            main::ManualLine::setup();
            // khởi tạo auto no PID
            main::autoPID::setup();
            //chân relay khoá
            pinMode(relay,OUTPUT);
        }
    } // namespace config
    
    

} // namespace main
