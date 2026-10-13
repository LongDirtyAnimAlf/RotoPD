#include "IT2704.h"

typedef union
{
    uint32_t Val;
    uint8_t v[4];// __PACKED;
    uint16_t w[2];// __PACKED;
    struct //__PACKED
    {
        uint8_t LB;
        uint8_t HB;
        uint8_t UB;
        uint8_t MB;
    } bytes;
    float f;
} DWORD_FLOAT;

// External dictionary (definition in .cpp)
extern Object dictionaryIT2704[];
extern const uint16_t NUM_OBJS_IT2704;

// ── Constructor ──────────────────────────────────────────────────────────────
IT2704::IT2704(CanOpenNode *node, uint8_t id, uint8_t channel)
    : _node(node),
      _id(id),
      _channel(channel)
{
}

// ── begin() ──────────────────────────────────────────────────────────────────
int8_t IT2704::begin(void)
{
    Object* target = findObject(IT2704_SOURCE_CURRENT, IT2704_SOURCE_CURRENT_CURRENT_LEVEL);

    return 0;
}

void IT2704::setID(uint8_t id)
{
    _id = id;
}

uint8_t IT2704::getID(void)
{
    return _id;
}

void IT2704::setChannel(uint8_t channel)
{
    _channel = channel;
}

uint8_t IT2704::getChannel(void)
{
    return _channel;
}

void IT2704::initDevice(void)
{
  Message Msg;
  DWORD_FLOAT data;  

  Msg.clearMsg();

  Msg.id        = (CO_CAN_ID_SDO_CLI + _id);
  Msg.dlc       = 8;

  // Set output slewrate
  // On: slow ; off: max (fast)

  Msg.data[SDO::COMMAND]      = WRITE_REQ_1BYTE_CMD;
  Msg.data[SDO::INDEX_LOW]    = (uint8_t)((IT2704_SOURCE_CURRENT + (_channel * 0x100)) & 0xFF);
  Msg.data[SDO::INDEX_HIGH]   = (uint8_t)((IT2704_SOURCE_CURRENT + (_channel * 0x100)) >> 8);

  // Write on slew max off
  Msg.data[SDO::SUB_INDEX]    = IT2704_SOURCE_CURRENT_CURRENT_SLEW_POSITIVE_MAX;
  Msg.data[SDO::DATA_4]       = 0; // 0:Off 1:On
  _node->sendMsg(Msg);

  // Write off slew max on
  Msg.data[SDO::SUB_INDEX]    = IT2704_SOURCE_CURRENT_CURRENT_SLEW_NEGATIVE_MAX;
  Msg.data[SDO::DATA_4]       = 1; // 0:Off 1:On
  _node->sendMsg(Msg);

  // Write on slew rate
  Msg.data[SDO::COMMAND]      = WRITE_REQ_4BYTE_CMD;
  Msg.data[SDO::INDEX_LOW]    = (uint8_t)((IT2704_SOURCE_CURRENT + (_channel * 0x100)) & 0xFF);
  Msg.data[SDO::INDEX_HIGH]   = (uint8_t)((IT2704_SOURCE_CURRENT + (_channel * 0x100)) >> 8);

  data.f = 0.01; // Amps/ms
  Msg.data[SDO::SUB_INDEX]    = IT2704_SOURCE_CURRENT_CURRENT_SLEW_POSITIVE;
  Msg.data[SDO::DATA_4]       = data.v[0];
  Msg.data[SDO::DATA_5]       = data.v[1];
  Msg.data[SDO::DATA_6]       = data.v[2];
  Msg.data[SDO::DATA_7]       = data.v[3];
  _node->sendMsg(Msg);


  // COMMON_SOURCE_SLOPE_TYPE // 0:Rate [A/ms] 1:Time

  //Current slew time positive
  //Current slew time negative
}

