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
    _connected = false;
    _checkTimer = millis();
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

void IT2704::initDevice(bool reset)
{
  Message Msg;
  DWORD_FLOAT data;  

  if (_connected)
  {
    if (reset)
    {
      _node->getCAN().abortAllPending(); // Discard all messages in hardware and queue

      // IT2704 enable CANopen !
      Msg.clearMsg();
      Msg.id        = 0;
      Msg.dlc       = 2;

      // IT2704 enable CANopen !
      // This is done by sending enter operational command
      // This is also the only possible way to start CANopen unfortunately
      Msg.data[0]   = CO_NMT_ENTER_OPERATIONAL;
      Msg.data[1]   = _id;
      _node->sendMsg(Msg);

      // Try to set pre-operational mode
      // The IT2704 works in this mode, but does not send TPDOs
      // So, less traffic !
      Msg.clearMsg();
      Msg.data[0]   = CO_NMT_ENTER_PRE_OPERATIONAL;
      Msg.data[1]   = _id;
      _node->sendMsg(Msg);

      Msg.clearMsg();

      Msg.id        = (CO_CAN_ID_SDO_CLI + _id);
      Msg.dlc       = 8;

      // Silence the 4 TPDOs
      // Should not be necessary if IT2704 is in pre-operational mode
      // But anyhow
      for (int j = 0; j < 4; j++)
      {
        // Set Set the transmission type of TPDO#j to 254
        Msg.clearData();
        Msg.setCommand(WRITE_REQ_1BYTE_CMD);
        Msg.setID(TX_PDO1_COMM_INDEX + j);
        Msg.setSubID(SUBIDX_PDO_TRANSMISSION_TYPE);
        Msg.setData(PDO_TXTYPE_ASYNC_MANUFACTURER);
        _node->sendMsg(Msg);
        // This message will be confirmed
        // We might have a look at it

        //Modify TPDO#j timer cycle as 0ms
        Msg.clearData();
        Msg.setCommand(WRITE_REQ_2BYTE_CMD);
        Msg.setID(TX_PDO1_COMM_INDEX + j);
        Msg.setSubID(SUBIDX_PDO_EVENT_TIMER);
        Msg.setData(0);
        _node->sendMsg(Msg);
        // This message will be confirmed
        // We might have a look at it
      }
    }

    Msg.clearMsg();

    Msg.id        = (CO_CAN_ID_SDO_CLI + _id);
    Msg.dlc       = 8;

    // Set output slewrate
    // On: slow ; off: max (fast)

    Msg.setCommand(WRITE_REQ_1BYTE_CMD);
    Msg.setID(IT2704_SOURCE_CURRENT + (_channel * 0x100));

    // Write slew couple/tracking off
    // This will allow off and on with different settings
    Msg.setSubID(IT2704_SOURCE_CURRENT_CURRENT_SLEW_COUPLE);
    Msg.setData(0); // 0:Off 1:On
    _node->sendMsg(Msg);

    // Write on slew max off
    Msg.setSubID(IT2704_SOURCE_CURRENT_CURRENT_SLEW_POSITIVE_MAX);
    Msg.setData(0); // 0:Off 1:On
    _node->sendMsg(Msg);

    // Write off slew max on
    Msg.setSubID(IT2704_SOURCE_CURRENT_CURRENT_SLEW_NEGATIVE_MAX);
    Msg.setData(1); // 0:Off 1:On
    _node->sendMsg(Msg);

    // Write on slew rate value
    Msg.setCommand(WRITE_REQ_4BYTE_CMD);
    Msg.setID(IT2704_SOURCE_CURRENT + (_channel * 0x100));
    Msg.setSubID(IT2704_SOURCE_CURRENT_CURRENT_SLEW_POSITIVE);

    data.f = 0.01f; // Amps/ms
    Msg.setData(data.Val);
    _node->sendMsg(Msg);

    // Inhibit settings
    Msg.clearData();
    Msg.setID(IT2704_SOURCE_VOLTAGE + (_channel * 0x100));

    Msg.setCommand(WRITE_REQ_1BYTE_CMD);
    Msg.setSubID(IT2704_SOURCE_VOLTAGE_INHIBIT_MODE);
    Msg.setData(1); // 0:Off 1:live 2:latching

    Msg.setCommand(WRITE_REQ_4BYTE_CMD);
  
    // Switch load on when voltage is above this level
    Msg.setSubID(IT2704_SOURCE_VOLTAGE_INHIBIT_VON);
    data.f = 4.0f; // Volt
    Msg.setData(data.Val);
    _node->sendMsg(Msg);

    // Only needed when live mode enabled
    // Switch load off when voltage is below this level
    Msg.setSubID(IT2704_SOURCE_VOLTAGE_INHIBIT_VOFF);
    data.f = 3.0f; // Volt
    Msg.setData(data.Val);
    _node->sendMsg(Msg);


    // COMMON_SOURCE_SLOPE_TYPE // 0:Rate [A/ms] 1:Time

    //Current slew time positive
    //Current slew time negative
  }
}

