#ifndef _COMMS_H_
#define _COMMS_H_

#ifdef ARDUINO_WIO_TERMINAL
#define USE_LCD 
#define BUZZER_PIN WIO_BUZZER /* sig pin of the buzzer */
//#define PIN_WIRE_SERCOM SERCOM3
//#define PIN_WIRE_BATT_SDA  PIN_WIRE_SDA
//#define PIN_WIRE_BATT_SCL  PIN_WIRE_SCL
#define PIN_WIRE_SERCOM SERCOM4
#define PIN_WIRE_BATT_SDA  PIN_WIRE1_SDA
#define PIN_WIRE_BATT_SCL  PIN_WIRE1_SCL
#endif

#ifdef SEEED_XIAO_M0
#define PIN_WIRE_SERCOM SERCOM2
#define PIN_WIRE_BATT_SDA PIN_WIRE_SDA
#define PIN_WIRE_BATT_SCL PIN_WIRE_SCL
#endif

#ifdef ARDUINO_SEEED_INDICATOR_RP2040
#define PIN_WIRE_BATT_SDA  PIN_WIRE0_SDA
#define PIN_WIRE_BATT_SCL  PIN_WIRE0_SCL
#include "Adafruit_TinyUSB.h"
#include "storage.h"
#endif

#ifdef ARDUINO_ESP32S3_DEV
#define USE_LCD 
#include "USB.h"
#include "USBHID.h"
#include "storage.h"
#endif

#include <Arduino.h>
#include "shared.h"
#include "extras.h"

#ifdef ARDUINO_SEEED_INDICATOR_RP2040
#include <Wire.h>
#endif

const byte DefaultBoardSerial[12] = {0xFF,0x1F,0xFF,0x2F,0xFF,0x3F,0xFF,0x4F,0xFF,0x5F,0xFF,0x6F};
const byte DefaultCalDate[4] = {20,26,01,01};
#define DEFAULTBOARDNUMBER 1

#if defined(ARDUINO_ARCH_SAMD)
#define WireBattery MyWire
#endif

#ifdef ARDUINO_SEEED_INDICATOR_RP2040
#define WireBattery Wire
#endif

#ifdef ARDUINO_ESP32S3_DEV
#define WireBattery Wire
#endif

#ifdef ARDUINO_ARCH_RP2040
#define WireBattery Wire
#endif


extern TBoardInfo BoardInfo;
extern volatile THIDData HIDData[];//[DAUGHTERBOARDCOUNT];

void Info_Add(const char *txt);
void Info_Add_Fmt(const char *format, ...);

bool process_command(void const *data, void *result);

bool initINA238(void);

bool initROTOPD(void);
int8_t taskRotoPDInit(void);
void collectRotoPDData(void);
void getRotoPDData(word* I,word* V,dword* P,word* T);

#endif
