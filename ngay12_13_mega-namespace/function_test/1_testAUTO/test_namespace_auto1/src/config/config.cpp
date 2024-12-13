#include "init.h"

namespace main
{
    namespace config
    {
        void setup(){
            pinMode(2, OUTPUT);            //OCR3B
            pinMode(5, OUTPUT);            //OCR3A
            // khai báo LCD menu
            main::lcd::setup();
            // khai báo manual
            main::manual::setup();
            //khởi tạo HC_SR04
            main::HCSR04::setup();
            // khởi tao xung
            main::PWM::setup(3,2000);
            // khởi tạo auto no PID
            main::autoNoPID::setup();
            //chân relay khoá
            pinMode(relay,OUTPUT);
        }
    } // namespace config
    
    

} // namespace main
