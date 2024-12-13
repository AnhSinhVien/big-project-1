#include <Arduino.h>

struct dataBluetooth
{
  float dataKp;
  float dataKi;
  float dataKd;

};

dataBluetooth readData;

void readDataBluetooth();

void setup() {
  // Khởi tạo Serial để giao tiếp với máy tính
  Serial.begin(9600); 
  
  // Khởi tạo Serial3 để giao tiếp với HC-05
  Serial3.begin(9600); 

  // Thông báo sẵn sàng
  Serial.println("Bluetooth HC-05 đang chờ dữ liệu...");
}

void loop() {
  // Kiểm tra xem có dữ liệu từ HC-05 qua Serial3 không
  readDataBluetooth();
}


void readDataBluetooth(){
  if (Serial3.available()) {
    // Đọc dữ liệu từ HC-05
    String data = Serial3.readString();
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
      readData.dataKp = value1;
      readData.dataKi = value2;
      readData.dataKd = value3;
    }
    

    // In kết quả
    Serial.println("Các giá trị float:");
    Serial.println(readData.dataKp,7); // In với 8 chữ số thập phân
    Serial.println(readData.dataKi,7);
    Serial.println(readData.dataKd,7);
  }
}