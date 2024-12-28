#include "init.h"
#include "GlobalsSpeed.h"

namespace main
{
    namespace autoPID
    {
        // Chân cảm biến hồng ngoại
        byte TT_sensor_hn[5] = {0};
        const int che_do_chan_sensorHN[5] = {-2,-1,0,1,2};
        int kt_che_do_chan_sensorHN[5] = {0};

        // các mức tốc độ
        #define STOP 65 

        // Các mức tốc độ pid
        #define BASE_SPEED 75  // Tốc độ cơ sở
        #define MAX_SPEED 80   // Tốc độ tối đa
        #define MIN_SPEED 72   // Tốc độ tối thiểu

        // biến tốc độ điều chỉnh
        uint16_t speedA, speedB;             // biến dùng tăng giảm tốc độ

        // PID điều khiển hướng
        //float KP = 1, KI = 0.000007, KD = 23;  // Hệ số PID
        struct PIDcar
        {
            int P = 0, I = 0, D = 0;
            float Kp = KP, Ki = KI, Kd = KD;  
            float PID_value;
            int errorPrev; // Lưu giá trị "error" trước đó
        };

        PIDcar car;

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

        // quản lí xoá màn hình LCD
        bool xoaHienThi1 = 0;
        bool xoaHienThi2 = 0;

        // time dùng lại tại vị trí đến
        uint32_t timeTable = 0;
        uint16_t timeStop = 10000;
        


        // hàm setup
        void setup(){

            Stop();
            main::lcd::lcd.setCursor(0,1);
            main::lcd::lcd.print("A: ");
            main::lcd::lcd.setCursor(3, 1);
            if (speedA < 10) main::lcd::lcd.print("0");  // Thêm số 0 nếu dưới 10
            main::lcd::lcd.print(speedA);
            //Bánh B
            main::lcd::lcd.setCursor(8,1);
            main::lcd::lcd.print("B: ");
            main::lcd::lcd.setCursor(11, 1);
            if (speedB < 10) main::lcd::lcd.print("0");  // Thêm số 0 nếu dưới 10
            main::lcd::lcd.print(speedB);

            Serial.print("A: "); Serial.print(speedA);
            Serial.print(" - ");
            Serial.print("B: "); Serial.println(speedB);
            //delay(500);
        }

        // ĐỌC DỮ LIỆU TỪ ESP32 BÀNG UART2
        void read_data(){
            if ((Serial2.available()) && main::lcd::allow_reading_auto == 1) {
                receivedData = Serial2.readStringUntil('\n');                  // Đọc toàn bộ chuỗi đến khi gặp dấu ngắt dòng
                receivedDataNumber = receivedData.toInt();                    // Chuyển chuỗi thành số nguyên
                main::lcd::lcd.setCursor(0,3);
                main::lcd::lcd.print("RDN: ");
                main::lcd::lcd.print(receivedDataNumber);
            }
        }

        // xe dừng 
        void Stop(){
            speedA = STOP;
            speedB = STOP;
            main::PWM::PWM_16Bit_Frequency(pinDkBanhA, speedA);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhB, speedB);
        }


