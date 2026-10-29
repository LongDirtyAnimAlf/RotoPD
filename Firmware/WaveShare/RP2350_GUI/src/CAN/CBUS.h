#ifndef _CBUS_H_
#define _CBUS_H_

#include <Arduino.h>

#include "./queue.h"
#include "./can.h"

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

#define    CANTXBUFFERSIZE            10u
#define    CANRXBUFFERSIZE            50u
#define    CANTXPRIOBUFFERSIZE        10u

class CBUSbase {
public:
  enum ERROR {
      ERROR_OK        = 0,
      ERROR_FAIL      = 1,
      ERROR_ALLTXBUSY = 2,
      ERROR_FAILINIT  = 3,
      ERROR_FAILTX    = 4,
      ERROR_NOMSG     = 5
  };

  CBUSbase();

  // General methods to be implemented

  virtual ERROR abortAllPending(bool withqueues = false) = 0;
  virtual bool checkInterrupt(void) = 0;

  virtual ERROR reset(void) = 0;
  virtual bool run(bool single = false) = 0;
  
  virtual ERROR sendMessage(const can_frame *frame) = 0;
  virtual ERROR sendMessage(const can_frame &frame) = 0;
  virtual ERROR sendMessagePrio(const can_frame *frame) = 0;
  virtual ERROR sendMessagePrio(const can_frame &frame) = 0;
  virtual ERROR readMessage(can_frame *frame) = 0;
  virtual ERROR readMessage(can_frame &frame) = 0;
  virtual uint32_t getRxQueueDepth(void) = 0;

  bool checkRxQueue(void) {return (CanRxBuffer.getCount() != 0);}
  bool getTxFull(void) {return ( CanTxBuffer.isFull() || CanTxPrioBuffer.isFull() );}
  bool getRxFull(void) {return CanRxBuffer.isFull();}
protected:                                          // protected members become private in derived classes
//private:
  CircularDataQueue<can_frame, CANRXBUFFERSIZE> CanRxBuffer;
  CircularDataQueue<can_frame, CANTXBUFFERSIZE> CanTxBuffer;
  CircularDataQueue<can_frame, CANTXPRIOBUFFERSIZE> CanTxPrioBuffer;
};

#endif // _CBUS_H_