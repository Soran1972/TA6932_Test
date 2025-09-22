#include "ta6932.h"

extern SPI_HandleTypeDef hspi1; // Use SPI1 from CubeMX

static inline void TA_STB(int v){
  HAL_GPIO_WritePin(TA_STB_PORT, TA_STB_PIN, v ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static HAL_StatusTypeDef TA_sendByte(uint8_t b){
  return HAL_SPI_Transmit(&hspi1, &b, 1, 10);
}

static void TA_cmd(uint8_t cmd){
  TA_STB(0);
  TA_sendByte(cmd);
  TA_STB(1);
}

static void TA_writeSeq(uint8_t startAddr, const uint8_t *data, uint8_t len){
  TA_cmd(0x40); // write, auto-inc
  TA_STB(0);
  uint8_t a = 0xC0 | (startAddr & 0x0F);
  TA_sendByte(a);
  for(uint8_t i=0;i<len && i<16;i++) TA_sendByte(data[i]);
  TA_STB(1);
}

void TA6932_DisplayOn(uint8_t br){
  if(br>7) br=7;
  TA_cmd(0x88 | br);
}

static const uint8_t SEG_FONT[12] = {
  0x3F,0x21,0x5D,0x75,0x63,0x76,0x7E,0x25,0x7F,0x77,0x00,0x40
};

static uint8_t g_buf[16];

static inline void putRaw(uint8_t addr, uint8_t v){ g_buf[addr & 0x0F] = v; }
static inline void putDigit(uint8_t addr, int d, int dp){
  uint8_t v = (d>=0 && d<=9)? SEG_FONT[d] : SEG_FONT[10];
  if(dp) v |= 0x80;
  putRaw(addr, v);
}

void TA6932_Init(void){
  TA_STB(1);
  TA6932_DisplayOn(7);
}

void TA6932_WriteAll(void){
  TA_writeSeq(0x00, g_buf, 16);
}

void TA6932_TestPattern(void){
  putDigit(0x00,1,0);
  putDigit(0x01,2,0);
  putDigit(0x02,3,0);
  putDigit(0x03,4,0);
  putDigit(0x04,5,0);
  putDigit(0x05,6,0);
  putDigit(0x06,2,0);
  putDigit(0x07,0,0);
  putDigit(0x08,2,0);
  putDigit(0x09,5,0);
  putDigit(0x0A,0,0);
  putDigit(0x0B,9,0);
  putDigit(0x0C,2,0);
  putDigit(0x0D,2,0);
  putRaw(0x0E,0x80); // colon on
  putRaw(0x0F,0x00);
  TA6932_WriteAll();
}

void TA6932_CounterDemo(void){
  for(int d=0; d<10; d++){
    for(int addr=0; addr<14; addr++){
      putDigit(addr, d, 0);
    }
    putRaw(0x0E, 0x80); // colon ON
    putRaw(0x0F, 0x00); // weekday off
    TA6932_WriteAll();
    HAL_Delay(1000); // 1 second delay
  }
}
