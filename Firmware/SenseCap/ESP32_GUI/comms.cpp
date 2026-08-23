#include "comms.h"

#ifdef ARDUINO_SEEED_INDICATOR_RP2040
#define USBSerial Serial
#include <PacketSerial.h>
extern COBSPacketSerial myPacketSerial; // for logging
#endif
#if defined(ARDUINO_ARCH_SAMD)  
#define USBSerial Serial
#endif
#ifdef ARDUINO_ESP32S3_DEV
#include "./src/UI/screenlogger.h"
extern USBCDC USBSerial;
#endif

void Info_Add(const char *txt)
{
  if (txt == NULL) return;

  #ifdef DEBUG
  USBSerial.println(txt);
  #endif

  #ifdef ARDUINO_ESP32S3_DEV
  ScreenLogger_Add(txt,true);
  #endif
}

void Info_Add_Fmt(const char *format, ...)
{
  char myString[128];
  va_list args;
  va_start(args, format);
  int result = vsnprintf(myString, sizeof(myString), format, args);
  va_end(args);                    // Clean up
  if (result < 0) return;   // encoding error
  // Optional: detect truncation
  if (result >= (int)sizeof(myString)) {
    // message was truncated
  }  
  Info_Add(myString);
}