void IT2704::setCurrent(uint16_t current) // in mA
{
  Message Msg;
  DWORD_FLOAT data;

  if (_connected)
  {
    Msg.clearMsg();

    Msg.id        = (CO_CAN_ID_SDO_CLI + _id);
    Msg.dlc       = 8;

    // Set current

    Msg.setCommand(WRITE_REQ_4BYTE_CMD);
    Msg.setID(IT2704_SOURCE_CURRENT + (_channel * 0x100));
    Msg.setSubID(IT2704_SOURCE_CURRENT_CURRENT_LEVEL);

    data.f = current; // current is given in mA !!
    data.f /= 1000.0f; // current is set in float A !!
    Msg.setData(data.Val);

    //Msg.data[SDO::DATA_4]       = data.v[0];
    //Msg.data[SDO::DATA_5]       = data.v[1];
    //Msg.data[SDO::DATA_6]       = data.v[2];
    //Msg.data[SDO::DATA_7]       = data.v[3];

    _node->sendMsg(Msg);

    //#define IT2704_SOURCE_VOLTAGE_INHIBIT_MODE                      SUBIDX_20  0:Off 1:live 2:latching
    //#define IT2704_SOURCE_VOLTAGE_INHIBIT_VON                       SUBIDX_21
    //#define IT2704_SOURCE_VOLTAGE_INHIBIT_VOFF                      SUBIDX_22
  }
}

void IT2704::setOutput(bool enabled)
{
  Message Msg;

  if (_connected)
  {
    Msg.clearMsg();

    Msg.id        = (CO_CAN_ID_SDO_CLI + _id);
    Msg.dlc       = 8;

    // Set output
    Msg.setCommand(WRITE_REQ_1BYTE_CMD);
    Msg.setID(IT2704_OUTPUT + (_channel * 0x100));
    Msg.setSubID(IT2704_OUTPUT_ONOFF_STATE);
    Msg.setData(enabled ? 1 : 0);

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
}

void IT2704::collectData(void)
{
  Message Msg;

  if (_connected)
  {
    Msg.clearMsg();

    Msg.id        = (CO_CAN_ID_SDO_CLI + _id);
    Msg.dlc       = 8;

    Msg.setCommand(READ_REQ_ANY_CMD);
    Msg.setID(IT2704_METER + (_channel * 0x100));

    Msg.setSubID(IT2704_METER_VOLTAGE);
    _node->sendMsg(Msg);
    // This message will be confirmed
    // Voltage data will be inside the response !

    Msg.setSubID(IT2704_METER_CURRENT);
    _node->sendMsg(Msg);
    // This message will be confirmed
    // Current data will be inside the response !

    Msg.setSubID(IT2704_METER_POWER);
    _node->sendMsg(Msg);

    //Msg.setSubID(IT2704_METER_AHOUR);
    //node.sendMsg(Msg);

    Msg.setSubID(IT2704_METER_WHOUR);
    _node->sendMsg(Msg);
  }
}

void IT2704::getData(const Message* Msg, uint16_t* I, uint16_t* V, uint32_t* P, uint32_t* E)
{
    DWORD_FLOAT data; 

    data.Val = Msg->getData(); 

    if (Msg->getSubID() == IT2704_METER_VOLTAGE)
    {
        *V = round(data.f * 1000);
    }
    if (Msg->getSubID() == IT2704_METER_CURRENT)
    {
        *I = round(data.f * 1000);
    }
    if (Msg->getSubID() == IT2704_METER_POWER)
    {
        *P = round(data.f * 1000);
    }
    if (Msg->getSubID() == IT2704_METER_WHOUR)
    {
        *E = round(data.f * 1000);
    }
}

void IT2704::queryConnection(void)
{
  Message Msg;

  if (_connected)
  {
    // Check connection every 5 seconds
    // Timer will be reset after reception of good IT2704 message
    // So, this will only happen in case connection is lost
    if (millis() - _checkTimer > 5000) 
    {
      _checkTimer = millis();
      _connected = false; 
    }
  }
  else
  {
    if (millis() - _checkTimer > 1000) // try reconnect every second
    {
      _checkTimer = millis();

      // If not connected, abort all pending messages
      _node->getCAN().abortAllPending();

      // This NMT command will try to set the IT2704 into operational mode
      // Only in this mode, CANopen is enabled and the device is responsive
      Msg.clearMsg();
      Msg.id        = 0;
      Msg.dlc       = 2;
      Msg.data[0]   = CO_NMT_ENTER_OPERATIONAL;
      Msg.data[1]   = _id;
      _node->sendMsg(Msg);

      // This message will query the IT2704 status
      // IT2704 will send a response on successfull reception of this message
      Msg.clearMsg();
      Msg.id        = (CO_CAN_ID_SDO_CLI + _id);
      Msg.dlc       = 8;
      Msg.setCommand(READ_REQ_1BYTE_CMD);
      Msg.setID(IT2704_COMMON);
      Msg.setSubID(COMMON_RESET);
      _node->sendMsg(Msg);
    }
  }
}

bool IT2704::checkConnection(const Message* MsgExt)
{
  Message Msg;

  if (_connected)
  {
    if (MsgExt->id == (CO_CAN_ID_SDO_SRV + _id))
    {
      // We have a valid SDO response message 
      // So, all ok and reset check timer
      _checkTimer = millis();
    }
    return true;
  }

  bool resetneeded;

  if (MsgExt->id == (CO_CAN_ID_SDO_SRV + _id))
  {
    // We have a valid SDO response message 
    // We are connected !!
    _connected = true;

    // Device is initialized, so no init reset needed
    resetneeded = false;
  }

  if (MsgExt->id == (CO_CAN_ID_HEARTBEAT + _id))
  {
    if (MsgExt->dlc == 1)
    {
      uint8_t IT2704_NMT_Status = MsgExt->data[0];

      if (IT2704_NMT_Status == CO_NMT_INITIALIZING)
      {
        // Startup CANopen message from IT2704 received
        // We are connected !!
        _connected = true;

        // Device is un-initialized, so init reset needed !!
        resetneeded = true;
      }
    }
  }

  if (_connected)
  {
    // Reset check timer
    _checkTimer = millis();

    // New connection , so init !!
    initDevice(resetneeded);
  }

  return false;
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
