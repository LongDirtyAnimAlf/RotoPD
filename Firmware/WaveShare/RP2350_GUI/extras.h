#ifdef ARDUINO_ESP32S3_DEV
#ifdef SENSECAP
#include "./src/Storage/indicator_storage_nvs.h"
#else
#include "./src/CAN/twai.h"
#include "./src/CAN/ESP32-TWAI-CAN.hpp"
#include "./src/CH32/WS_CH32_IO.h"
#endif
#endif
