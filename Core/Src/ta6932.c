// TA6932 driver (with unified One-API + full legacy API)
// Version: 1.0
//Date:25/9/2025
// - يحافظ على الدوال السابقة (WriteAll/WriteOneRaw/...)
// - يحتوي على font7seg + setGlyph
// - يضيف الدوال الموحّدة: TA_RAW(), TA6932_putOne(), TA6932_putOneBuf()

#include "ta6932.h"

// SPI handle المُنشأ من CubeMX (عدّل لو تستخدم SPI ثاني)
extern SPI_HandleTypeDef hspi1;

// ===== Low-level =====
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
  TA_cmd(0x40); // Data set: write, auto-increment
  TA_STB(0);
  uint8_t a = 0xC0 | (startAddr & 0x0F);
  TA_sendByte(a);
  for(uint8_t i=0;i<len && i<16;i++) TA_sendByte(data[i]);
  TA_STB(1);
}

// ===== Font table (Common-Cathode; bit7 للـ dp خارجياً) =====
static uint8_t font7seg[128];   // ASCII -> 7-seg pattern (bit7 for dp)
static void font_init(void);

static void font_init(void){
  // digits '0'..'9' (حسب خريطتك CC)
  font7seg['0'] = 0x3F; font7seg['1'] = 0x21; font7seg['2'] = 0x5D; font7seg['3'] = 0x75; font7seg['4'] = 0x63;
  font7seg['5'] = 0x76; font7seg['6'] = 0x7E; font7seg['7'] = 0x25; font7seg['8'] = 0x7F; font7seg['9'] = 0x77;

  // رموز شائعة
  font7seg[' '] = 0x00; // فراغ
  font7seg['-'] = 0x40; // شرطة (g فقط)
  font7seg['_'] = 0x10; // underscore (d)

  // حروف تقريبية (يمكن تعديلها بـ setGlyph)
  font7seg['A'] = font7seg['a'] = 0x6F;
  font7seg['b'] = 0x7A;
  font7seg['C'] = 0x5A;
  font7seg['c'] = 0x58;
  font7seg['d'] = 0x79;
  font7seg['E'] = 0x5E;
  font7seg['F'] = 0x4E;
  font7seg['G'] = 0x7A;
  font7seg['H'] = 0x6C;
  font7seg['I'] = 0x24;
  font7seg['J'] = 0x31;
  font7seg['K'] = 0x6C;
  font7seg['L'] = 0x1A;
  font7seg['M'] = 0x2D;
  font7seg['N'] = 0x2C;
  font7seg['n'] = 0x68;
  font7seg['o'] = 0x78;
  font7seg['P'] = 0x67;
  font7seg['Q'] = 0x73;
  font7seg['r'] = 0x48;
  font7seg['S'] = 0x76;
  font7seg['t'] = 0x5E & ~0x20;
  font7seg['U'] = 0x3A;
  font7seg['V'] = 0x3A;
  font7seg['W'] = 0x3D;
  font7seg['X'] = 0x6C;
  font7seg['Y'] = 0x73;
  font7seg['Z'] = 0x5B;
}

// ===== Display control =====
static uint8_t s_brightness = 7; // آخر مستوى سطوع
static uint8_t g_buf[16];

void TA6932_SetBrightness(uint8_t level){  // 0..7
  if(level > 7) level = 7;
  s_brightness = level;
  TA_cmd(0x88 | (s_brightness & 0x07));  // Display ON + brightness
}
void TA6932_DisplayOn(void){
  TA_cmd(0x88 | (s_brightness & 0x07));
}
void TA6932_DisplayOff(void){
  TA_cmd(0x80); // OFF
}

// ===== Buffer helpers =====
void TA6932_putRaw(uint8_t addr, uint8_t v){ g_buf[addr & 0x0F] = v; }

void TA6932_putDigit(uint8_t addr, int d, int dp){
  uint8_t v = 0x00;
  if (d >= 0 && d <= 9) v = font7seg['0' + d];
  if (dp) v |= 0x80;
  TA6932_putRaw(addr, v);
}
void TA6932_putChar(uint8_t addr, char ch, int dp){
  uint8_t v = font7seg[(uint8_t)ch];
  if (v == 0x00 && ch != ' ') v = 0x00; // غير معرّف → فراغ
  if (dp) v |= 0x80;
  TA6932_putRaw(addr, v);
}
void TA6932_setGlyph(uint8_t ch, uint8_t pattern){
  font7seg[ch] = pattern & 0x7F; // bit7 للـ dp يُضاف خارجياً
}

