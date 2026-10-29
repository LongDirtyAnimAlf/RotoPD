#ifndef _CBUS_H_
#define _CBUS_H_

#include <Arduino.h>

#include "./queue.h"
#include "./can.h"

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

#define DEFAULT_PRIORITY 0xB               // default CBUS messages priority. 1011 = 2|3 = normal/low

#define    CANTXPRIOBUFFERSIZE        50U
#define    CANTXBUFFERSIZE            10u
#define    CANRXBUFFERSIZE            50u

class CANFrame {
public:
  uint32_t id;
  bool ext;
  bool rtr;
  uint8_t len;
  uint8_t data[8] = {};
};

typedef struct _buffer_entry2 {
  unsigned long _item_insert_time;
  CANFrame _item;
} buffer_entry2_t;

class CBUScoe;

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
  //CBUSbase(CBUSConfig *the_config);

  // these methods are pure virtual and must be implemented by the derived class
  // as a consequence, it is not possible to create an instance of this class

#ifdef ARDUINO_ARCH_RP2040
  //virtual bool begin(bool poll = false, SPIClassRP2040 & spi = SPI) = 0;
#else
  //virtual bool begin(bool poll = false, SPIClass & spi = SPI) = 0;
#endif
  //virtual bool begin(bool poll = false) = 0;
  //virtual bool available(void) = 0;
  //virtual CANFrame getNextMessage(void) = 0;
  //virtual bool sendMessage(CANFrame *msg, bool rtr = false, bool ext = false, byte priority = DEFAULT_PRIORITY) = 0;
  //virtual bool sendMessageNoUpdate(CANFrame *msg) = 0;
  //virtual void reset(void) = 0;


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

  bool checkRxQueue(void) {return CanRxBuffer.getCount();}
  bool getTxFull(void) {return ( CanTxBuffer.isFull() || CanTxPrioBuffer.isFull() );}
  bool getRxFull(void) {return CanRxBuffer.isFull();}

  // implementations of these methods are provided in the base class

  byte getCANID(unsigned long header);
  bool isExt(CANFrame *msg);
  bool isRTR(CANFrame *msg);

  unsigned int _numMsgsSent, _numMsgsRcvd;
protected:                                          // protected members become private in derived classes
  CANFrame _msg;
  unsigned char *_mparams;
  unsigned char *_mname;
  CBUScoe *coe_obj = nullptr;

private:
  CircularDataQueue<can_frame, CANRXBUFFERSIZE> CanRxBuffer;
  CircularDataQueue<can_frame, CANTXBUFFERSIZE> CanTxBuffer;
  CircularDataQueue<can_frame, CANTXPRIOBUFFERSIZE> CanTxPrioBuffer;
};

class circular_buffer2 {          // avoid name clash with implementation in CBUSMCP_CAN
public:
  circular_buffer2(byte num_items);
  ~circular_buffer2();
  bool available(void);
  void put(const CANFrame *cf);
  CANFrame *peek(void);
  CANFrame *get(void);
  unsigned long insert_time(void);
  bool full(void);
  void clear(void);
  bool empty(void);
  byte size(void);
  byte free_slots(void);
  unsigned int puts();
  unsigned int gets();
  byte hwm(void);
  unsigned int overflows(void);

private:
  bool _full;
  byte _head, _tail, _capacity, _size, _hwm;
  unsigned int _puts, _gets, _overflows;
  buffer_entry2_t *_buffer;
};

class CBUScoe {

public:
  CBUScoe(const byte num_items = 4);
  ~CBUScoe();
  void put(const CANFrame *msg);
  CANFrame get(void);
  bool available(void);

private:
  circular_buffer2 *coe_buff;
};

#endif // _CBUS_H_