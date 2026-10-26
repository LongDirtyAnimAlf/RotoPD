#include "message.hpp"
#include "sdo.hpp"

// Clear all message fields
void Message::clearMsg() {
  id = 0; 
  dlc = 0;
  clearData();
}

// Clear message data
void Message::clearData() {
  memset(data, 0, sizeof(data));
}

void Message::setCommand(uint8_t cmd) {
  data[SDO::COMMAND] = cmd;
}
void Message::setID(uint16_t ID) {
  data[SDO::INDEX_LOW]   = (ID & 0xFF);
  data[SDO::INDEX_HIGH]  = ((ID >> 8) & 0xFF);
}
void Message::setSubID(uint8_t sID) {
  data[SDO::SUB_INDEX]   = sID;
}

void Message::setData(uint32_t payload)
{
  data[SDO::DATA_4] = ((payload >> 0) & 0xFF);
  data[SDO::DATA_5] = ((payload >> 8) & 0xFF);
  data[SDO::DATA_6] = ((payload >> 16) & 0xFF);
  data[SDO::DATA_7] = ((payload >> 24) & 0xFF);
}

uint8_t Message::getCommand() const
{
  return (data[SDO::COMMAND]);
}
uint16_t Message::getID() const
{
  return (data[SDO::INDEX_LOW] + (data[SDO::INDEX_HIGH] * 256));
}
uint8_t Message::getSubID() const
{
  return (data[SDO::SUB_INDEX]);
}
uint32_t Message::getData() const
{
  return (uint32_t)(data[SDO::DATA_4]+(data[SDO::DATA_5] << 8)+(data[SDO::DATA_6] << 16)+(data[SDO::DATA_7] << 24));
}
