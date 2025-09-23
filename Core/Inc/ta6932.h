#ifndef __TA6932_H
#define __TA6932_H

// >>> Controller: STM32C011F6P6 (STM32C0 series) <<<
#include "stm32c0xx_hal.h"   // Correct HAL family header for STM32C0

// STB pin (edit to your board if different)
#define TA_STB_PORT  GPIOA
#define TA_STB_PIN   GPIO_PIN_4

// API
void TA6932_Init(void);
void TA6932_WriteAll(void);
void TA6932_TestPattern(void);
void TA6932_CounterDemo(void);


// Buffer helpers
void TA6932_putRaw(uint8_t addr, uint8_t v);      // write raw pattern to back buffer (no transmit)
void TA6932_putDigit(uint8_t addr, int d, int dp);// 0..9 to back buffer
void TA6932_putChar(uint8_t addr, char ch, int dp);// limited ASCII to back buffer
void TA6932_Clear(void);                           // clear buffer and WriteAll

// Single-digit direct write (fixed-address)
void TA6932_WriteOneRaw(uint8_t addr, uint8_t value);
void TA6932_putDigitOne(uint8_t addr, int d, int dp);
void TA6932_putCharOne(uint8_t addr, char ch, int dp);

// Brightness / ON/OFF
void TA6932_SetBrightness(uint8_t level); // 0..7
void TA6932_DisplayOn(void);
void TA6932_DisplayOff(void);

#endif