// تعبئة البافر كامل (بدون memcpy حسب تفضيلك)
void TA6932_loadBuffer(const uint8_t *src){
  for (int i=0;i<16;i++) g_buf[i] = src[i];
}
void TA6932_loadBuffer16(uint8_t b0,uint8_t b1,uint8_t b2,uint8_t b3,
                         uint8_t b4,uint8_t b5,uint8_t b6,uint8_t b7,
                         uint8_t b8,uint8_t b9,uint8_t b10,uint8_t b11,
                         uint8_t b12,uint8_t b13,uint8_t b14,uint8_t b15){
  g_buf[0]=b0;  g_buf[1]=b1;  g_buf[2]=b2;  g_buf[3]=b3;
  g_buf[4]=b4;  g_buf[5]=b5;  g_buf[6]=b6;  g_buf[7]=b7;
  g_buf[8]=b8;  g_buf[9]=b9;  g_buf[10]=b10; g_buf[11]=b11;
  g_buf[12]=b12; g_buf[13]=b13; g_buf[14]=b14; g_buf[15]=b15;
}

// ===== Public API =====
void TA6932_Init(void){
  TA_STB(1);                 // STB idle HIGH
  font_init();               // تهيئة الفونت
  s_brightness = 7;
  TA6932_DisplayOn();        // تشغيل على سطوع 7
}
void TA6932_WriteAll(void){
  TA_writeSeq(0x00, g_buf, 16);
}
void TA6932_Clear(void){
  for (int i=0;i<16;i++) g_buf[i] = 0x00;
  TA6932_WriteAll();
}

// ===== Fixed-address single write (واجهات قديمة) =====
void TA6932_WriteOneRaw(uint8_t addr, uint8_t value){
  // 0x44: fixed-address write. ثم [0xC0|addr] + [data].
  TA_STB(0); uint8_t cmd = 0x44; HAL_SPI_Transmit(&hspi1, &cmd, 1, 10); TA_STB(1);
  TA_STB(0);
  uint8_t a = 0xC0 | (addr & 0x0F);
  HAL_SPI_Transmit(&hspi1, &a, 1, 10);
  HAL_SPI_Transmit(&hspi1, &value, 1, 10);
  TA_STB(1);
  TA6932_putRaw(addr, value); // مزامنة البافر
}
void TA6932_putDigitOne(uint8_t addr, int d, int dp){
  uint8_t v = 0x00;
  if (d >= 0 && d <= 9) v = font7seg['0' + d];
  if (dp) v |= 0x80;
  TA6932_WriteOneRaw(addr, v);
}
void TA6932_putCharOne(uint8_t addr, char ch, int dp){
  uint8_t v = font7seg[(uint8_t)ch];
  if (v == 0x00 && ch != ' ') v = 0x00;
  if (dp) v |= 0x80;
  TA6932_WriteOneRaw(addr, v);
}

// ===== Unified One-API (الجديدة) =====
static uint8_t TA_resolveValue(int value, int dp){
  uint8_t v = 0x00;
  if (value & 0x100){                // RAW via TA_RAW()
    v = (uint8_t)(value & 0xFF);
  } else if (value >= 0 && value <= 9){ // Digit
    v = font7seg['0' + value];
  } else if (value >= 32 && value <= 126){ // Printable ASCII
    v = font7seg[(uint8_t)value];
  } else {
    v = 0x00; // غير معروف → فراغ
  }
  if (dp) v |= 0x80;
  return v;
}
void TA6932_putOne(uint8_t addr, int value, int dp){
  uint8_t v = TA_resolveValue(value, dp);
  TA6932_WriteOneRaw(addr, v);
}
void TA6932_putOneBuf(uint8_t addr, int value, int dp){
  uint8_t v = TA_resolveValue(value, dp);
  TA6932_putRaw(addr, v);
}

// ===== Demos =====
void TA6932_TestPattern(void){
  // HH:MM = 12:34
  TA6932_putDigit(0x00,1,0); TA6932_putDigit(0x01,2,0);
  TA6932_putDigit(0x02,3,0); TA6932_putDigit(0x03,4,0);
  // SS = 56
  TA6932_putDigit(0x04,5,0); TA6932_putDigit(0x05,6,0);
  // YYYY = 2025
  TA6932_putDigit(0x06,2,0); TA6932_putDigit(0x07,0,0);
  TA6932_putDigit(0x08,2,0); TA6932_putDigit(0x09,5,0);
  // MM = 09
  TA6932_putDigit(0x0A,0,0); TA6932_putDigit(0x0B,9,0);
  // DD = 22
  TA6932_putDigit(0x0C,2,0); TA6932_putDigit(0x0D,2,0);
  // D15 colon (dp)
  TA6932_putRaw(0x0E,0x80);
  // D16 weekday off
  TA6932_putRaw(0x0F,0x00);
  TA6932_WriteAll();
}
void TA6932_CounterDemo(void){
  for(int d=0; d<10; d++){
    for(int addr=0; addr<14; addr++){
      TA6932_putDigit(addr, d, 0);
    }
    TA6932_putRaw(0x0E, 0x80); // colon ON
    TA6932_putRaw(0x0F, 0x00); // weekday off
    TA6932_WriteAll();
    HAL_Delay(1000);
  }
}
