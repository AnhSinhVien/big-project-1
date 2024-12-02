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
        volatile uint16_t* pinDkBanhB = &OCR3B;       // Bánh phải (chân 3)
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
        const int xung_max = 100;

        // timer test ic điện
        #define event 100
        uint32_t time_1 = 0;
        uint8_t event_dem = 0;
        #define delay_dc 300

        // hàm setup
        void setup(){
            // Chân điều khiển tốc độ
            OCR3A = 0;
            OCR3B = 0;
            outSpeedBanhA = 0;
            outSpeedBanhB = 0;
            

            // Chân đảo bánh
            pinMode(pinDaoBanhA, OUTPUT);
            pinMode(pinDaoBanhB, OUTPUT);
            pinMode(pinDaoBanhTC, INPUT_PULLUP);
            digitalWrite(pinDaoBanhA, 0);     // mặc định của đảo bánh
            digitalWrite(pinDaoBanhB, 0);
            time_1 = millis();

            //chân relay khoá
            digitalWrite(relay, 0);  // khoá bánh
        }
        
        //  CHẾ ĐỘ MANUAL
        void modeManual(){
            main::lcd::allow_reading_manual = 1;
            if (main::lcd::allow_reading_manual == 1)
            {
                if (Serial3.available())
                {
                    read_manual = Serial3.read();
                    switch (read_manual)
                    {
                    case 'f': daoBanhQuay();  break;
                    case 'l': daoBanhQuay();  break;
                    case 'r': daoBanhQuay();  break;
                    case 'b': daoBanhQuay();  break;
                    default: modeStop(); break;
                    }
                }
                readSpeedPOT_HG();
            }
        }

        // Đọc dữ liệu speed từ triết áp
        void readSpeedPOT_HG(){
            if (read_manual == 'f')  // Chạy liên tục khi đang ở chế độ tiến
            {
                if (event_dem == 0)
                {
                    time_1 = millis();
                    event_dem = 1;
                }

                if ((millis() - time_1) >= event)
                {
                    time_1 = millis();
                    outSpeedBanhA += 10;
                    outSpeedBanhB += 10;
                    if (outSpeedBanhA >= xung_max)
                    {
                        outSpeedBanhA = xung_max;
                        outSpeedBanhB = xung_max;
                    }
                    moveForward();
                }
            }
            else if (read_manual == 'r')
            {
                if (event_dem == 0)
                {
                    time_1 = millis();
                    event_dem = 1;
                }

                if ((millis() - time_1) >= event)
                {
                    time_1 = millis();
                    outSpeedBanhA = 0;
                    outSpeedBanhB += 10;
                    if (outSpeedBanhB >= xung_max)
                    {
                        outSpeedBanhA = 0;
                        outSpeedBanhB = xung_max;
                    }
                    moveLeft(); 
                }
            }
            else if (read_manual == 'l')
            {
                if (event_dem == 0)
                {
                    time_1 = millis();
                    event_dem = 1;
                }

                if ((millis() - time_1) >= event)
                {
                    time_1 = millis();
                    outSpeedBanhA += 10;
                    outSpeedBanhB = 0;
                    if (outSpeedBanhA >= xung_max)
                    {
                        outSpeedBanhA = xung_max;
                        outSpeedBanhB = 0;
                    }
                    moveRight(); 
                }
            }
            else if (read_manual == 'b')
            {
                if (event_dem == 0)
                {
                    time_1 = millis();
                    event_dem = 1;
                }

                if ((millis() - time_1) >= event)
                {
                    time_1 = millis();
                    outSpeedBanhA += 10;
                    outSpeedBanhB += 10;
                    if (outSpeedBanhA >= xung_max)
                    {
                    outSpeedBanhA = xung_max;
                    outSpeedBanhB = xung_max;
                    }
                    moveBack(); 
                }
            }           
            else
            {
                event_dem = 0;
                outSpeedBanhA = 0;
                outSpeedBanhB = 0;
            }
            

            // Hiển thị tốc độ bánh A
            main::lcd::lcd.setCursor(0, 2);
            main::lcd::lcd.print("SPEEDA:");
            main::lcd::lcd.setCursor(7, 2);
            if (outSpeedBanhA < 10) main::lcd::lcd.print("00");  // Thêm số 0 nếu dưới 10
            else if (outSpeedBanhA < 100 && outSpeedBanhA >= 10) main::lcd::lcd.print("0");  // Thêm số 0 nếu dưới 10
            main::lcd::lcd.print(outSpeedBanhA);
            // Hiển thị tốc độ bánh B
            main::lcd::lcd.setCursor(10, 2);
            main::lcd::lcd.print("SPEEDB:");
            main::lcd::lcd.setCursor(17, 2);
            if (outSpeedBanhB < 10) main::lcd::lcd.print("00");  // Thêm số 0 nếu dưới 10
            else if (outSpeedBanhB < 100 && outSpeedBanhB >= 10) main::lcd::lcd.print("0");  // Thêm số 0 nếu dưới 10
            main::lcd::lcd.print(outSpeedBanhB);
        }


        // Hàm đảo bánh khi ấn phím tiến -> lùi và lùi -> tiến
        void daoBanhQuay(){
            if (read_manual == 'b' && daoChieuBanh == 0)
            {
                digitalWrite(pinDaoBanhA,1);
                digitalWrite(pinDaoBanhB,1);
                daoChieuBanh = 1;
                delay(delay_dc);
                digitalWrite(pinDaoBanhA,0);
                digitalWrite(pinDaoBanhB,0);
            }
            if((read_manual == 'f'|| read_manual == 'l'|| read_manual == 'r') && daoChieuBanh == 1){
                digitalWrite(pinDaoBanhA,1);
                digitalWrite(pinDaoBanhB,1);
                daoChieuBanh = 0;
                delay(delay_dc);
                digitalWrite(pinDaoBanhA,0);
                digitalWrite(pinDaoBanhB,0);
            }
            lcd_dao_banh();
        }


        // Hàm đảo chiều quay bánh khi đang ở trạng thái lùi khi chuyển chế độ auto
        void daoBanhQuayAUTO(){
            if (daoChieuBanh == 1)
            {
                digitalWrite(pinDaoBanhA,1);
                digitalWrite(pinDaoBanhB,1);
                delay(delay_dc);
                digitalWrite(pinDaoBanhA,0);
                digitalWrite(pinDaoBanhB,0);
                daoChieuBanh = 0;
            }
            lcd_dao_banh();
        }

        // Hàm đảo bánh thủ công
        void Dao_Banh_thu_cong(){
            last_DaoBanhTC = digitalRead(pinDaoBanhTC);
            if (last_DaoBanhTC == 0 && before_DaoBanhTC == 1)
            {
                digitalWrite(pinDaoBanhA,1);
                digitalWrite(pinDaoBanhB,1);
                delay(delay_dc);
                digitalWrite(pinDaoBanhA,0);
                digitalWrite(pinDaoBanhB,0);
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
            main::lcd::lcd.setCursor(5,1);
            main::lcd::lcd.print("          ");
            main::lcd::lcd.setCursor(5,1);
            main::lcd::lcd.print("Di thang");
            main::PWM::PWM_16Bit_Frequency(pinDkBanhA, outSpeedBanhA);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhB, outSpeedBanhB);
        }
        // ĐI LÙI
        void moveBack(){
            main::lcd::lcd.setCursor(6,1);
            main::lcd::lcd.print("          ");
            main::lcd::lcd.setCursor(6,1);
            main::lcd::lcd.print("Di lui");
            main::PWM::PWM_16Bit_Frequency(pinDkBanhA, outSpeedBanhA);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhB, outSpeedBanhB);
        }
        // SANG TRÁI
        void moveLeft(){
            main::lcd::lcd.setCursor(5,1);
            main::lcd::lcd.print("          ");
            main::lcd::lcd.setCursor(5,1);
            main::lcd::lcd.print("Sang trai");
            main::PWM::PWM_16Bit_Frequency(pinDkBanhA, outSpeedBanhA - outSpeedBanhA);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhB, outSpeedBanhB);
        }
        // SANG PHẢI
        void moveRight(){
            main::lcd::lcd.setCursor(5,1);
            main::lcd::lcd.print("          ");
            main::lcd::lcd.setCursor(5,1);
            main::lcd::lcd.print("Sang phai");
            main::PWM::PWM_16Bit_Frequency(pinDkBanhA, outSpeedBanhA);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhB, outSpeedBanhB - outSpeedBanhB);
        }
        // STOP
            void modeStop(){
            outSpeedBanhA = 0;
            outSpeedBanhB = 0;
            main::lcd::lcd.setCursor(5,1);
            main::lcd::lcd.print("          ");
            main::lcd::lcd.setCursor(7,1);
            main::lcd::lcd.print("Stop");
            main::PWM::PWM_16Bit_Frequency(pinDkBanhA, outSpeedBanhA - outSpeedBanhA);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhB, outSpeedBanhB - outSpeedBanhB);
        }

    } // namespace manual
    
} // namespace main
