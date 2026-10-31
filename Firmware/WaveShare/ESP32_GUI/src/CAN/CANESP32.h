#ifndef _CBUSESP32_H_
#define _CBUSESP32_H_

//#if defined(ARDUINO_ARCH_ESP32)

#include <atomic>
#include "./CBUS.h"
#include "./../ESP32/CAN/ESP32-TWAI-CAN.hpp"
#include "./../ESP32/CAN/twai.h"

class CBUSDevice : public CBUSbase {
public:
  CBUSDevice(void);

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
private:
  TwaiCAN caninterface;  
  mutex_t can_mutex;
  //volatile bool HandleTxMessages;
  std::atomic<bool> HandleTxMessages;
  bool processTxQueue(void);

  bool readTWAIMessage(struct can_frame *frame);
  bool sendTWAIMessage(const struct can_frame *frame);
};

//#endif

#endif // _CBUSESP32_H_