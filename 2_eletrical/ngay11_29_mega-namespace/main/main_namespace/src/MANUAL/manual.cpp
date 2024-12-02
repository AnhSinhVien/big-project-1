#include "init.h"
#include "HardwareSerial.h"


namespace main
{
    namespace manual
    {
        // BIẾN MANUAL
        //*******************************chú thích**************************
        //  f = 'tiến', b = 'lùi' ,
        //  l = 'quay trái',    r = 'quay phải'
        //******************************************************************
        // khai báo chân
        volatile uint16_t* pinDkBanhA = &OCR3A;       // Bánh trái (chân 5)
        volatile uint16_t* pinDkBanhB = &OCR3C;       // Bánh phải (chân 3)
        char read_manual;
        // biến trạng thái đảo bánh (0: tiến, 1: lùi)
        byte daoChieuBanh = 0;
        // chân đảo bánh thủ công
        byte last_DaoBanhTC = 1;
        byte before_DaoBanhTC = 1;
        // Chân đọc tốc độ
        //ĐỌC CÁC BÁNH
        uint32_t readSpeedBanhA = 0;
        uint32_t readSpeedBanhB = 0;
        // BĂM XUNG CÁC BÁNH
        uint16_t outSpeedBanhA;
        uint16_t outSpeedBanhB;
        const int xung_max = 50;

        // hàm setup
        void setup(){
            // Chân điều khiển tốc độ
            pinMode(3, OUTPUT);
            pinMode(5, OUTPUT);
            OCR3A = 0;
            OCR3C = 0;
            

            // Chân đảo bánh
            pinMode(pinDaoBanhA, OUTPUT);
            pinMode(pinDaoBanhB, OUTPUT);
            pinMode(pinDaoBanhTC, INPUT_PULLUP);
            digitalWrite(pinDaoBanhA, 1);     // mặc định của đảo bánh
            digitalWrite(pinDaoBanhB, 1);
        }
        
        //  CHẾ ĐỘ MANUAL
        void modeManual(){
            main::lcd::allow_reading_manual = 1;
            readSpeedPOT_HG();
            if ((Serial3.available()) &&  main::lcd::allow_reading_manual == 1)
            {
                read_manual = Serial3.read();
                switch (read_manual)
                {
                case 'f': daoBanhQuay(); daoChieuBanh = 0; moveForward(); break;
                case 'l': daoBanhQuay(); daoChieuBanh = 0; moveLeft(); break;
                case 'r': daoBanhQuay(); daoChieuBanh = 0; moveRight(); break;
                case 'b': daoBanhQuay(); daoChieuBanh = 1; moveBack(); break;
                default: modeStop(); break;
                }
            }
            // moveForward();
            // delay(2000);
            // moveLeft();
            // delay(2000);
            // moveRight();
            // delay(2000);
        }

        // Đọc dữ liệu speed từ triết áp
        void readSpeedPOT_HG(){
            // readSpeedBanhA = analogRead(pinReadSpeedBanhA);
            // readSpeedBanhB = analogRead(pinReadSpeedBanhB);
            // outSpeedBanhA = (readSpeedBanhA*xung_max)/1023;
            // outSpeedBanhB = (readSpeedBanhB*xung_max)/1023;
            outSpeedBanhA = 230;
            outSpeedBanhB = 230;

            // Hiển thị tốc độ bánh A
            main::lcd::lcd.setCursor(0, 2);
            main::lcd::lcd.print("SPEEDA:");
            main::lcd::lcd.setCursor(7, 2);
            if (outSpeedBanhA < 10) main::lcd::lcd.print("0");  // Thêm số 0 nếu dưới 10
            main::lcd::lcd.print(outSpeedBanhA);
            // Hiển thị tốc độ bánh B
            main::lcd::lcd.setCursor(10, 2);
            main::lcd::lcd.print("SPEEDB:");
            main::lcd::lcd.setCursor(17, 2);
            if (outSpeedBanhB < 10) main::lcd::lcd.print("0");  // Thêm số 0 nếu dưới 10
            main::lcd::lcd.print(outSpeedBanhB);
        }


        // Hàm đảo bánh khi ấn phím tiến -> lùi và lùi -> tiến
        void daoBanhQuay(){
            if (read_manual == 'b' && daoChieuBanh == 0)
            {
                digitalWrite(pinDaoBanhA,0);
                digitalWrite(pinDaoBanhB,0);
                delay(70);
                digitalWrite(pinDaoBanhA,1);
                digitalWrite(pinDaoBanhB,1);
            }
            if((read_manual == 'f'|| read_manual == 'l'|| read_manual == 'r') && daoChieuBanh == 1){
                digitalWrite(pinDaoBanhA,0);
                digitalWrite(pinDaoBanhB,0);
                delay(70);
                digitalWrite(pinDaoBanhA,1);
                digitalWrite(pinDaoBanhB,1);
            }
            lcd_dao_banh();
        }


