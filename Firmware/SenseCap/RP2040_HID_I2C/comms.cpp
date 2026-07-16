#include "comms.h"

#if defined(ARDUINO_ARCH_SAMD)
MYSERCOM mysercom(PIN_WIRE_SERCOM);
TwoWire MyWire(&mysercom, PIN_WIRE_BATT_SDA, PIN_WIRE_BATT_SCL);
#endif

#ifdef ARDUINO_SEEED_INDICATOR_RP2040
#define DELAYUS(_us) busy_wait_us_32(_us)
#endif
#if defined(ARDUINO_ARCH_SAMD)  
#define DELAYUS(_us) delayMicroseconds(_us)
#endif

volatile THIDData HIDData[DAUGHTERBOARDCOUNT] = {0};

TBoardInfo BoardInfo = 
{
  #ifdef ARDUINO_SEEED_INDICATOR_RP2040
  true, // DataInValid
  #endif
  #if defined(ARDUINO_ARCH_SAMD)  
  false, // DataValid
  #endif
  {0}, // Default BoardSerial
  0 // Default BoardNumber
};

uint16_t UpdateCrc(uint16_t crc, const uint8_t* data_p, uint8_t length)
{
  uint8_t x;

  while (length--){
      x = crc >> 8 ^ *data_p++;
      x ^= x>>4;
      crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
  }
  return crc;
}

static inline bool CheckWireStuck(void)
{
  bool Result = false;
  Result |= (digitalRead(PIN_WIRE_BATT_SDA) == LOW);  
  Result |= (digitalRead(PIN_WIRE_BATT_SCL) == LOW);
  return Result;    
}

// Emulate opendrain pins
void PinLow(uint32_t ulPin)
{
  digitalWrite(ulPin, LOW);
  pinMode(ulPin, OUTPUT);
}
void PinHigh(uint32_t ulPin)
{
  pinMode(ulPin, INPUT_PULLUP);
  digitalWrite(ulPin, HIGH);  
}

void ResetWire(void)
{
  byte bits;

  #ifdef DEBUG
  Serial.println("Wire reset starting.");
  #endif

  pinMode(PIN_WIRE_BATT_SDA, INPUT_PULLUP);
  pinMode(PIN_WIRE_BATT_SCL, INPUT_PULLUP);

  #ifdef ARDUINO_SEEED_INDICATOR_RP2040
  gpio_set_function(PIN_WIRE_BATT_SCL, GPIO_FUNC_SIO);
  gpio_set_function(PIN_WIRE_BATT_SDA, GPIO_FUNC_SIO);
  #endif

  DELAYUS(100U);

  if ( (digitalRead(PIN_WIRE_BATT_SDA) == LOW) && (digitalRead(PIN_WIRE_BATT_SCL) == HIGH) )
  {
    #ifdef DEBUG
    Serial.println("Wire reset manual clock.");
    #endif

    // Send at max 9 clock pulses
    bits=0;
    while ( (bits++ < 9) && (digitalRead(PIN_WIRE_BATT_SDA) == LOW) )
    {
      PinLow(PIN_WIRE_BATT_SCL);
      DELAYUS(5);
      PinHigh(PIN_WIRE_BATT_SCL);
      DELAYUS(5);
    }

    if (digitalRead(PIN_WIRE_BATT_SDA) == HIGH)
    {
      // Bus recovered : send a STOP
      PinLow(PIN_WIRE_BATT_SDA);
      DELAYUS(5);
      PinHigh(PIN_WIRE_BATT_SDA);
    }
    #ifdef DEBUG
    Serial.printf("Wire reset manual clock bits needed: #%d.\r\n", bits);
    #endif
  }

  DELAYUS(100U);

  if (CheckWireStuck())
  {
    if (digitalRead(PIN_WIRE_BATT_SCL) == HIGH)
    {
      #ifdef DEBUG
      Serial.println("Wire reset for SCL low 50ms.");
      #endif
      // Force clock low for 50ms should reset all SAMD10 slaves.
      PinLow(PIN_WIRE_BATT_SCL);
      DELAYUS(50000U);
      PinHigh(PIN_WIRE_BATT_SCL);
    }
  }

  DELAYUS(100U);

  #ifdef DEBUG
  if (CheckWireStuck())
  {
    Serial.println("Resetting wire failed.");
  }
  else
  {
    Serial.println("Resetting wire successful.");
  }
  #endif
}

