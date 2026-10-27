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
    uint8_t tpoindex;
    uint8_t localchannel;

    if (reset)
    {
      _node->getCAN().abortAllPending(true); // Discard all messages in hardware and queue

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

      /*

      // Try to set pre-operational mode
      // The IT2704 works in this mode, but does not send TPDOs
      // So, less traffic !
      Msg.clearMsg();
      Msg.data[0]   = CO_NMT_ENTER_PRE_OPERATIONAL;
      Msg.data[1]   = _id;
      _node->sendMsg(Msg);

      */

      Msg.clearMsg();

      Msg.id        = (CO_CAN_ID_SDO_CLI + _id);
      Msg.dlc       = 8;

      // Silence the 4 TPDOs
      // Should not be necessary if IT2704 is in pre-operational mode
      // But anyhow
      for ( tpoindex = 2; tpoindex < 4; tpoindex++ )
      {
        // Set Set the transmission type of TPDO#j to 254
        Msg.clearData();
        Msg.setCommand(WRITE_REQ_1BYTE_CMD);
        Msg.setID(TPO_TX_COMM_ID[tpoindex]);
        Msg.setSubID(SUBIDX_PDO_TRANSMISSION_TYPE);
        Msg.setData(PDO_TXTYPE_ASYNC_MANUFACTURER);
        _node->sendMsg(Msg);
        // This message will be confirmed
        // We might have a look at it

        //Modify TPDO#j timer cycle as 0ms
        Msg.clearData();
        Msg.setCommand(WRITE_REQ_2BYTE_CMD);
        Msg.setID(TPO_TX_COMM_ID[tpoindex]);
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


    /*

    // Configure Consumer Heartbeat Time for safety

    // First, set count to 0
    Msg.setCommand(WRITE_REQ_1BYTE_CMD);
    Msg.setID(CONSUMER_HEARTBEAT_INDEX);
    Msg.setSubID(SUBIDX_HIGHEST_SUPPORTED);
    Msg.setData(0);
    _node->sendMsg(Msg,true);    

    // Write heartbeat data
    Msg.setCommand(WRITE_REQ_4BYTE_CMD);
    Msg.setID(CONSUMER_HEARTBEAT_INDEX);
    Msg.setSubID(SUBIDX_1);
    // Where to listen to ?
    // At what interval time
    Msg.setData((MasterNodeID << 16) | (10 * 1000)); // 10 seconds interval for the Master
    _node->sendMsg(Msg,true);    

    // Last, set count to 1
    Msg.setCommand(WRITE_REQ_1BYTE_CMD);
    Msg.setID(CONSUMER_HEARTBEAT_INDEX);
    Msg.setSubID(SUBIDX_HIGHEST_SUPPORTED);
    Msg.setData(1);
    _node->sendMsg(Msg,true);    

    */

    // Set output slewrate
    // Coupling: off; On: slow ; off: max (fast)

    Msg.setCommand(WRITE_REQ_1BYTE_CMD);
    Msg.setID(IT2704_SOURCE_CURRENT + (_channel * 0x100));

    // Write slew couple/tracking off
    // This will allow off and on with different settings
    // This must be executed first !!
    Msg.setSubID(IT2704_SOURCE_CURRENT_CURRENT_SLEW_COUPLE);
    Msg.setData(0); // 0:Off 1:On
    _node->sendMsg(Msg, true);

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

    // Set inhibit mode
    // This must be executed first !!
    Msg.setCommand(WRITE_REQ_1BYTE_CMD);
    Msg.setSubID(IT2704_SOURCE_VOLTAGE_INHIBIT_MODE);
    Msg.setData(1); // 0:Off 1:live 2:latching
    //Msg.setData(2, true); // 0:Off 1:live 2:latching
    _node->sendMsg(Msg, true);

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


    // Disable auto-clear energy
    Msg.setCommand(WRITE_REQ_1BYTE_CMD);
    Msg.setID(IT2704_SENSE + (_channel * 0x100));
    Msg.setSubID(IT2704_SENSE_ENERGY_AUTOCLEAR_STATE);
    Msg.setData(0); // 0:Off 1:On
    _node->sendMsg(Msg);


    // Change TPDOs to send channel data !!
    // All messages have priority, die to the importance of the order (I presume)
    
    localchannel = _channel; // single channel setup

    //for ( localchannel = 0; localchannel < 8; localchannel++ ) // all channel setup
    {
      // We define three TPDOs per channel
      // TPDO1 = voltage + current
      // TPDO2 = power + energy
      // TPDO3 = capacity + time

      // Start TPDO index
      tpoindex = (localchannel * 3);

      for ( uint8_t tpochannelindex = 1; tpochannelindex <= 3; tpochannelindex++ )
      {
        // Disable TPDO: Set Bit 31 of COB-ID at 0x1800 + ID Sub01 to 1    
        Msg.clearData();
        Msg.setID(TPO_TX_COMM_ID[tpoindex]);
        Msg.setCommand(WRITE_REQ_4BYTE_CMD);
        Msg.setSubID(SUBIDX_PDO_COB_ID);
        data.Val = (0x80000000U); // bit 31 = 1 
        Msg.setData(data.Val);
        _node->sendMsg(Msg,true);

        // Write the new COB-ID into TPDO: Keep Bit 31 = 1
        Msg.clearData();
        Msg.setID(TPO_TX_COMM_ID[tpoindex]);
        Msg.setCommand(WRITE_REQ_4BYTE_CMD);
        Msg.setSubID(SUBIDX_PDO_COB_ID);
        data.Val = (0x80000000U + TPO_COB_ID_EXTENDED[tpoindex] + _id); // bit 31 = 1 
        Msg.setData(data.Val);
        _node->sendMsg(Msg,true);

        // Clear Mapping Count: Write 0 to 0x1A00 + ID Sub00 .
        Msg.clearData();
        Msg.setID(TPO_TX_MAP_ID[tpoindex]);
        Msg.setCommand(WRITE_REQ_1BYTE_CMD);
        Msg.setSubID(SUBIDX_PDO_MAP_COUNT);
        Msg.setData(0);
        _node->sendMsg(Msg,true);

        //Write Mapping Entry 1: Write mapping entry to 0x1A00 + ID Sub01.
        Msg.clearData();
        Msg.setID(TPO_TX_MAP_ID[tpoindex]);
        Msg.setCommand(WRITE_REQ_4BYTE_CMD);
        Msg.setSubID(SUBIDX_PDO_MAP_1);
        data.Val = 0;
        data.w[1] = IT2704_METER + (localchannel * 0x100);
        switch(tpochannelindex)
        {
          case 1: {data.v[1] = IT2704_METER_VOLTAGE; break;}
          case 2: {data.v[1] = IT2704_METER_POWER; break;}
          case 3: {data.v[1] = IT2704_METER_AHOUR; break;}
        }
        data.v[0] = SIZE_BIT_FLOAT32;
        Msg.setData(data.Val);
        _node->sendMsg(Msg,true);

        //Write Mapping Entry 2: Write mapping entry to 0x1A00 + ID Sub02.
        Msg.clearData();
        Msg.setID(TPO_TX_MAP_ID[tpoindex]);
        Msg.setCommand(WRITE_REQ_4BYTE_CMD);
        Msg.setSubID(SUBIDX_PDO_MAP_2);
        data.Val = 0;
        data.w[1] = IT2704_METER + (localchannel * 0x100);
        switch(tpochannelindex)
        {
          case 1: {data.v[1] = IT2704_METER_CURRENT; break;}
          case 2: {data.v[1] = IT2704_METER_WHOUR; break;}
          case 3: {data.v[1] = IT2704_METER_ON_TIME; break;}
        }
        data.v[0] = SIZE_BIT_FLOAT32;
        Msg.setData(data.Val);
        _node->sendMsg(Msg,true);

        // Set Mapping Count: Write 2 to 0x1A00 + ID Sub00 .
        Msg.clearData();
        Msg.setID(TPO_TX_MAP_ID[tpoindex]);
        Msg.setCommand(WRITE_REQ_1BYTE_CMD);
        Msg.setSubID(SUBIDX_PDO_MAP_COUNT);
        Msg.setData(2); // Two 32 bit floats per TPDO
        _node->sendMsg(Msg,true);

        // Enable TPDO: Clear Bit 31 of COB-ID at 0x1800 + ID Sub01 to 0    
        Msg.clearData();
        Msg.setID(TPO_TX_COMM_ID[tpoindex]);
        Msg.setCommand(WRITE_REQ_4BYTE_CMD);
        Msg.setSubID(SUBIDX_PDO_COB_ID);
        data.Val = (0x00000000U + TPO_COB_ID_EXTENDED[tpoindex] + _id); // bit 31 = 0 
        Msg.setData(data.Val);
        _node->sendMsg(Msg,true);

        tpoindex++;
      }
    }

    // We might use the remaining TPDOs for output status !!
    /*
    "TPDO1
    (0x180+ID)"	Channel1 onoff	char	1	"channel 2,5 on;channel 1,3,4,6,7,8 off
    00 01 00 00 01 00 00 00"	Only valid if the channel exists
	  Channel2 onoff	char	1		
	  Channel3 onoff	char	1		
	  Channel4 onoff	char	1		
	  Channel5 onoff	char	1		
	  Channel6 onoff	char	1		
	  Channel7 onoff	char	1		
	  Channel8 onoff	char	1		
    */


  }
}

