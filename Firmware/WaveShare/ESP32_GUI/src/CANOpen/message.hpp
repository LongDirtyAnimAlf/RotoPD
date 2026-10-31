#ifndef MESSAGE_H
#define MESSAGE_H

#include <cstring> // Required for memset
#include "object.hpp"

// Generic message type
class Message {
  public:
  bool prio;  
  uint32_t id;  
  uint8_t  dlc; // data length code
  uint8_t data[8];
  
  Message() : 
    prio(false),
    id(0), 
    dlc(0) 
  {
    memset(data, 0, sizeof(data));
  }; 
  
  Message(uint32_t id, uint8_t dlc) : 
    id(id), 
    dlc(dlc) 
  {
    //memset(data, 0, sizeof(data));
    std::memset(data, 0, sizeof(data));
  };

  void clearMsg();
  void clearData();
  void setCommand(uint8_t cmd);
  void setID(uint16_t ID);
  void setSubID(uint8_t sID);
  void setData(uint32_t payload);
  uint8_t getCommand() const;
  uint16_t getID() const;
  uint8_t getSubID() const;
  uint32_t getData() const;

  //virtual ~Message() {};
  // Standard, non-virtual destructor lets the compiler treat this class as plain data
  ~Message() = default;    
};

#endif