bool process_command(void const *data, void *result)
{
  CommandType_t cCmd = CMD_unknown;

  byte dataindexer,j,databyte;

  DWORD_VAL dw_data;
  WORD_VAL  w_data;

  byte* databuffer = (byte*)data;
  byte* resultbuffer = (byte*)result;

  bool DataToSend = true;
  bool GUIUpdateNeeded = false;
  bool StatusUpdateNeeded = false;

  byte PDOCount = 0;
  AP33772S_PDO PDO;

  #ifdef CONSOLE_DEBUG
  bool DEBUGONSCREEN = false;
  #endif

  PBatteryBoard LocalBatteryBoard = &BatteryBoards[0];

	cCmd=(CommandType_t)databuffer[COMMANDPOSITION];
  // device index = databuffer[INDEXPOSITION];
  // datalength = databuffer[LENGTHPOSITION];

  // Data start at position 3 !!
  // 0   = Command
  // 1   = Index
  // 2   = Length
  // 3.. = Data
  dataindexer = DATASTART;

  if (cCmd == CMD_set_MAXPDO)
  {
    #ifdef DEBUG
    Serial.printf("Max PDO.\r\n");
    #endif
    // PD Index
    LocalBatteryBoard->BM.pdoIndex=(databuffer[dataindexer++]);
    pd.setMaxPDO(LocalBatteryBoard->BM.pdoIndex);
  }

  if ( (cCmd == CMD_set_FIXEDPDO) || (cCmd == CMD_set_PPSPDO) || (cCmd == CMD_set_AVSPDO) )
  {
    // We got a SetPDO command
    StatusUpdateNeeded = true;
    // Process the settings !
    // Settings start at index DATASTART
    // PD Index
    LocalBatteryBoard->BM.pdoIndex=(databuffer[dataindexer++]);
    // targetVoltage
    for ( j=0; j<4; j++ ) {dw_data.v[j]=databuffer[dataindexer++];}
    LocalBatteryBoard->BM.targetVoltage=dw_data.Val;
    // maxCurrent
    for ( j=0; j<4; j++ ) {dw_data.v[j]=databuffer[dataindexer++];}
    LocalBatteryBoard->BM.maxCurrent=dw_data.Val;

    #ifdef DEBUG
    Serial.printf("Received SetPD command.\r\n");
    Serial.printf("PDO index:%d.\r\n", LocalBatteryBoard->BM.pdoIndex);
    Serial.printf("PDO voltage:%dmV.\r\n", LocalBatteryBoard->BM.targetVoltage);
    Serial.printf("PDO current:%dmA.\r\n", LocalBatteryBoard->BM.maxCurrent);
    #endif

    switch (cCmd)
    {
      case CMD_set_FIXEDPDO:
      {
        #ifdef DEBUG
        Serial.printf("Fixed PDO.\r\n");
        #endif

        LocalBatteryBoard->BM.pdoMode = pmFixed; 
        j = pd.setFixPDO(LocalBatteryBoard->BM.pdoIndex, LocalBatteryBoard->BM.maxCurrent);
        break;
      }
      case CMD_set_PPSPDO:
      {
        #ifdef DEBUG
        Serial.printf("PPS PDO.\r\n");
        #endif

        LocalBatteryBoard->BM.pdoMode = pmPPS; 
        j = pd.setPPSPDO(LocalBatteryBoard->BM.pdoIndex, LocalBatteryBoard->BM.targetVoltage, LocalBatteryBoard->BM.maxCurrent);
        break;
      }
      case CMD_set_AVSPDO:
      {
        #ifdef DEBUG
        Serial.printf("AVS PDO.\r\n");
        #endif

        LocalBatteryBoard->BM.pdoMode = pmAVS; 
        j = pd.setAVSPDO(LocalBatteryBoard->BM.pdoIndex, LocalBatteryBoard->BM.targetVoltage, LocalBatteryBoard->BM.maxCurrent);
        break;
      }
      default:
      {
        j = AP33772S_ERR_I2C;
      }
    }
  }

  if (cCmd == CMD_get_PDOList)
  {
    PDOCount = pd.readAllPDOs();

    #ifdef DEBUG
    Serial.printf("Received GetAllPDO command. PDOs: %d\r\n",PDOCount);
    #endif
  }

  if (cCmd == CMD_read_PDOList)
  {
    PDOCount = pd.getValidPDOCount();

    #ifdef DEBUG
    Serial.printf("Received read PDO list. PDOs: %d\r\n",PDOCount);
    #endif
  }

  if (cCmd == CMD_set_value)
  {
    #ifdef DEBUG
    Serial.printf("Received SetValue command.\r\n");
    #endif

    LocalBatteryBoard->BM.Status=TStageMode(databuffer[dataindexer]);
    for ( j=0; j<4; j++ ) {dw_data.v[j]=databuffer[dataindexer++];}
    LocalBatteryBoard->BM.SetValue=dw_data.Val;
    StatusUpdateNeeded = true;

    switch(LocalBatteryBoard->BM.Status)
    {
      case smCurrent :
      case smPower :
      case smResistor : 
      {
        #ifdef DEBUG
        Serial.printf("Output ON.\r\n");
        #endif

        pd.setOutput(true);
        break;
      }
      case smCharge :
      {
        break;
      }
      default :
      {
        #ifdef DEBUG
        Serial.printf("Output OFF.\r\n");
        #endif

        pd.setOutput(false);
      }
    }
    
  }

  if (cCmd == CMD_get_data)
  {
    #ifdef DEBUG
    Serial.printf("Received GetData command.\r\n");
    #endif

    float ina_mA  = ina238.getMilliAmpere();
    float ina_mV  = ina238.getBusMilliVolt();
    float ina_mW  = ina238.getMilliWatt();
    float ina_T   = ina238.getTemperature();

    LocalBatteryBoard->Voltage = round(ina_mV);
    LocalBatteryBoard->Current = round(ina_mA);
    LocalBatteryBoard->Power = round(ina_mW);
    LocalBatteryBoard->Temperature = round(ina_T * 10);

    GUIUpdateNeeded = true;
  }
  
  // Start processing the collected data !!
  {
    // Data start at position 3 !!
    // 0   = Command
    // 1   = Index
    // 2   = Length
    // 3.. = Data

    // Echo back command and index
	  resultbuffer[COMMANDPOSITION]=databuffer[COMMANDPOSITION];
    resultbuffer[INDEXPOSITION]=databuffer[INDEXPOSITION];

    // Skip length
    // Will be set in a later stage
    dataindexer = DATASTART;

    // Return data
    {
      if (cCmd == CMD_get_data)
      {
        //memcpy(&resultbuffer[i],&LocalBatteryBoard->Voltage, 2);
        //i += 2;
        w_data.Val = LocalBatteryBoard->Voltage;
        for ( j=0; j<2; j++ ) {resultbuffer[dataindexer++] = w_data.v[j];}
        w_data.Val = LocalBatteryBoard->Current;
        for ( j=0; j<2; j++ ) {resultbuffer[dataindexer++] = w_data.v[j];}
        dw_data.Val = LocalBatteryBoard->Power;
        for ( j=0; j<4; j++ ) {resultbuffer[dataindexer++] = dw_data.v[j];}
        w_data.Val = LocalBatteryBoard->Temperature;
        for ( j=0; j<2; j++ ) {resultbuffer[dataindexer++] = w_data.v[j];}

        #ifdef DEBUG
        /*
        Serial.printf("GetData command results.\r\n");
        Serial.printf("Voltage:%dmV.\r\n", LocalBatteryBoard->Voltage);
        Serial.printf("Current:%dmA.\r\n", LocalBatteryBoard->Current);
        Serial.printf("Temperature:%d°C.\r\n", (LocalBatteryBoard->Temperature / 10));
        */
        #endif

      }

      if (cCmd == CMD_set_value)
      {
        resultbuffer[dataindexer++]=(byte)LocalBatteryBoard->BM.Status;
        dw_data.Val = LocalBatteryBoard->BM.SetValue;
        for ( j=0; j<4; j++ ) {resultbuffer[dataindexer++] = dw_data.v[j];}
      }  

      if ( (cCmd == CMD_get_PDOList) || (cCmd == CMD_read_PDOList) )
      {
        resultbuffer[dataindexer++] = PDOCount;

        if (PDOCount>0)
        {
          for ( j=1; j<14; j++ )
          {
            if (pd.readPDO(j, PDO))
            {
              if (PDO.valid)
              {
                w_data.Val = PDO.raw;
                resultbuffer[dataindexer++] = PDO.index;
                resultbuffer[dataindexer++] = w_data.v[0];
                resultbuffer[dataindexer++] = w_data.v[1];
              }  
            }
          }
        }
      }

      if (cCmd == CMD_set_MAXPDO)
      {
        resultbuffer[dataindexer++] = LocalBatteryBoard->BM.pdoIndex;
        w_data.Val = 0;
        if (pd.readPDO(LocalBatteryBoard->BM.pdoIndex, PDO))
        {
          if (PDO.valid)
          {
            LocalBatteryBoard->BM.pdoMode=(TPDOMode)PDO.type;
            LocalBatteryBoard->BM.targetVoltage=PDO.maxVoltage_mV;
            LocalBatteryBoard->BM.maxCurrent=PDO.maxCurrent_mA;
            w_data.Val = PDO.raw;
          }
        }
        resultbuffer[dataindexer++] = w_data.v[0];
        resultbuffer[dataindexer++] = w_data.v[1];
      }

      if (cCmd == CMD_get_firmware)
      {
        LocalBatteryBoard->NeedsDataUpdate;
      }
    }

    // echo back length
    resultbuffer[LENGTHPOSITION]=dataindexer;
  }

  LocalBatteryBoard->NeedsGUIUpdate |= GUIUpdateNeeded;
  LocalBatteryBoard->NeedsStatusUpdate |= StatusUpdateNeeded;

  // Trivial: return true to indicate we have data to return.
  // Should always be true with this firmware !!
  return (DataToSend);
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* hid_report_out, uint16_t bufsize)
{
  (void) report_id;
  (void) report_type;
  (void) bufsize;

  byte hid_report_in[HID_INT_IN_EP_SIZE] = {0};
  byte j;

  bool SendData = true;
	byte cCmd = hid_report_out[0];

	hid_report_in[0] = cCmd;

	switch (cCmd)
  {
	  case 0:
		{
      // Should never happen !!
      break;
    }

	  case USB_CMD_get_serial:
		{
      for ( j=0; j<12; j++ )
      {
        hid_report_in[j+1] = BoardInfo.BoardSerial[j];
      }
      break;
    }

		case USB_CMD_set_serial:
		{
      for ( j=0; j<12; j++ )
      {
        BoardInfo.BoardSerial[j] = hid_report_out[j+1];
        hid_report_in[j+1] = BoardInfo.BoardSerial[j];
      }
      #ifdef ARDUINO_SEEED_INDICATOR_RP2040
      BoardInfo.InValid = false;
      #endif
      #if defined(ARDUINO_ARCH_SAMD)  
      BoardInfo.Valid = true;
      #endif
      storePutBoardInfo(&BoardInfo);
			break;
		}

		case USB_CMD_get_firmware:
		{
			hid_report_in[1]=FW_MAJOR;
			hid_report_in[2]=FW_MINOR;
			break;
		}

    case USB_CMD_get_board_number:
    {
      hid_report_in[1] = BoardInfo.BoardNumber;
      break;
    }

    case USB_CMD_set_board_number:
    {
      BoardInfo.BoardNumber = hid_report_out[1];
      #ifdef ARDUINO_SEEED_INDICATOR_RP2040
      BoardInfo.InValid = false;
      #endif
      #if defined(ARDUINO_ARCH_SAMD)  
      BoardInfo.Valid = true;
      #endif
      storePutBoardInfo(&BoardInfo);
      break;
    }

		default:
		{
      byte cBat=0;
      //cBat=hid_report_out[1];
      //if (cBat<DAUGHTERBOARDCOUNT)
      if (true)
      {
        SendData = false;
        for ( j=0; j<HID_INT_OUT_EP_SIZE; j++ ) HIDData[cBat].HIDEPOUTData[j] = hid_report_out[j];
        HIDData[cBat].DataReceived = true;
      }
      else
      {
        // Should never happen !!
        #ifdef DEBUG
        Serial.printf("Severe error. Wrong battery number:%d.\r\n", cBat);
        #endif
      }
    }
  }

  if (SendData)
  {
     // This delay seems necessary for communication with PC host
     // I do not know why ... :-()
    delayMicroseconds(1000U);

    // Send report back to host
    usb_hid.sendReport(0, hid_report_in, HID_INT_IN_EP_SIZE);
  }
}
