#ifdef ARDUINO_ARCH_RP2040
#include "./src/RP2350/lvgl/lv_port_disp.h"
#endif

#ifdef ARDUINO_ARCH_ESP32
#include "./src/ESP32/lvgl/lvgldriver.h"
#endif

#include "touch.h"

#include "./src/UI/screenbase.h"
#include "./src/UI/matrix_screen.h"
#include "./src/UI/screensaver.h"
#include "./src/Fonts/fonts.h"
