#include "init.h"

namespace main
{
    namespace config
    {
        void setup(){
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
        }
    } // namespace config
    
    

} // namespace main
