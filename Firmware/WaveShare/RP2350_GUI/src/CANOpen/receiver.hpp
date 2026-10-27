#ifndef RECEIVER_H
#define RECEIVER_H

#include "message.hpp"

#ifdef ARDUINO_ARCH_RP2040
#include "./../RP2350/CAN/mcp2515.h"
#endif

#ifdef ARDUINO_ARCH_ESP32
#include "./../ESP32/CAN/ESP32-TWAI-CAN.hpp"
typedef TwaiCAN MCP2515;
#endif

// Receiver message type
struct Receiver : public Message {
  MCP2515 *CAN;
  // specify CAN bus for receiver message
  Receiver(MCP2515 *CAN) : 
    CAN(CAN)
  {};

  bool run(void);  // handle canbus activity
  void printBuf(); // print message to serial monitor
};

#endif