        // Hàm đảo chiều quay bánh khi đang ở trạng thái lùi khi chuyển chế độ auto
        void daoBanhQuayAUTO(){
            if (daoChieuBanh == 1)
            {
                digitalWrite(pinDaoBanhA,0);
                digitalWrite(pinDaoBanhB,0);
                delay(70);
                digitalWrite(pinDaoBanhA,1);
                digitalWrite(pinDaoBanhB,1);
                daoChieuBanh = 0;
            }
            lcd_dao_banh();
        }

        // Hàm đảo bánh thủ công
        void Dao_Banh_thu_cong(){
            last_DaoBanhTC = digitalRead(pinDaoBanhTC);
            if (last_DaoBanhTC == 0 && before_DaoBanhTC == 1)
            {
                digitalWrite(pinDaoBanhA,0);
                digitalWrite(pinDaoBanhB,0);
                delay(70);
                digitalWrite(pinDaoBanhA,1);
                digitalWrite(pinDaoBanhB,1);
            }
            before_DaoBanhTC = last_DaoBanhTC;
        }

        // hiển thị lên lcd chế độ chiều quay
        void lcd_dao_banh(){
            main::lcd::lcd.setCursor(0, 3);
            main::lcd::lcd.print("che do quay: ");
            if (daoChieuBanh == 0)
            {
                main::lcd::lcd.setCursor(13, 3);
                main::lcd::lcd.print("Tien");
            }
            else{
                main::lcd::lcd.setCursor(13, 3);
                main::lcd::lcd.print("lui ");
            }
        }

        // HÀM CHUYỂN ĐỘNG
        // ĐI THẲNG
        void moveForward(){
            main::lcd::lcd.setCursor(3,1);
            main::lcd::lcd.print("          ");
            main::lcd::lcd.setCursor(3,1);
            main::lcd::lcd.print("Di thang");
            // analogWrite(pinDkBanhA, outSpeedBanhA);
            // analogWrite(pinDkBanhB, outSpeedBanhB);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhA, outSpeedBanhA);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhB, outSpeedBanhB);
        }
        // ĐI LÙI
        void moveBack(){
            main::lcd::lcd.setCursor(3,1);
            main::lcd::lcd.print("          ");
            main::lcd::lcd.setCursor(3,1);
            main::lcd::lcd.print("Di lui");
            // analogWrite(pinDkBanhA, outSpeedBanhA);
            // analogWrite(pinDkBanhB, outSpeedBanhB);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhA, outSpeedBanhA);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhB, outSpeedBanhB);
        }
        // SANG TRÁI
        void moveLeft(){
            main::lcd::lcd.setCursor(3,1);
            main::lcd::lcd.print("          ");
            main::lcd::lcd.setCursor(3,1);
            main::lcd::lcd.print("Sang trai");
            // analogWrite(pinDkBanhA, outSpeedBanhA - outSpeedBanhA);
            // analogWrite(pinDkBanhB, outSpeedBanhB);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhA, outSpeedBanhA - outSpeedBanhA);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhB, outSpeedBanhB);
        }
        // SANG PHẢI
        void moveRight(){
            main::lcd::lcd.setCursor(3,1);
            main::lcd::lcd.print("          ");
            main::lcd::lcd.setCursor(3,1);
            main::lcd::lcd.print("Sang phai");
            // analogWrite(pinDkBanhA, outSpeedBanhA);
            // analogWrite(pinDkBanhB, outSpeedBanhB - outSpeedBanhB);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhA, outSpeedBanhA);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhB, outSpeedBanhB - outSpeedBanhB);
        }
        // STOP
            void modeStop(){
            main::lcd::lcd.setCursor(3,1);
            main::lcd::lcd.print("          ");
            main::lcd::lcd.setCursor(3,1);
            main::lcd::lcd.print("Stop");
            // analogWrite(pinDkBanhA, outSpeedBanhA - outSpeedBanhA);
            // analogWrite(pinDkBanhB, outSpeedBanhB - outSpeedBanhB);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhA, outSpeedBanhA - outSpeedBanhA);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhB, outSpeedBanhB - outSpeedBanhB);
        }

    } // namespace manual
    
} // namespace main