void IT2704::setCurrent(uint16_t current) // in mA
{
  Message Msg;
  DWORD_FLOAT data;

  Msg.clearMsg();

  Msg.id        = (CO_CAN_ID_SDO_CLI + _id);
  Msg.dlc       = 8;

  // Set current

  data.f = current; // current is set in mA !!
  data.f /= 1000.0;

  Msg.data[SDO::COMMAND]      = WRITE_REQ_4BYTE_CMD;
  Msg.data[SDO::INDEX_LOW]    = (uint8_t)((IT2704_SOURCE_CURRENT + (_channel * 0x100)) & 0xFF);
  Msg.data[SDO::INDEX_HIGH]   = (uint8_t)((IT2704_SOURCE_CURRENT + (_channel * 0x100)) >> 8);
  Msg.data[SDO::SUB_INDEX]    = IT2704_SOURCE_CURRENT_CURRENT_LEVEL;
  Msg.data[SDO::DATA_4]       = data.v[0];
  Msg.data[SDO::DATA_5]       = data.v[1];
  Msg.data[SDO::DATA_6]       = data.v[2];
  Msg.data[SDO::DATA_7]       = data.v[3];

  _node->sendMsg(Msg);

  //#define IT2704_SOURCE_VOLTAGE_INHIBIT_MODE                      SUBIDX_20  0:Off 1:live 2:latching
  //#define IT2704_SOURCE_VOLTAGE_INHIBIT_VON                       SUBIDX_21
  //#define IT2704_SOURCE_VOLTAGE_INHIBIT_VOFF                      SUBIDX_22

}

void IT2704::setOutput(bool enabled)
{
  Message Msg;

  Msg.clearMsg();

  Msg.id        = (CO_CAN_ID_SDO_CLI + _id);
  Msg.dlc       = 8;

  // Set output

  Msg.data[SDO::COMMAND]      = WRITE_REQ_1BYTE_CMD;
  Msg.data[SDO::INDEX_LOW]    = (uint8_t)((IT2704_OUTPUT + (_channel * 0x100)) & 0xFF);
  Msg.data[SDO::INDEX_HIGH]   = (uint8_t)((IT2704_OUTPUT + (_channel * 0x100)) >> 8);
  Msg.data[SDO::SUB_INDEX]    = IT2704_OUTPUT_ONOFF_STATE;
  Msg.data[SDO::DATA_4]       = (enabled ? 1 : 0);

  _node->sendMsg(Msg);

  // COMMON_SOURCE_SLOPE_TYPE // 0:Rate [A/ms] 1:Time

//Current slew positive
//Current slew negative
//Current slew time positive
//Current slew time negative
//Current slew couple
//Current slew  positive max // 0:Off 1:On
//Current slew negative max // 0:Off 1:On



}

void IT2704::collectData(void)
{
  Message Msg;

  Msg.clearMsg();

  Msg.id        = (CO_CAN_ID_SDO_CLI + _id);
  Msg.dlc       = 8;

  Msg.data[SDO::COMMAND]      = READ_REQ_ANY_CMD;
  Msg.data[SDO::INDEX_LOW]    = (uint8_t)((IT2704_METER + (_channel * 0x100)) & 0xFF);
  Msg.data[SDO::INDEX_HIGH]   = (uint8_t)((IT2704_METER + (_channel * 0x100)) >> 8);
  
  Msg.data[SDO::SUB_INDEX]    = IT2704_METER_VOLTAGE;
  _node->sendMsg(Msg);
  // This message will be confirmed
  // Voltage data will be inside the response !

  Msg.data[SDO::SUB_INDEX]    = IT2704_METER_CURRENT;
  _node->sendMsg(Msg);
  // This message will be confirmed
  // Current data will be inside the response !

  Msg.data[SDO::SUB_INDEX]    = IT2704_METER_POWER;
  _node->sendMsg(Msg);

  //Msg.data[SDO::SUB_INDEX]    = IT2704_METER_AHOUR;
  //node.sendMsg(Msg);

  Msg.data[SDO::SUB_INDEX]    = IT2704_METER_WHOUR;
  _node->sendMsg(Msg);
}

// Return object index position in dictionary array
Object* IT2704::findObject(uint16_t index, uint8_t subIndex) {
  // loop through object dictionary to find matching indices
  for (int i = 0; i < NUM_OBJS_IT2704; ++i)
  {
    if (index == dictionaryIT2704[i].index && subIndex == dictionaryIT2704[i].subIndex)
    {
      return (&dictionaryIT2704[i]);
    }
  }
  return NULL; // return NULL if no match found
}

