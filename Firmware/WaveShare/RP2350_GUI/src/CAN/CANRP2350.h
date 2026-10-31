#ifndef _CBUSRP2350_H_
#define _CBUSRP2350_H_

//#ifdef ARDUINO_ARCH_RP2040

#include <atomic>
#include "./CBUS.h"
#include "./../RP2350/CAN/mcp2515.h"
#include "pico/mutex.h"

class CBUSRP2350Device : public CBUSbase {
public:
  CBUSRP2350Device(void);

  void begin(void);
  void begin(uint8_t MasterID, uint8_t IT2704I);

  // Virtual overrides

  CANERROR abortCANAllPending(bool withqueues = false);
  bool checkCANInterrupt(void);
  CANERROR resetCAN(void);
  bool taskCAN(bool single = false);
  CANERROR sendCANMessage(const can_frame *frame);
  CANERROR sendCANMessage(const can_frame &frame);
  CANERROR sendCANMessagePrio(const can_frame *frame);
  CANERROR sendCANMessagePrio(const can_frame &frame);
  CANERROR readCANMessage(can_frame *frame);
  CANERROR readCANMessage(can_frame &frame);

  const MCP2515& getMCP2515() const { return caninterface; }
  MCP2515& getMCP2515() { return caninterface; }  

private:
  MCP2515 caninterface;  
  const uint8_t _intPin;
  mutex_t can_mutex;

  //volatile bool HandleTxMessages;
  std::atomic<bool> HandleTxMessages;

  bool processTxQueue(void);
};

//extern CBUSRP2350Device CBUSDevice;

//#endif

#endif // _CBUSRP2350_H_