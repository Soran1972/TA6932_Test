
/*
 * ds3231.c
 *
 * Simple DS3231 RTC driver for STM32 (HAL)
 *
 * Comments are written in English for code clarity.
 */

#include "ds3231_v1.h"
#include "string.h"

static I2C_HandleTypeDef *hI2C = NULL;

/* Helper: BCD <-> binary */
uint8_t DS3231_BCD2BIN(uint8_t val) {
    return (val >> 4) * 10 + (val & 0x0F);
}

uint8_t DS3231_BIN2BCD(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

void DS3231_Init(I2C_HandleTypeDef *hi2c) {
    hI2C = hi2c;
}

/* Write multiple bytes to DS3231 register */
static HAL_StatusTypeDef ds_write(uint8_t reg, uint8_t *pdata, uint16_t size) {
    if (hI2C == NULL) return HAL_ERROR;
    return HAL_I2C_Mem_Write(hI2C, DS3231_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, pdata, size, 1000);
}

/* Read multiple bytes from DS3231 register */
static HAL_StatusTypeDef ds_read(uint8_t reg, uint8_t *pdata, uint16_t size) {
    if (hI2C == NULL) return HAL_ERROR;
    return HAL_I2C_Mem_Read(hI2C, DS3231_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, pdata, size, 1000);
}

HAL_StatusTypeDef DS3231_SetTime(DS3231_TimeTypeDef *time) {
    uint8_t buf[7];
    if (!time) return HAL_ERROR;

    buf[0] = DS3231_BIN2BCD(time->seconds);
    buf[1] = DS3231_BIN2BCD(time->minutes);
    buf[2] = DS3231_BIN2BCD(time->hours);
    buf[3] = DS3231_BIN2BCD(time->day);
    buf[4] = DS3231_BIN2BCD(time->date);
    buf[5] = DS3231_BIN2BCD(time->month);
    buf[6] = DS3231_BIN2BCD(time->year % 100);

    return ds_write(DS3231_REG_SECONDS, buf, 7);
}

HAL_StatusTypeDef DS3231_GetTime(DS3231_TimeTypeDef *time) {
    uint8_t buf[7];
    HAL_StatusTypeDef st = ds_read(DS3231_REG_SECONDS, buf, 7);
    if (st != HAL_OK) return st;

    time->seconds = DS3231_BCD2BIN(buf[0] & 0x7F);
    time->minutes = DS3231_BCD2BIN(buf[1] & 0x7F);
    /* Hours may contain 12/24 and AM/PM bits; handle only 24-hour mode here */
    time->hours   = DS3231_BCD2BIN(buf[2] & 0x3F);
    time->day     = DS3231_BCD2BIN(buf[3] & 0x07);
    time->date    = DS3231_BCD2BIN(buf[4] & 0x3F);
    time->month   = DS3231_BCD2BIN(buf[5] & 0x1F);
    time->year    = 2000 + DS3231_BCD2BIN(buf[6]);

    return HAL_OK;
}

HAL_StatusTypeDef DS3231_Enable1HzSQW(void) {
    uint8_t ctrl;
    HAL_StatusTypeDef st = ds_read(DS3231_REG_CONTROL, &ctrl, 1);
    if (st != HAL_OK) return st;

    /* Clear INTCN to enable square-wave output; RS1/RS2 = 00 => 1Hz */
    ctrl &= ~(1 << 2); // clear INTCN
    ctrl &= ~((1 << 3) | (1 << 4)); // RS1 = RS2 = 0
    return ds_write(DS3231_REG_CONTROL, &ctrl, 1);
}

HAL_StatusTypeDef DS3231_DisableSQW(void) {
    uint8_t ctrl;
    HAL_StatusTypeDef st = ds_read(DS3231_REG_CONTROL, &ctrl, 1);
    if (st != HAL_OK) return st;
    /* Set INTCN to route interrupts (disable square wave) */
    ctrl |= (1 << 2);
    return ds_write(DS3231_REG_CONTROL, &ctrl, 1);
}

HAL_StatusTypeDef DS3231_ReadTemperature(float *temperature) {
    uint8_t tb[2];
    HAL_StatusTypeDef st = ds_read(DS3231_REG_TEMP_MSB, tb, 2);
    if (st != HAL_OK) return st;

    int8_t msb = (int8_t)tb[0];
    uint8_t lsb = tb[1];

    /* Temperature is in MSB + (LSB >> 6) * 0.25 */
    *temperature = msb + ((lsb >> 6) * 0.25f);
    return HAL_OK;
}
