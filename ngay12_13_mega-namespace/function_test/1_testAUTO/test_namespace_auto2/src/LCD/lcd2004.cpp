#include "init.h"
#include "GlobalsSpeed.h"

namespace main
{
    namespace lcd
    {
        LiquidCrystal_I2C lcd(0x27,20,4);
        // biến chọn chế độ menu
        volatile bool value_chooseMenu = 0;         // bật menu
        volatile bool value_selectChoose = 0;       // lựa chọn chế độ
        volatile bool chooseChanged = false;        // Cờ để báo rằng menu đã thay đổi
        uint8_t chooseStatus = 0;                   // biến khoá đã chọn
        uint8_t statusWork = 0;

        // cho phép đọc dữ liệu auto hay manual
        bool allow_reading_auto = 0;
        bool allow_reading_manual = 0;

        // kí tự mũi tên
        uint8_t muiTen[] = { B10000, B11000, B11100, B11110, B11110, B11100, B11000, B10000 };

        // quản lí xoá màn hình LCD
        bool xoaHienThi1 = 0;

        void setup(){
            lcd.init();                      // initialize the lcd 
            lcd.backlight();
            lcd.createChar(0, muiTen);

            // khai báo chân dùng ngắt
            pinMode(pin_select, INPUT_PULLUP);
            pinMode(pin_choose,INPUT_PULLUP);
            attachInterrupt(digitalPinToInterrupt(pin_select),selectChoose,FALLING);    // ngắt lựa chọn
            attachInterrupt(digitalPinToInterrupt(pin_choose),chooseMenu,FALLING);    // ngắt chọn
        }

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
                digitalWrite(relay, 1);  //khoá bánh
                allow_reading_auto = 0;
                allow_reading_manual = 0;
                OCR3A = 0;
                OCR3B = 0;
                outSpeedBanhA = 0;
                outSpeedBanhB = 0;
                if (xoaHienThi1 == 0)
                {
                    main::lcd::lcd.clear();
                    xoaHienThi1 = 1;
                }
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
                //Dao_Banh_thu_cong();
            }
        }

        // CHẾ ĐỘ LỰA CHỌN HOẠT ĐỘNG
        void che_do_da_chon(){
            menuSelect();
            
            // kết quả
            if (value_chooseMenu == 0 && chooseStatus == 0)
            {
                if(chooseChanged == true){
                    // lcd.clear();
                    // lcd.setCursor(7, 0);
                    // lcd.print("AUTO");
                    digitalWrite(relay, 0);  //mở khoá bánh
                    lcd.clear();
                    main::PWM::PWM_16Bit_Frequency(pinDkBanhA, 65);
                    main::PWM::PWM_16Bit_Frequency(pinDkBanhB, 65);
                    xoaHienThi1 = 0;
                }
                // hành động 1: AUTO
                main::autoNoPID::modeAuto();
            }

            if (value_chooseMenu == 0 && chooseStatus == 1)
            {
                if(chooseChanged == true){
                    // lcd.clear();
                    // lcd.setCursor(7, 0);
                    // lcd.print("MANUAL");
                    digitalWrite(relay, 0);  //mở khoá bánh
                    lcd.clear();
                    xoaHienThi1 = 0;
                }

                // hành động 2: MANUAL 
                //main::manual::modeManual();
                main::ManualLine::modeDoLine();
            }
            chooseChanged = false;
        }

        
    } // namespace lcd
    
} // namespace main
