#ifndef __TA6932_H
#define __TA6932_H

// >>> Controller: STM32C011F6P6 (STM32C0 series) <<<
#include "stm32c0xx_hal.h"
#include <stdint.h>

// --- STB pin (عدّلها حسب لوحتك إن لزم) ---
#ifndef TA_STB_PORT
#define TA_STB_PORT  GPIOA
#endif
#ifndef TA_STB_PIN
#define TA_STB_PIN   GPIO_PIN_4
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ===== Core API =====
void TA6932_Init(void);
void TA6932_WriteAll(void);
void TA6932_TestPattern(void);
void TA6932_CounterDemo(void);

// ===== Buffer helpers (back-buffered) =====
void TA6932_putRaw(uint8_t addr, uint8_t v);        // يكتب نمط خام في البافر (بدون إرسال)
void TA6932_putDigit(uint8_t addr, int d, int dp);  // رقم 0..9 إلى البافر
void TA6932_putChar(uint8_t addr, char ch, int dp); // محرف ASCII عبر الفونت
void TA6932_Clear(void);                            // مسح وإرسال

// تعبئة سريعة لكل البافر
void TA6932_loadBuffer(const uint8_t *src);         // نسخ 16 بايت إلى البافر
void TA6932_loadBuffer16(uint8_t b0,uint8_t b1,uint8_t b2,uint8_t b3,
                         uint8_t b4,uint8_t b5,uint8_t b6,uint8_t b7,
                         uint8_t b8,uint8_t b9,uint8_t b10,uint8_t b11,
                         uint8_t b12,uint8_t b13,uint8_t b14,uint8_t b15);

// ===== كتابة خانة واحدة مباشرة (Fixed Address) =====
void TA6932_WriteOneRaw(uint8_t addr, uint8_t value);
void TA6932_putDigitOne(uint8_t addr, int d, int dp);
void TA6932_putCharOne(uint8_t addr, char ch, int dp);

// ===== السطوع / التشغيل والإيقاف =====
void TA6932_SetBrightness(uint8_t level); // 0..7 (وتشغيل العرض)
void TA6932_DisplayOn(void);
void TA6932_DisplayOff(void);

// ===== تحكم بالفونت =====
void TA6932_setGlyph(uint8_t ch, uint8_t pattern);  // ضبط/تغيير نمط محرف (bit7=dp؛ a..g في bit0..6)

#ifdef __cplusplus
}
#endif
#endif
