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

// USB HID report descriptor.
uint8_t const desc_hid_report[] = 
{
  TUD_HID_REPORT_DESC_GENERIC_INOUT(HID_INT_OUT_EP_SIZE)
};
const byte DefaultBoardSerial[12] = {0xFF,0x1F,0xFF,0x2F,0xFF,0x3F,0xFF,0x4F,0xFF,0x5F,0xFF,0x6F};

#if defined(ARDUINO_ARCH_SAMD)
#define WireBattery MyWire
#endif

#ifdef ARDUINO_SEEED_INDICATOR_RP2040
#define WireBattery Wire
#endif

#ifdef ARDUINO_ESP32S3_DEV
#define WireBattery Wire
#endif

extern TwoWire WireBattery;

extern TBatteryBoard BatteryBoards[];//[DAUGHTERBOARDCOUNT];
extern TBoardInfo BoardInfo;
extern volatile THIDData HIDData[];//[DAUGHTERBOARDCOUNT];

#ifdef ARDUINO_ESP32S3_DEV
extern USBHID HID;
extern USBCDC USBSerial;
#endif

#ifdef ARDUINO_SEEED_INDICATOR_RP2040
extern Adafruit_USBD_HID HID;
#endif

//extern AP33772S usbpd;
extern AP33772S pd;
extern INA238 ina238;

bool process_command(void const *data, void *result);

bool initINA238(void);

bool InitROTOPD(void);
void taskRotoPDInit(void);
void collectRotoPDData(void);
void getRotoPDData(word* I,word* V,dword* P,word* T);

#ifdef ARDUINO_ESP32S3_DEV
void set_report_callback(uint8_t report_id, uint8_t const* hid_report_out, uint16_t bufsize);
#else
void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* hid_report_out, uint16_t bufsize);
#endif

#endif
