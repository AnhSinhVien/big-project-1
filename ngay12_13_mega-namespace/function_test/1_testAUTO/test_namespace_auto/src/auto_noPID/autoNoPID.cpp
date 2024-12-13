#include "init.h"

namespace main
{
    namespace autoNoPID
    {
        // Chân cảm biến hồng ngoại
        byte TT_sensor_hn[5] = {0};
        const int che_do_chan_sensorHN[5] = {-2,-1,0,1,2};
        int kt_che_do_chan_sensorHN[5] = {0};

        // các mức tốc độ
        #define FAST 85   //!
        #define NORMAL 80 //!
        #define SLOW 75   //!
        #define STOP 65 

        // biến tốc độ điều chỉnh
        uint16_t speedA, speedB;             // biến dùng tăng giảm tốc độ

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

        // xe dừng 
        void Stop(){
            speedA = STOP;
            speedB = STOP;
            main::PWM::PWM_16Bit_Frequency(main::manual::pinDkBanhA, speedA);
            main::PWM::PWM_16Bit_Frequency(main::manual::pinDkBanhB, speedB);
        }

        // xe tiến
        void DiTien(){
            speedA = SLOW;      // tốc độ bánh A
            speedB = SLOW;      // tóc độ bánh B
            main::PWM::PWM_16Bit_Frequency(main::manual::pinDkBanhA, speedA);
            main::PWM::PWM_16Bit_Frequency(main::manual::pinDkBanhB, speedB);
        }

        // chuyển hướng trái phải
        void CustomHuong(uint8_t PWM_A, uint8_t PWM_B){
            speedA = PWM_A;      // tốc độ bánh A
            speedB = PWM_B;      // tóc độ bánh B
            main::PWM::PWM_16Bit_Frequency(main::manual::pinDkBanhA, speedA);
            main::PWM::PWM_16Bit_Frequency(main::manual::pinDkBanhB, speedB);
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

            // xoá kí tự thừa của giao diện chờ lệnh
            if (xoaHienThi2 == 0)
            {
                main::lcd::lcd.clear();
                xoaHienThi2 = 1;
            }
            

            // Biến kiểm tra đến vạch
            byte dem_vach = 0;
            // Điều kiện khoảng cách
            if(main::HCSR04::sensorData.distance >= 7)
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
                    }
                }
                if (dem_vach != 0)
                {
                    capPoint2 = millis();
                    chong_nhieu1 = 0;
                    dung = 0;
                }
                //Serial.print("bàn: ");
                //Serial.println(vi_tri);
                //Serial.print(" - ");

                // biến 'direction' sẽ gán giá trị cuối cùng khi xe rời vạch nhằm tìm lại đường
                // ngoài đường line 
                if (dem_vach == 5)
                {
                    if (millis() - capPoint >= TIME_OUT)
                    {
                        direction = 0;
                        Stop();
                    }
                    else
                    {
                        switch (direction)
                        {
                        case 3: // Lệch phải nhiều ... xoay phải vừa
                            CustomHuong(SLOW, FAST);
                            break;
                        case 2: // Lệch phải nhiều ... xoay phải nhanh
                            CustomHuong(STOP, NORMAL);
                            break;
                        case 1: // Lệch phải ít ... xoay phải chậm
                            CustomHuong(SLOW, NORMAL);
                            break;
                        case 0: // Giữa ... cứ đi thẳng tiếp
                            DiTien();
                            break;
                        case -1: // Lệch phải nhiều ... xoay trái chậm
                            CustomHuong(NORMAL, SLOW);
                            break;
                        case -2: // Lệch phải nhiều ... xoay trái nhanh
                            CustomHuong(NORMAL, STOP);
                            break;
                        case -3: // Lệch phải ít ... xoay trái vừa
                            CustomHuong(FAST, SLOW);
                            break;
                        }
                    }
                }
                // khi xe trong đường line
                else
                {
                    if (dung == 0)
                    {
                        capPoint = millis();
                        direction = sum;
                        switch (sum)
                        {
                        case 3: // Lệch phải nhiều ... xoay phải vừa
                        CustomHuong(SLOW, FAST);
                        break;
                        case 2: // Lệch phải nhiều ... xoay phải nhanh
                        CustomHuong(STOP, NORMAL);
                        break;
                        case 1: // Lệch phải ít ... xoay phải chậm
                        CustomHuong(SLOW, NORMAL);
                        break;
                        case 0: // Giữa ... cứ đi thẳng tiếp
                        DiTien();
                        break;
                        case -1: // Lệch phải nhiều ... xoay trái chậm
                        CustomHuong(NORMAL, SLOW);
                        break;
                        case -2: // Lệch phải nhiều ... xoay trái nhanh
                        CustomHuong(NORMAL, STOP);
                        break;
                        case -3: // Lệch phải ít ... xoay trái vừa
                        CustomHuong(FAST, SLOW);
                        break;
                        }
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
            int dem_time_dung_tai_table = 1000;
            if(receivedDataNumber != 0)
            {
                while (receivedDataNumber == vi_tri && (dem_time_dung_tai_table--) !=0 )
                {
                    main::lcd::lcd.setCursor(0,2);
                    main::lcd::lcd.print("stop: ");
                    main::lcd::lcd.setCursor(6,2);
                    if(dem_time_dung_tai_table < 10)
                    {
                       main::lcd::lcd.print("000"); 
                    }
                    else if (dem_time_dung_tai_table >= 10 && dem_time_dung_tai_table < 100)
                    {
                        main::lcd::lcd.print("00"); 
                    }
                    else if (dem_time_dung_tai_table >= 100 && dem_time_dung_tai_table < 1000)
                    {
                        main::lcd::lcd.print("0");
                    }
                    main::lcd::lcd.print(dem_time_dung_tai_table);
                    main::lcd::lcd.print("ms");

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

                    delay(2);
                    // trở về quầy
                    if(dem_time_dung_tai_table<=0){
                        receivedDataNumber = 0;
                    }
                }
            }
        }



    } // namespace autoNoPID
    
} // namespace main
