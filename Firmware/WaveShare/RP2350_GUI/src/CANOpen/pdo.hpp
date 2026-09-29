#ifndef PDO_H
#define PDO_H

#include "message.hpp"

class PDO : public Message {
public:
  uint8_t numObjects;       // number of objects mapped to PDO message data field
  uint32_t timer;           // last transmission / reception timestamp
  uint16_t cycleTime;       // Event Timer (ms) – sub-index 05h
  uint8_t  transmissionType;// Transmission type – sub-index 02h (CiA 301)
  uint16_t inhibitTime;     // Inhibit time (units of 100 µs) – sub-index 03h

  enum class Type {
    RX,
    TX
  };
  const Type type;

  PDO(uint16_t id, Type type, uint16_t cycleTime) :
    Message(id, PDO_LEN),
    type(type),
    cycleTime(cycleTime),
    transmissionType(0xFF),   // default: event-driven (CiA 301)
    inhibitTime(0),            // default: disabled
    timer(0),
    numObjects(0)
  {
    for (int i = 0; i < 8; i++) {
      objList[i] = nullptr;
    }
  }

  bool set(Object &obj, uint8_t position);
  bool set(uint16_t index, uint8_t subIndex, uint8_t position);
  void updateData();

  Object *getObject(uint8_t position) const {
    if (position < 8)
      return objList[position];
    return nullptr; // return nullptr if position is out of bounds
  };

  private: 
  Object *objList[8]; // list of objects mapped to PDO message data field
};

#endif