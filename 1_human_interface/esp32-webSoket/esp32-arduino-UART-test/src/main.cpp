#include<Arduino.h>


String receivedData = "";
int receivedDataNumber = 0;

void setup() {
  // Khởi tạo UART với tốc độ 9600 baud
  Serial.begin(9600);
  
}

void loop() {
  if (Serial.available()) {
    receivedData = Serial.readString();
    receivedDataNumber = receivedData.toInt();  // num sẽ là 123
    Serial.println(receivedData);
    Serial.println(receivedDataNumber);
  }

}
