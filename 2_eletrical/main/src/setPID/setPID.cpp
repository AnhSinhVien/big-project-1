#include "init.h"
#include "GlobalsSpeed.h"

namespace main
{
    namespace setPID
    {
        String data;
        void readDataBluetooth(){
            if (Serial3.available()) {
                // Đọc dữ liệu từ HC-05
                data = Serial3.readString();
                Serial.print("data: ");
                Serial.println(data); // In với 8 chữ số thập phân

                // Hiển thị dữ liệu lên Serial Monitor
                float value1, value2, value3;
                int spaceIndex1 = data.indexOf(' '); // Vị trí khoảng trắng đầu tiên
                int spaceIndex2 = data.indexOf(' ', spaceIndex1 + 1); // Vị trí khoảng trắng thứ hai
                    
                // Chuyển đổi các giá trị từ chuỗi sang float
                value1 = data.substring(0, spaceIndex1).toFloat();
                value2 = data.substring(spaceIndex1 + 1, spaceIndex2).toFloat();
                value3 = data.substring(spaceIndex2 + 1).toFloat();

                if ((value1 + value2 +value3) != 0)
                {
                    KP = value1;
                    KI = value2;
                    KD = value3;
                }   
                // In kết quả
                // Serial.println("Các giá trị float:");
                // Serial.println(readData.dataKp,7); // In với 8 chữ số thập phân
                // Serial.println(readData.dataKi,7);
                // Serial.println(readData.dataKd,7);
            }

            main::lcd::lcd.setCursor(0,0);
            main::lcd::lcd.print("Kp: ");    main::lcd::lcd.print(KP,2);
            main::lcd::lcd.setCursor(0,1);
            main::lcd::lcd.print("Ki: ");    main::lcd::lcd.print(KI,5);
            main::lcd::lcd.setCursor(0,2);
            main::lcd::lcd.print("Kd: ");    main::lcd::lcd.print(KD,1);
        }
    } // namespace setPID
    
    
} // namespace main
