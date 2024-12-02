#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  

// biến chọn chế độ menu
volatile bool value_chooseMenu = 0;
volatile bool value_selectChoose = 0;
volatile bool chooseChanged = false;  // Cờ để báo rằng menu đã thay đổi
uint8_t chooseStatus = 0;
uint8_t statusWork = 0;

// kí tự mũi tên
uint8_t muiTen[] = { B10000, B11000, B11100, B11110, B11110, B11100, B11000, B10000 };

// chân chọn chế độ
#define pin_choose 19
#define pin_select 18


// hàm ngắt
void chooseMenu(){
  value_chooseMenu = !value_chooseMenu;
  chooseChanged = true;
}

void selectChoose(){
  value_selectChoose = !value_selectChoose;
  chooseChanged = true;
}

void menuSelect(){
  if(value_chooseMenu == 1)
  {
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
    
  }


}

// chế độ hoạt động
void che_do_da_chon(){
  menuSelect();
  
  // kết quả
  if (value_chooseMenu == 0 && chooseStatus == 0)
  {
    if(chooseChanged == true){
      lcd.clear();
    }
    // hành động 1: AUTO
    lcd.setCursor(7, 0);
    lcd.print("AUTO");
  }

  if (value_chooseMenu == 0 && chooseStatus == 1)
  {
    if(chooseChanged == true){
      lcd.clear();
    }
    // hành động 2: MANUAL 
    lcd.setCursor(7, 0);
    lcd.print("MANUAL");
  }
  chooseChanged = false;
}



void setup() {
  Serial.begin(9600);
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.createChar(0, muiTen);

  // khai báo chân dùng ngắt
  pinMode(pin_select, INPUT_PULLUP);
  pinMode(pin_choose,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pin_select),selectChoose,FALLING);    // ngắt lựa chọn
  attachInterrupt(digitalPinToInterrupt(pin_choose),chooseMenu,FALLING);    // ngắt chọn
}

void loop() {
  che_do_da_chon();
  Serial.println(chooseStatus);
}