void IT2704::clearEnergy(void)
{
  Message Msg;

  if (_connected)
  {
    Msg.clearMsg();

    Msg.id        = (CO_CAN_ID_SDO_CLI + _id);
    Msg.dlc       = 8;
    Msg.setCommand(WRITE_REQ_1BYTE_CMD);
    Msg.setID(IT2704_SENSE + (_channel * 0x100));
    Msg.setSubID(IT2704_SENSE_ENERGY_CLEAR);
    Msg.setData(1); // 1:Clear
    _node->sendMsg(Msg);
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

    _node->sendMsg(Msg);
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

    _node->sendMsg(Msg,true); // Setting output always with priority !!
  }
}

void IT2704::collectData(void)
{
  Message Msg;

  static bool ToggleDataRequest = false;

  if (_connected)
  {
    Msg.clearMsg();

    Msg.id        = (CO_CAN_ID_SDO_CLI + _id);
    Msg.dlc       = 8;

    Msg.setCommand(READ_REQ_4BYTE_CMD);
    Msg.setID(IT2704_METER + (_channel * 0x100));

    // Toogling needed due to some of these messages not getting through
    // So only two messages at a time !

    ToggleDataRequest = !ToggleDataRequest;

    if (ToggleDataRequest)
    {
      Msg.setSubID(IT2704_METER_VOLTAGE);
      _node->sendMsg(Msg);

      Msg.setSubID(IT2704_METER_CURRENT);
      _node->sendMsg(Msg);
    }
    else
    {
      Msg.setSubID(IT2704_METER_POWER);
      _node->sendMsg(Msg);

      Msg.setSubID(IT2704_METER_AHOUR);
      _node->sendMsg(Msg);
    }
    //Msg.setSubID(IT2704_METER_WHOUR);
    //_node->sendMsg(Msg);
  }
}

