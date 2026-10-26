#ifdef ARDUINO_ESP32S3_DEV
#ifdef SENSECAP
#include "./src/ESP32/Storage/indicator_storage_nvs.h"
#else
#include "./src/ESP32/CAN/twai.h"
#include "./src/ESP32/CAN/ESP32-TWAI-CAN.hpp"
#include "./src/ESP32/CH32/WS_CH32_IO.h"
#endif
#endif
