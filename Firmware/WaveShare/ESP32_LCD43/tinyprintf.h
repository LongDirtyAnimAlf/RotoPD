#ifndef _TINYPRINTF_H_
#define _TINYPRINTF_H_

//#pragma once          // or classic include guards

#include <Arduino.h>

#include "./src/printf/printf.h"

#ifdef __cplusplus
extern "C" __attribute__((used)) inline void putchar_(char character) {
  (void)character;
}
#else
__attribute__((used)) static inline void putchar_(char character) {
  (void)character;
}
#endif

#endif /*_TINYPRINTF_H_*/