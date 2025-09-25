
/*
 * ds3231_v3.h  (STM32C0xx-ready)
 *
 * DS3231 RTC driver for STM32 HAL
 * - Based on v2; adds Control/Status read-write helpers
 * - Adds one-shot initializer using OSF flag
 */

#ifndef __DS3231_V3_H__
#define __DS3231_V3_H__

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

/* STATUS bit7: OSF (Oscillator Stop Flag). 1 => time invalid / not initialized */
#define DS3231_STATUS_OSF      (1u << 7)
/* CONTROL bit7: EOSC (Enable Oscillator). 0 => run, 1 => stop */
#define DS3231_CONTROL_EOSC    (1u << 7)
/* CONTROL bit2: INTCN. 0 => SQW output, 1 => interrupt */
#define DS3231_CONTROL_INTCN   (1u << 2)
/* CONTROL bits3..4: RS rate select. 00=>1Hz, 01=>1.024kHz, 10=>4.096kHz, 11=>8.192kHz */
#define DS3231_CONTROL_RS_MASK ((1u << 3) | (1u << 4))

typedef struct {
    uint8_t seconds; // 0-59
    uint8_t minutes; // 0-59
    uint8_t hours;   // 0-23
    uint8_t day;     // 1-7
    uint8_t date;    // 1-31
    uint8_t month;   // 1-12
    uint16_t year;   // e.g., 2025
} DS3231_TimeTypeDef;

/* Init with HAL I2C handle */
void DS3231_Init(I2C_HandleTypeDef *hi2c);

/* Basic time I/O */
HAL_StatusTypeDef DS3231_SetTime(DS3231_TimeTypeDef *time);
HAL_StatusTypeDef DS3231_GetTime(DS3231_TimeTypeDef *time);

/* SQW control */
HAL_StatusTypeDef DS3231_Enable1HzSQW(void);
HAL_StatusTypeDef DS3231_DisableSQW(void);

/* Temperature */
HAL_StatusTypeDef DS3231_ReadTemperature(float *temperature);

/* BCD helpers */
uint8_t DS3231_BCD2BIN(uint8_t val);
uint8_t DS3231_BIN2BCD(uint8_t val);

/* NEW in v3: direct register access */
HAL_StatusTypeDef DS3231_ReadControl(uint8_t *val);
HAL_StatusTypeDef DS3231_WriteControl(uint8_t val);
HAL_StatusTypeDef DS3231_ReadStatus(uint8_t *val);
HAL_StatusTypeDef DS3231_WriteStatus(uint8_t val);

/* NEW in v3: one-shot initializer
 * If OSF=1 (time invalid), this writes default time/date, enables SQW@1Hz, clears OSF.
 * Returns HAL_OK if ready to use afterwards.
 */
HAL_StatusTypeDef DS3231_EnsureInitialized(const DS3231_TimeTypeDef *default_time);

#endif // __DS3231_V3_H__
