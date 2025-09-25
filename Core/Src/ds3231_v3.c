
/*
 * ds3231_v3.c  (STM32C0xx-ready)
 *
 * Implementation of DS3231 driver (v3)
 * - Adds Control/Status helpers and EnsureInitialized() using OSF bit
 */

#include "ds3231_v3.h"
#include "string.h"

static I2C_HandleTypeDef *hI2C = NULL;

/* Helpers: BCD <-> Binary */
uint8_t DS3231_BCD2BIN(uint8_t val){ return (uint8_t)((val>>4)*10 + (val&0x0F)); }
uint8_t DS3231_BIN2BCD(uint8_t val){ return (uint8_t)(((val/10)<<4) | (val%10)); }

void DS3231_Init(I2C_HandleTypeDef *hi2c){ hI2C = hi2c; }

/* Low-level R/W */
static HAL_StatusTypeDef ds_write(uint8_t reg, const uint8_t *pdata, uint16_t size){
    if (!hI2C) return HAL_ERROR;
    return HAL_I2C_Mem_Write(hI2C, DS3231_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t*)pdata, size, 1000);
}
static HAL_StatusTypeDef ds_read(uint8_t reg, uint8_t *pdata, uint16_t size){
    if (!hI2C) return HAL_ERROR;
    return HAL_I2C_Mem_Read(hI2C, DS3231_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, pdata, size, 1000);
}

/* Basic time I/O */
HAL_StatusTypeDef DS3231_SetTime(DS3231_TimeTypeDef *time){
    if (!time) return HAL_ERROR;
    uint8_t buf[7];
    buf[0]=DS3231_BIN2BCD(time->seconds);
    buf[1]=DS3231_BIN2BCD(time->minutes);
    buf[2]=DS3231_BIN2BCD(time->hours);
    buf[3]=DS3231_BIN2BCD(time->day);
    buf[4]=DS3231_BIN2BCD(time->date);
    buf[5]=DS3231_BIN2BCD(time->month);
    buf[6]=DS3231_BIN2BCD((uint8_t)(time->year%100));
    return ds_write(DS3231_REG_SECONDS, buf, 7);
}
HAL_StatusTypeDef DS3231_GetTime(DS3231_TimeTypeDef *time){
    uint8_t buf[7];
    HAL_StatusTypeDef st = ds_read(DS3231_REG_SECONDS, buf, 7);
    if (st!=HAL_OK) return st;
    time->seconds=(uint8_t)DS3231_BCD2BIN(buf[0]&0x7F);
    time->minutes=(uint8_t)DS3231_BCD2BIN(buf[1]&0x7F);
    time->hours  =(uint8_t)DS3231_BCD2BIN(buf[2]&0x3F); // 24h
    time->day    =(uint8_t)DS3231_BCD2BIN(buf[3]&0x07);
    time->date   =(uint8_t)DS3231_BCD2BIN(buf[4]&0x3F);
    time->month  =(uint8_t)DS3231_BCD2BIN(buf[5]&0x1F);
    time->year   =(uint16_t)(2000 + DS3231_BCD2BIN(buf[6]));
    return HAL_OK;
}

/* SQW control */
HAL_StatusTypeDef DS3231_Enable1HzSQW(void){
    uint8_t ctrl;
    HAL_StatusTypeDef st = ds_read(DS3231_REG_CONTROL, &ctrl, 1);
    if (st != HAL_OK) return st;
    ctrl &= ~DS3231_CONTROL_EOSC;             // ensure oscillator running
    ctrl &= ~DS3231_CONTROL_INTCN;            // route SQW
    ctrl &= ~DS3231_CONTROL_RS_MASK;          // 00 => 1Hz
    return ds_write(DS3231_REG_CONTROL, &ctrl, 1);
}
HAL_StatusTypeDef DS3231_DisableSQW(void){
    uint8_t ctrl;
    HAL_StatusTypeDef st = ds_read(DS3231_REG_CONTROL, &ctrl, 1);
    if (st != HAL_OK) return st;
    ctrl |= DS3231_CONTROL_INTCN;             // interrupt mode
    return ds_write(DS3231_REG_CONTROL, &ctrl, 1);
}

/* Temperature */
HAL_StatusTypeDef DS3231_ReadTemperature(float *temperature){
    uint8_t tb[2];
    HAL_StatusTypeDef st = ds_read(DS3231_REG_TEMP_MSB, tb, 2);
    if (st != HAL_OK) return st;
    int8_t msb = (int8_t)tb[0];
    uint8_t lsb = tb[1];
    *temperature = (float)msb + ((float)(lsb >> 6) * 0.25f);
    return HAL_OK;
}

/* NEW in v3: direct register helpers */
HAL_StatusTypeDef DS3231_ReadControl(uint8_t *val){ return ds_read(DS3231_REG_CONTROL, val, 1); }
HAL_StatusTypeDef DS3231_WriteControl(uint8_t val){ return ds_write(DS3231_REG_CONTROL, &val, 1); }
HAL_StatusTypeDef DS3231_ReadStatus(uint8_t *val){  return ds_read(DS3231_REG_STATUS,  val, 1); }
HAL_StatusTypeDef DS3231_WriteStatus(uint8_t val){  return ds_write(DS3231_REG_STATUS, &val, 1); }

/* NEW in v3: one-shot initializer based on OSF bit */
HAL_StatusTypeDef DS3231_EnsureInitialized(const DS3231_TimeTypeDef *default_time){
    HAL_StatusTypeDef st;
    uint8_t stat=0, ctrl=0;

    st = DS3231_ReadStatus(&stat);
    if (st != HAL_OK) return st;

    if (stat & DS3231_STATUS_OSF){
        /* Ensure oscillator and 1Hz SQW */
        st = DS3231_ReadControl(&ctrl);
        if (st != HAL_OK) return st;
        ctrl &= ~DS3231_CONTROL_EOSC;
        ctrl &= ~DS3231_CONTROL_INTCN;
        ctrl &= ~DS3231_CONTROL_RS_MASK; // 1Hz
        st = DS3231_WriteControl(ctrl);
        if (st != HAL_OK) return st;

        /* Write default time/date */
        if (default_time){
            DS3231_TimeTypeDef tmp = *default_time;
            st = DS3231_SetTime(&tmp);
            if (st != HAL_OK) return st;
        }

        /* Clear OSF so we don't re-init next boot */
        stat &= (uint8_t)~DS3231_STATUS_OSF;
        st = DS3231_WriteStatus(stat);
        if (st != HAL_OK) return st;
    }

    return HAL_OK;
}
