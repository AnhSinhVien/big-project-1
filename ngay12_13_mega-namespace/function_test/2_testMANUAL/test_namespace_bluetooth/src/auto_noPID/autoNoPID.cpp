#include "init.h"

namespace main
{
    namespace autoNoPID
    {
        // Chân cảm biến hồng ngoại
        byte TT_sensor_hn[5] = {0};
        const int che_do_chan_sensorHN[5] = {-2,-1,0,1,2};
        int kt_che_do_chan_sensorHN[5] = {0};

        // vị trí bàn
        byte vi_tri = 0;
        byte chong_nhieu1 = 0;

        // nhận thông tin lên esp32
        String receivedData = "";
        int receivedDataNumber = 0;

        // hàm setup
        void setup(){
            // BIẾN ĐIỀU KHIỂN AUTO
            for (int i = 0; i < 5; i++)
            {
                pinMode(sensor_hn[i], INPUT);
            }
        }

        // ĐỌC DỮ LIỆU TỪ ESP32 BÀNG UART2
        void read_data(){
        if ((Serial2.available()) && main::lcd::allow_reading_auto == 1) {
            receivedData = Serial2.readStringUntil('\n');                  // Đọc toàn bộ chuỗi đến khi gặp dấu ngắt dòng
            receivedDataNumber = receivedData.toInt();                    // Chuyển chuỗi thành số nguyên
        }
        }

        void modeAuto(){
            main::lcd::allow_reading_auto = 1;
            
            // ĐỂ ĐẢM BẢO LÀ DÙ CÓ KẾT THÚC CHẾ ĐỘ MANUAL Ở CHẾ ĐỘ LÙI THÌ SANG AUTO VẪN ĐI THẲNG
            main::manual::daoBanhQuayAUTO();

            // tính toán khoảng cách sensor HC_SR04
            main::HCSR04::calculateDistance();
            


            // trong trường hợp tại vị trí start và cần đợi lệnh
            while (receivedDataNumber == 0 && vi_tri == 0 && main::lcd::value_chooseMenu == 0)
            {
                read_data();
                main::lcd::lcd.setCursor(0,0);
                main::lcd::lcd.print("                ");
                main::lcd::lcd.setCursor(0,0);
                main::lcd::lcd.print("doi lenh");

                main::manual::modeStop();
            }
            

            // Biến kiểm tra đến vạch
            byte dem_vach = 0;
            // Điều kiện khoảng cách
            if(main::HCSR04::sensorData.distance >= 7){
                // Đọc cảm biến hồng ngoại
                for (int i = 0; i < 5; i++)
                {
                TT_sensor_hn[i] = digitalRead(sensor_hn[i]);
                // kt xem đang ở line nào
                if(TT_sensor_hn[i] == 0){
                    // kt chế độ chân line và gán giá trị
                    kt_che_do_chan_sensorHN[i] = che_do_chan_sensorHN[i];
                }
                else{
                    kt_che_do_chan_sensorHN[i] = 0;
                    dem_vach++;
                }
                }
                // tính tổng và tìm ra đường đi
                int sum = kt_che_do_chan_sensorHN[0]+kt_che_do_chan_sensorHN[1]+kt_che_do_chan_sensorHN[2]+kt_che_do_chan_sensorHN[3]+kt_che_do_chan_sensorHN[4];
                //Serial.println(sum);

                // vị trí bàn
                if (dem_vach == 5 && chong_nhieu1 == 0)
                {
                vi_tri++;
                chong_nhieu1 = 1;
                if (vi_tri == 7)
                {
                    vi_tri = 0;
                }
                Serial2.println(String(vi_tri));
            
                }
                if (dem_vach != 5)
                {
                chong_nhieu1 = 0;
                }
                //Serial.print("bàn: ");
                //Serial.println(vi_tri);
                //Serial.print(" - ");

                // hướng di chuyển
                if (sum>0)
                {
                //Serial.println("Sang trái");
                main::manual::moveLeft();
                }
                else if (sum<0)
                {
                //Serial.println("Sang phải");
                main::manual::moveRight();
                }
                else
                {
                //Serial.println("Đi thẳng");
                main::manual::moveForward();
                }
                
                delay(7);
            }
            else{
                //Serial.println("Stop");
                main::manual::modeStop();
            }

            // in ra màn hình LCD16x2
            main::lcd::lcd.setCursor(0,0);
            main::lcd::lcd.print(main::HCSR04::sensorData.distance);
            main::lcd::lcd.print("cm - ");
            main::lcd::lcd.setCursor(7,0);
            main::lcd::lcd.print("ban: ");
            main::lcd::lcd.print(vi_tri);

            // Đến vị trí bàn được gửi
            int dem_time_dung_tai_table = 1000;
            if(receivedDataNumber != 0){
                while (receivedDataNumber == vi_tri && (dem_time_dung_tai_table--) !=0 )
                {
                main::lcd::lcd.setCursor(0,0);
                main::lcd::lcd.print(dem_time_dung_tai_table);
                main::lcd::lcd.print("s");

                main::manual::modeStop();

                delay(3);
                // trở về quầy
                if(dem_time_dung_tai_table<=0){
                    receivedDataNumber = 0;
                }
                
                }
            }
        }



    } // namespace autoNoPID
    
} // namespace main
