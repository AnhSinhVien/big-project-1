#ifndef GLOBALS_SPEED_H 
#define GLOBALS_SPEED_H 

#include "HardwareSerial.h"

// 2 chân điều khiển động cơ
extern volatile uint16_t* pinDkBanhA;       // Bánh trái (chân 5)
extern volatile uint16_t* pinDkBanhB;       // Bánh phải (chân 3)

// tốc độ động cơ
extern uint16_t outSpeedBanhA;
extern uint16_t outSpeedBanhB;

#endif // GLOBALS_SPEED_H 