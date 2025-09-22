#ifndef __TA6932_H
#define __TA6932_H

#include "stm32C0xx_hal.h" // change to your MCU family header

// Configure STB pin here
#define TA_STB_PORT  GPIOA
#define TA_STB_PIN   GPIO_PIN_4

// API
void TA6932_Init(void);
void TA6932_TestPattern(void);
void TA6932_WriteAll(void);
void TA6932_DisplayOn(uint8_t br);
void TA6932_CounterDemo(void);

#endif
