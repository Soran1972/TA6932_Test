
/*
 * ds3231_v2.h  (STM32C0xx-ready)
 *
 * Simple DS3231 RTC driver for STM32 (HAL) — same API as v1
 * Changes: HAL include -> stm32c0xx_hal.h
 */

#ifndef __DS3231_V2_H__
#define __DS3231_V2_H__

#include "stdint.h"
#include "stm32c0xx_hal.h" // STM32C0xx family

#define DS3231_I2C_ADDR        (0x68 << 1) // HAL expects 8-bit address
#define DS3231_REG_SECONDS     0x00
#define DS3231_REG_MINUTES     0x01
#define DS3231_REG_HOURS       0x02
#define DS3231_REG_DAY         0x03
#define DS3231_REG_DATE        0x04
#define DS3231_REG_MONTH       0x05
#define DS3231_REG_YEAR        0x06
#define DS3231_REG_CONTROL     0x0E
#define DS3231_REG_STATUS      0x0F
#define DS3231_REG_TEMP_MSB    0x11
#define DS3231_REG_TEMP_LSB    0x12

typedef struct {
    uint8_t seconds; // 0-59
    uint8_t minutes; // 0-59
    uint8_t hours;   // 0-23
    uint8_t day;     // 1-7
    uint8_t date;    // 1-31
    uint8_t month;   // 1-12
    uint16_t year;   // e.g., 2025
} DS3231_TimeTypeDef;

void DS3231_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef DS3231_SetTime(DS3231_TimeTypeDef *time);
HAL_StatusTypeDef DS3231_GetTime(DS3231_TimeTypeDef *time);
HAL_StatusTypeDef DS3231_Enable1HzSQW(void);
HAL_StatusTypeDef DS3231_DisableSQW(void);
HAL_StatusTypeDef DS3231_ReadTemperature(float *temperature);
uint8_t DS3231_BCD2BIN(uint8_t val);
uint8_t DS3231_BIN2BCD(uint8_t val);

#endif // __DS3231_V2_H__
