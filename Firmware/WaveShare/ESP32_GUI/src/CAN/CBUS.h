#ifndef _CANBUS_H_
#define _CANBUS_H_

#include <Arduino.h>

#include "./can.h"

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

class CBUSbase {
public:
  enum CANERROR {
      ERROR_OK        = 0,
      ERROR_FAIL      = 1,
      ERROR_ALLTXBUSY = 2,
      ERROR_FAILINIT  = 3,
      ERROR_FAILTX    = 4,
      ERROR_NOMSG     = 5
  };

  CBUSbase();

  // General methods to be implemented

  virtual CANERROR abortCANAllPending(bool withqueues = false) = 0;
  virtual bool checkCANInterrupt(void) = 0;

  virtual CANERROR resetCAN(void) = 0;
  virtual bool taskCAN(bool single = false) = 0;
  
  virtual CANERROR sendCANMessage(const can_frame *frame) = 0;
  virtual CANERROR sendCANMessage(const can_frame &frame) = 0;
  virtual CANERROR sendCANMessagePrio(const can_frame *frame) = 0;
  virtual CANERROR sendCANMessagePrio(const can_frame &frame) = 0;
  virtual CANERROR readCANMessage(can_frame *frame) = 0;
  virtual CANERROR readCANMessage(can_frame &frame) = 0;
  
  virtual uint32_t getCANRxQueueDepth(void) = 0;
  virtual uint32_t getCANTxQueueDepth(void) = 0;
  virtual uint32_t getCANTxPrioQueueDepth(void) = 0;
  virtual bool checkCANRxQueue(void) = 0;
  virtual bool getCANTxFull(void) = 0;
  virtual bool getCANRxFull(void) = 0;
};

#endif // _CANBUS_H_