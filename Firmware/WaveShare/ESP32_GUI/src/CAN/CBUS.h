#ifndef _CANBUS_H_
#define _CANBUS_H_

#include <Arduino.h>

#include "./queue.h"
#include "./can.h"

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

#define    CANTXBUFFERSIZE            50u
#define    CANRXBUFFERSIZE            75u
#define    CANTXPRIOBUFFERSIZE        50u

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
  
  uint32_t getCANRxQueueDepth(void) {return CanRxBuffer.getCount();}
  uint32_t getCANTxQueueDepth(void) {return CanTxBuffer.getCount();}
  uint32_t getCANTxPrioQueueDepth(void) {return CanTxPrioBuffer.getCount();}

  bool checkCANRxQueue(void) {return ( !CanRxBuffer.isEmpty() );}
  bool getCANTxFull(void) {return ( CanTxBuffer.isFull() || CanTxPrioBuffer.isFull() );}
  bool getCANRxFull(void) {return ( CanRxBuffer.isFull() );}
protected:                                          // protected members become private in derived classes
//private:
  CircularDataQueue<can_frame, CANRXBUFFERSIZE> CanRxBuffer;
  CircularDataQueue<can_frame, CANTXBUFFERSIZE> CanTxBuffer;
  CircularDataQueue<can_frame, CANTXPRIOBUFFERSIZE> CanTxPrioBuffer;
};

#endif // _CANBUS_H_