void IT2704::collectDataEnergyOnly(void)
{
  Message Msg;

  if (_connected)
  {
    Msg.clearMsg();

    Msg.id        = (CO_CAN_ID_SDO_CLI + _id);
    Msg.dlc       = 8;
    Msg.setCommand(READ_REQ_ANY_CMD);
    Msg.setID(IT2704_METER + (_channel * 0x100));
    Msg.setSubID(IT2704_METER_WHOUR);
    _node->sendMsg(Msg, true);
  }
}


void IT2704::getData(const Message* Msg, uint16_t* I, uint16_t* V, uint32_t* P, uint32_t* A, uint32_t* E)
{
    DWORD_FLOAT data; 
    data.Val = Msg->getData();
    data.f *= 1000; 
    switch(Msg->getSubID())
    {
      case IT2704_METER_VOLTAGE   : {*V = round(data.f); break;}
      case IT2704_METER_CURRENT   : {*I = round(data.f); break;}      
      case IT2704_METER_POWER     : {*P = round(data.f); break;}
      case IT2704_METER_AHOUR     : {*A = round(data.f); break;}      
      case IT2704_METER_WHOUR     : {*E = round(data.f); break;}
    }
}

bool IT2704::getTPDOEnergyData(const Message* Msg, uint32_t* E)
{

  if (Msg->id == (TPO_COB_ID[_channel / 2] + _id)) // We received a correct TPDO message from the IT2704
  {
    DWORD_FLOAT data; 

    if((_channel % 2) == 0)
    {
      memcpy(&data.v[0], &Msg->data[0], 4);
    }
    else
    {
      memcpy(&data.v[4], &Msg->data[4], 4);
    }

    *E = round(data.f * 1000);

    return true;
  }

  return false;
}

bool IT2704::isConnected(void)
{
  return _connected;
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

      _node->getCAN().abortAllPending(true); // Discard all messages in hardware and queue

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

bool IT2704::checkTxBuffer()
{
  MCP2515& mcp2515 = _node->getCAN();
  return (mcp2515.getTxFull());
}

bool IT2704::checkRxBuffer()
{
  MCP2515& mcp2515 = _node->getCAN();
  return (mcp2515.getRxFull());
}


bool IT2704::checkConnection(const Message* MsgExt)
{
  Message Msg;

  if (_connected)
  {
    if ( (MsgExt->id == (CO_CAN_ID_SDO_SRV + _id)) || (MsgExt->id == (TPO_COB_ID[_channel / 2] + _id)) )
    {
      // We have a valid SDO response message from the IT2704
      // We received a correct TPDO message from the IT2704
      // So, all ok and reset check timer
      _checkTimer = millis();
    }

    return true;
  }

  bool resetneeded;

  if ( (MsgExt->id == (CO_CAN_ID_SDO_SRV + _id)) || (MsgExt->id == (TPO_COB_ID[_channel / 2] + _id)) )
  {
    // We have a valid SDO response message from the IT2704
    // We received a correct TPDO message from the IT2704

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