        // chuyển hướng trái phải
        void CustomHuong(uint8_t PWM_A, uint8_t PWM_B){
            speedA = PWM_A;      // tốc độ bánh A
            speedB = PWM_B;      // tóc độ bánh B
            main::PWM::PWM_16Bit_Frequency(pinDkBanhA, speedA);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhB, speedB);
        }

        void PID_Control(int8_t errorNow) {
            car.Kp = KP;
            car.Ki = KI;
            car.Kd = KD;
            // Tính toán PID
            car.P = errorNow;
            //car.I += errorNow;
            car.I = 0.9 * car.I + errorNow;  // bộ lọc trung bình
            car.D = errorNow - car.errorPrev;
            // Tính toán giá trị PID
            int16_t temp = constrain(car.I, -30, 30); 
            car.I  = temp;

            // Tính toán giá trị PID
            car.PID_value = (car.Kp * car.P) + (car.Ki * car.I) + (car.Kd * car.D);

            // Cập nhật tốc độ bánh xe dựa trên correction
            speedA = constrain(BASE_SPEED + car.PID_value, MIN_SPEED, MAX_SPEED);
            speedB = constrain(BASE_SPEED - car.PID_value, MIN_SPEED, MAX_SPEED);

            // Gửi tốc độ đến động cơ
            main::PWM::PWM_16Bit_Frequency(pinDkBanhA, speedA);
            main::PWM::PWM_16Bit_Frequency(pinDkBanhB, speedB);

            car.errorPrev = errorNow;
        }


        
        void modeAuto(){
            main::lcd::allow_reading_auto = 1;
            
            // ĐỂ ĐẢM BẢO LÀ DÙ CÓ KẾT THÚC CHẾ ĐỘ MANUAL Ở CHẾ ĐỘ LÙI THÌ SANG AUTO VẪN ĐI THẲNG
            // main::manual::daoBanhQuayAUTO();

            // tính toán khoảng cách sensor HC_SR04
            main::HCSR04::calculateDistance();
            //read_data();
            
            // trong trường hợp tại vị trí start và cần đợi lệnh
            while (receivedDataNumber == 0 && vi_tri == 0 && main::lcd::value_chooseMenu == 0)
            {
                read_data();
                //receivedDataNumber = 3;
                // xoá kí tự thừa của giao diện chờ hoạt động
                if (xoaHienThi1 == 0)
                {
                    main::lcd::lcd.clear();
                    xoaHienThi1 = 1;
                }
                main::lcd::lcd.setCursor(6,0);
                main::lcd::lcd.print("doi lenh");
                // Bánh A
                main::lcd::lcd.setCursor(0,1);
                main::lcd::lcd.print("A: ");
                main::lcd::lcd.setCursor(3, 1);
                main::lcd::lcd.print(speedA);
                //Bánh B
                main::lcd::lcd.setCursor(8,1);
                main::lcd::lcd.print("B: ");
                main::lcd::lcd.setCursor(11, 1);
                main::lcd::lcd.print(speedB);

                Stop();
                // cài lại biến xoá hiển thị
                xoaHienThi2 = 0;
            }
            // cài lại biến xoá hiển thị
            xoaHienThi1 = 0;

            //xoá kí tự thừa của giao diện chờ lệnh
            if (xoaHienThi2 == 0)
            {
                main::lcd::lcd.clear();
                xoaHienThi2 = 1;
            }
            

            // Biến kiểm tra đến vạch
            byte dem_vach = 0;
            //main::HCSR04::sensorData.distance = 10;
            // Điều kiện khoảng cách
            if(main::HCSR04::sensorData.distance >= 20)
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
                        Serial2.println(String(vi_tri)); 
                    }
                    // khi bị nhấc lên khỏi line
                    if (millis() - capPoint2 >= TIME_OUT)
                    {
                        dung = 1;
                        Stop();
                        car.I = 0;
                    }
                }
                if (dem_vach != 0)
                {
                    capPoint2 = millis();
                    chong_nhieu1 = 0;
                    dung = 0;
                }
                Serial.print("bàn: ");
                Serial.println(vi_tri);
                Serial.print(" - ");

                // biến 'direction' sẽ gán giá trị cuối cùng khi xe rời vạch nhằm tìm lại đường
                // ngoài đường line 
                if (dem_vach == 5)
                {
                    if (millis() - capPoint >= TIME_OUT)
                    {
                        Stop();
                        car.I = 0;
                    }
                }
                // khi xe trong đường line
                else
                {
                    if (dung == 0)
                    {
                        capPoint = millis();
                        // Điều khiển robot theo PID
                        switch (sum)
                        {
                            /* ------------------------ 11110 ------------------------ */
                            case 2:
                                direction = 3;
                                break;
                            /* ------------------------ 11100 ------------------------ */
                            case 3:
                                direction = 2;
                                break;
                            /* ------------------------ 11101 ------------------------ */
                            case 1:
                                direction = 1;
                                break;
                            /* ------------------------ 11011 ------------------------ */
                            case 0:
                                direction = 0;
                                break;
                            /* ------------------------ 10111 ------------------------ */
                            case -1:
                                direction = -1;
                                break;
                            /* ------------------------ 00111 ------------------------ */
                            case -3:
                                direction = -2;
                                break;
                            /* ------------------------ 01111 ------------------------ */
                            case -2:
                                direction = -3;
                                break;
                            /* ------------------------ Các trường hợp khác ------------------------ */
                            default:
                                break;
                        }
                        PID_Control(direction);
                        
                    }
                }
            }
            else
            {
                //Serial.println("Stop");
                Stop();
            }

            // in ra màn hình LCD16x2
            main::lcd::lcd.setCursor(0,0);
            if (main::HCSR04::sensorData.distance < 10)
            {
                main::lcd::lcd.print("0");  // Thêm số 0 nếu dưới 10
            }
            
            main::lcd::lcd.print(main::HCSR04::sensorData.distance);
            main::lcd::lcd.print("cm - ");
            main::lcd::lcd.setCursor(7,0);
            main::lcd::lcd.print("ban: ");
            main::lcd::lcd.print(vi_tri);

            // hiển thị tốc độ 
            // Bánh A
            main::lcd::lcd.setCursor(0,1);
            main::lcd::lcd.print("A: ");
            main::lcd::lcd.setCursor(3, 1);
            if (speedA < 10) main::lcd::lcd.print("0");  // Thêm số 0 nếu dưới 10
            main::lcd::lcd.print(speedA);
            //Bánh B
            main::lcd::lcd.setCursor(8,1);
            main::lcd::lcd.print("B: ");
            main::lcd::lcd.setCursor(11, 1);
            if (speedB < 10) main::lcd::lcd.print("0");  // Thêm số 0 nếu dưới 10
            main::lcd::lcd.print(speedB);


            // Đến vị trí bàn được gửi
             // Đến vị trí bàn được gửi
            timeTable = millis();
            if(receivedDataNumber != 0)
            {
                if (receivedDataNumber == vi_tri)
                {
                    if (xoaHienThi2 == 1)
                    {
                        main::lcd::lcd.clear();
                        xoaHienThi2 = 0;
                    }
                    
                    while (millis() - timeTable < timeStop)
                    {
                        // in ra màn hình LCD16x2
                        main::lcd::lcd.setCursor(0,0);
                        if (main::HCSR04::sensorData.distance < 10)
                        {
                            main::lcd::lcd.print("0");  // Thêm số 0 nếu dưới 10
                        }
                        
                        main::lcd::lcd.print(main::HCSR04::sensorData.distance);
                        main::lcd::lcd.print("cm - ");
                        main::lcd::lcd.setCursor(7,0);
                        main::lcd::lcd.print("ban: ");
                        main::lcd::lcd.print(vi_tri);

                        main::lcd::lcd.setCursor(0,2);
                        main::lcd::lcd.print("stop: ");
                        // Bánh A
                        main::lcd::lcd.setCursor(0,1);
                        main::lcd::lcd.print("A: ");
                        main::lcd::lcd.setCursor(3, 1);
                        main::lcd::lcd.print(speedA);
                        //Bánh B
                        main::lcd::lcd.setCursor(8,1);
                        main::lcd::lcd.print("B: ");
                        main::lcd::lcd.setCursor(11, 1);
                        main::lcd::lcd.print(speedB);

                        Stop();
                    }
                    
                    // trở về quầy
                    receivedDataNumber = 0;
                }
            }

            
            Serial.print("A: "); Serial.print(speedA);
            Serial.print(" - ");
            Serial.print("B: "); Serial.print(speedB);
            Serial.print(" - ");Serial.println(car.PID_value);
        }



    } // namespace autoNoPID
    
} // namespace main
