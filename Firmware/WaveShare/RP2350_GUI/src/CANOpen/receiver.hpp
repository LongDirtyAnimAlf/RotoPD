#ifndef RECEIVER_H
#define RECEIVER_H

#include "message.hpp"

#ifdef ARDUINO_ARCH_RP2040
#include "./../CAN/CANRP2350.h"
#endif

// Receiver message type
struct Receiver : public Message {
  CBUSbase *CAN;
  // specify CAN bus for receiver message
  Receiver(CBUSbase *CAN) : 
    CAN(CAN)
  {};

  bool run(void);  // handle canbus activity
  void printBuf(); // print message to serial monitor
};

#endif