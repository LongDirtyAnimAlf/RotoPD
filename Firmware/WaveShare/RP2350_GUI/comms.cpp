#include "comms.h"

#include "./src/UI/screenlogger.h"

void Info_Add(const char *txt)
{
  if (txt == NULL) return;

  ScreenLogger_Add(txt,true);
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
