#ifndef _CBUSRP2350_H_
#define _CBUSRP2350_H_

//#ifdef ARDUINO_ARCH_RP2040

#include "./CBUS.h"

#include "./../RP2350/CAN/mcp2515.h"

class CBUSRP2350Device : public CBUSbase {
public:
  CBUSRP2350Device(void);

  void begin(uint8_t MasterID = -1, uint8_t IT2704I = -1);
  bool checkInterrupt(void);
  ERROR abortAllPending(bool withqueues = false);

  ERROR reset(void);
  bool run(bool single = false);
  ERROR sendMessage(const can_frame *frame);
  ERROR sendMessage(const can_frame &frame);
  ERROR sendMessagePrio(const can_frame *frame);
  ERROR sendMessagePrio(const can_frame &frame);
  ERROR readMessage(can_frame *frame);
  ERROR readMessage(can_frame &frame);
  uint32_t getRxQueueDepth(void);

  const MCP2515& getCAN() const { return can; }
  MCP2515& getCAN() { return can; }  

private:
  MCP2515 can;  
  const uint8_t intPin;

};

extern CBUSRP2350Device CBUSDevice;

//#endif

#endif // _CBUSRP2350_H_