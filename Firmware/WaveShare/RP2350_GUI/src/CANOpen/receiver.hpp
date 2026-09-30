#ifndef RECEIVER_H
#define RECEIVER_H

#include "./message.hpp"

// Receiver message type
struct Receiver : public Message {
  MCP2515 *CAN;
  bool active;
  // specify CAN bus for receiver message
  Receiver(MCP2515 *CAN) : 
    CAN(CAN), 
    active(false)
  {};

  bool read(void); // read message from CAN bus
  void printBuf(); // print message to serial monitor
};

#endif