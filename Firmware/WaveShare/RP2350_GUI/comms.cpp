#include "comms.h"

#ifdef ENABLEUSB

#ifdef ARDUINO_ARCH_RP2040
#ifdef USE_TINYUSB
extern Adafruit_USBD_HID HID;
#endif
#endif

#ifdef ARDUINO_ARCH_ESP32
#if defined(ARDUINO_USB_MODE) && ARDUINO_USB_MODE == 0  // USB-OTG / TinyUSB
extern USBHID HID;
//extern USBCDC USBSerial;
#endif
#endif

#endif //ENABLEUSB

#if defined(ARDUINO_ARCH_SAMD)
MYSERCOM mysercom(PIN_WIRE_SERCOM);
TwoWire MyWire(&mysercom, PIN_WIRE_BATT_SDA, PIN_WIRE_BATT_SCL);
#endif

#ifdef ARDUINO_SEEED_INDICATOR_RP2040
#define DELAYUS(_us) busy_wait_us_32(_us)
#define USBSerial Serial
#include <PacketSerial.h>
extern COBSPacketSerial myPacketSerial; // for logging
#endif
#if defined(ARDUINO_ARCH_SAMD)  
#define DELAYUS(_us) delayMicroseconds(_us)
#define USBSerial Serial
#endif
#ifdef ARDUINO_ARCH_ESP32
#include "./src/UI/screenbase.h"
#define DELAYUS(_us) delayMicroseconds(_us)
#endif

#ifdef ARDUINO_ARCH_RP2040
#include "./src/UI/screenbase.h"
#define DELAYUS(_us) delayMicroseconds(_us)
#endif

static INA238 ina238(INA238_ADDRESS,(TwoWire*)&WireBattery);

CircularDataQueue<HidReportFrame, 8, true> hidQueue;

//THIDData* const CurrentHIDData = HIDData;

static float ina_mA_c       = 0;
static float ina_mV_c       = 0;
static float ina_mW_c       = 0;
#ifdef WITHINA238TEMPERATURE
static float ina_T_c        = 0;
#endif
static int   ina_counter_c  = 0;

TBoardInfo BoardInfo;

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

void Info_Add(const char *txt)
{
  if (txt == NULL) return;

  #ifdef DEBUG
  Serial.println(txt);
  #endif

  #if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_RP2040) 
  ScreenLogger_Add(txt,true);
  #endif

  #ifdef ARDUINO_SEEED_INDICATOR_RP2040
  // Send info to LCD
  int size = strlen(txt);
  uint8_t *s2 = (uint8_t*)malloc(size+2); // Add space for starting indicator and ending 0
  if (s2 == NULL) return;
  s2[COMMANDPOSITION] = CMD_logdata;
  memcpy((s2+1), txt, size);
  s2[size+1] = 0;
  myPacketSerial.send(s2, (size+2));
  free(s2);
  #endif
}

void Info_Add_Fmt(const char *format, ...)
{
  char myString[128];
  va_list args;
  va_start(args, format);
  int result = vsnprintf(myString, sizeof(myString), format, args);
  va_end(args);                    // Clean up
  if (result < 0) return;   // encoding error
  // Optional: detect truncation
  if (result >= (int)sizeof(myString)) {
    // message was truncated
  }  
  Info_Add(myString);
}

static inline bool CheckWireStuck(void)
{
  #if defined(ARDUINO_ARCH_ESP32)
  return WS_CH32_IO::checkI2CBus();
  #elif defined(ARDUINO_ARCH_RP2040) 
  return false; 
  #else
  bool Result = false;
  Result |= (digitalRead(PIN_WIRE_BATT_SDA) == LOW);  
  Result |= (digitalRead(PIN_WIRE_BATT_SCL) == LOW);
  return Result;    
  #endif
}

#if !defined(ARDUINO_ARCH_ESP32) && !defined(ARDUINO_ARCH_RP2040) 

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
#endif

void ResetWire(void)
{
  #if defined(ARDUINO_ARCH_ESP32)
  WS_CH32_IO::recoverI2CBus();
  #elif defined(ARDUINO_ARCH_RP2040) 
  return;
  #else
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
  #endif
}

bool initCANopen(void)
{
  // Init IT2704
  load.setID(BoardInfo.IT2704NodeID);
  load.setChannel(BoardInfo.IT2704Channel);

  // Init CANOpen
  node.setNodeId(BoardInfo.LocalNodeID);

  // Enable/Disable NMT state machine and PDO mapping (Default is enabled)
  ///node.disableNMT(true); 
  node.setHeartbeatTime(5000);      // 5 s heartbeat

  node.begin();

  return true;
}

bool initINA238(void)
{
  if(!ina238.begin())
  {
    Info_Add("Comms. Init INA238 failed !");
    return false;
  }
  else
  {
    ina238.reset();
    ina238.setADCRange(1);
    if ((BoardInfo.shuntcorrection<100) && (BoardInfo.shuntcorrection>-100))
    {
      ina238.setMaxCurrentShunt(7, (float)((5000.0+BoardInfo.shuntcorrection)/1000000.0) ); // Based on RotoPD Pro schematic
    }
    else
    {
      ina238.setMaxCurrentShunt(7, 0.005); // Based on RotoPD Pro schematic
    }
    ina238.setOverCurrentLimit(5500); // Max out 5,5A threshold

    ina238.setShuntVoltageConversionTime(INA238_150_us);
    #ifdef WITHINA238TEMPERATURE
    ina238.setMode(INA238_MODE_CONT_TEMP_BUS_SHUNT);
    #else
    ina238.setMode(INA238_MODE_CONT_BUS_SHUNT);
    #endif
    //ina238.setBusVoltageConversionTime(uint8_t bvct = INA238_1052_us);
    //ina238.setTemperatureConversionTime(uint8_t tct = INA238_1052_us);
    //ina238.setCurrentConversionTime(INA2XX_TIME_280_us);    

    ina238.setAverage(INA238_16_SAMPLES); 
    //ina238.setDiagnoseAlertBit(INA238_DIAG_ALERT_LATCH); //Set to Alert latch


    ina238.setDiagnoseAlertBit(INA238_DIAG_SHUNT_OVER_LIMIT);
    ina238.clearDiagnoseAlertBit(INA238_DIAG_ALERT_LATCH); //Set to Transparent
    ina238.clearDiagnoseAlertBit(INA238_DIAG_ALERT_POLARITY);

    Info_Add("Comms. Init INA238 success.");
  }
  return true;
}

void collectRotoPDData(void)
{
  ina_mA_c      += ina238.getMilliAmpere();
  ina_mV_c      += ina238.getBusMilliVolt();
  ina_mW_c      += ina238.getMilliWatt();
  #ifdef WITHINA238TEMPERATURE
  ina_T_c       += ina238.getTemperature();
  #endif
  ina_counter_c++;
}

void getRotoPDData(uint16_t* I,uint16_t* V,uint32_t* P,uint16_t* T)
{
  float li;
  if (ina_counter_c == 0)
  {
    // Get latest data
    li = ina238.getMilliAmpere();
    if (li<0) li=0;
    *I = lroundf(li);
    *V = lroundf(ina238.getBusMilliVolt());
    *P = lroundf(ina238.getMilliWatt());
    #ifdef WITHINA238TEMPERATURE
    *T = lroundf(ina238.getTemperature() * 10);
    #endif
  }
  else
  {
    // Get average data
    li = (ina_mA_c / ina_counter_c);
    if (li<0) li=0;
    *I = lroundf(li);
    *V = lroundf(ina_mV_c / ina_counter_c);
    *P = lroundf(ina_mW_c / ina_counter_c);
    #ifdef WITHINA238TEMPERATURE
    *T = lroundf(((ina_T_c *10) / ina_counter_c));
    #endif
    ina_mA_c      = 0;
    ina_mV_c      = 0;
    ina_mW_c      = 0;
    #ifdef WITHINA238TEMPERATURE
    ina_T_c       = 0;
    #endif
    ina_counter_c = 0;
  }

  #ifdef REV11
  // Compensate for XRS40N10 Static Drain-Source On-Resistance and shunt resistor
  word VC = ((*I * (XRS40N10ONRESISTANCE+SHUNTRESISTANCE)) / 1000);
  *V += VC;
  #endif
}

bool initROTOPD(void)
{
  // RotoPD Pro setup
  {
    // Required for RotoPD Pro. Prevent UVP from issuing hard reset.
    // VOUT connected to +5V
    pd.clearConfig(CONFIG_UVP_EN);
    pd.clearConfig(CONFIG_OVP_EN);
    pd.clearConfig(CONFIG_OCP_EN);

    if (pd.begin() != AP33772S_OK)
    {
      delay(500);
      if (pd.begin() != AP33772S_OK)
      {
        Info_Add("Comms. Init AP33772S failed !");
        pd.dumpRegisters(Serial);
        return (false);
      }
    }
    // Protection thresholds
    // Temperature only for RotoPD Pro
    // VOUT ISENSP AND VCC are connected to +5V
    //pd.setOVPOffset_mV(2000);
    //pd.setUVPThreshold(UVP_80PCT);


    pd.setOCPThreshold_mA(0);      // auto = 110% of PDO

    pd.setOTPThreshold_C(120);
    pd.setConfig(CONFIG_OTP_EN);
    pd.setDeratingThreshold_C(85);
    pd.setConfig(CONFIG_DR_EN);

    // Switch off output
    //pd.setOutput(false);

    Info_Add("Comms. Init AP33772S success.");
    if (pd.getOutput())
      Info_Add("Comms. AP33772S output on.");
    else
      Info_Add("Comms. AP33772S output off.");
    
    // Interrupts
    //pd.setInterruptMask(MASK_ALL);
    //pd.attachInterruptCallback(pdISR);

  }
  return (true);
}

int8_t taskRotoPDInit(void)
{
  // To be done !!

  byte j = 0;
  byte PDOCount = -1;

  if (pd.isConnected())
  {
    #ifdef DEBUG
    //Serial.println("RotoPD Pro connected.");
    #endif

    j = pd.getStatus();

    #ifdef DEBUG
    //Serial.printf("Status:= 0x%02X (%d).\r\n", (uint8_t)(j<0?0xFF:j), (uint8_t)(j<0?0:j));
    #endif

    if (j & STATUS_FAULTS)
    {
      char error_label[4];
      error_label[0] = '\0';
      switch(j)
      {
        case STATUS_OVP : {strcpy(error_label,"OVP"); break;}
        case STATUS_UVP : {strcpy(error_label,"UVP"); break;}
        case STATUS_OCP : {strcpy(error_label,"OCP"); break;}
        case STATUS_OTP : {strcpy(error_label,"OTP"); break;}
        default         : {strcpy(error_label,"UNK"); break;}
      }
      Info_Add_Fmt("Comms. [RotoPD] RotoPD fault: %s.",error_label);
    }

    if (j & STATUS_STARTED)
    {
      PDOCount = 0;

      Info_Add("Comms. RotoPD Pro started.");
      Info_Add_Fmt("Comms. Status = 0x%02X (%d).", (uint8_t)(j<0?0xFF:j), (uint8_t)(j<0?0:j));

      PDOCount = pd.getValidPDOCount();

      Info_Add_Fmt("Comms. Initial PDO count: %d.",PDOCount);

      // We have a power up !!
      // Init the AP33772S / RotoPD
      if (initROTOPD())
      {
        // Check if we already have the new PDO's
        if (((j & STATUS_NEWPDO)  && (j & STATUS_READY)) || (PDOCount>0) || (pd.waitForPDOs(2000)==AP33772S_OK))
        {
          // Request / read list of PDOs
          if (PDOCount == 0) PDOCount = pd.readAllPDOs();

          Info_Add_Fmt("Comms. New PDO's !! PDO count: %d.",PDOCount);

          if (PDOCount>0)
          {
            #ifdef DEBUG
            Serial.println("PDO list below.");
            pd.printPDOs(Serial);
            Serial.println("Done.");
            #endif
          }
        }

        j = pd.getOpMode();

        if (j & OPMODE_PDMOD) Info_Add("Comms. [RotoPD] PD connected");
        if (j & OPMODE_LGCYMOD) Info_Add("Comms. [RotoPD] legacy mode");
        if (j & OPMODE_CCFLIP) Info_Add("Comms. [RotoPD] cable flipped");
      }
      else
      {
        if (j & OPMODE_DR) Info_Add("Comms. [RotoPD] Init error !");
      }
    
    }
    else
    {
      j = pd.getOpMode();
      if (j & OPMODE_DR) Info_Add("Comms. [RotoPD] derating !!");
    }
  }

  return (PDOCount);
}

bool process_command(void const *data, void *result)
{
  CommandType_t cCmd = CMD_unknown;
  byte BN = 0;

  byte dataindexer,j,databyte;

  DWORD_VAL dw_data;
  WORD_VAL  w_data;

  byte* databuffer = (byte*)data;
  byte* resultbuffer = (byte*)result;

  bool DataToSend = true;
  bool GUIUpdateNeeded = false;
  bool StatusUpdateNeeded = false;

  bool Engage = false;

  byte PDOCount = 0;
  AP33772S_PDO PDO;

  #ifdef CONSOLE_DEBUG
  bool DEBUGONSCREEN = false;
  #endif

	cCmd  = (CommandType_t)databuffer[COMMANDPOSITION];
  BN    = databuffer[BOARDPOSITION];
  // datalength = databuffer[LENGTHPOSITION];

  // Data start at position 3 !!
  // 0   = Command
  // 1   = BoardNumber
  // 2   = Length
  // 3.. = Data
  dataindexer = DATASTART;

  switch (cCmd)
  {
    case CMD_set_batterymode:
    {
      // Unfortunately, we have to set the batterymode by ourselves !!
      CurrentBoard->TD.Active = (TBatteryMode)databuffer[dataindexer++];
      break;
    }
    case CMD_set_threshold:
    {
      // Get the stagemode to be used
      TStageMode SM = (TStageMode)databuffer[dataindexer++];
      PStageData SD = GetCorrectStageSettings(CurrentSettings, SM);
      if (SD != NULL)
      {
        // Get the thresholdmode to be used
        TThresholdModes TM = (TThresholdModes)databuffer[dataindexer++];
        if (TM >= tmLast) TM = tmNONE;
        if (TM != tmNONE)
        {
          // Now get the data to set the correct threshold
          PThresholdDataItem TS = &SD->ThresholdSettings[TM];
          TS->Enabled = (databuffer[dataindexer++] != 0);
          for ( j=0; j<2; j++ ) {dw_data.v[j] = databuffer[dataindexer++];}
          TS->SetValue = dw_data.Val;
          //TS->Triggered = (databuffer[dataindexer++] != 0);
        }
      }
      break;
    }
    case CMD_set_MAXPDO:
    case CMD_set_FIXEDPDO:
    case CMD_set_PPSPDO:
    case CMD_set_AVSPDO:
    {
      // We got a SetPDO command
      StatusUpdateNeeded = true;
      // Process the settings !
      // Settings start at index DATASTART
      // PD Index
      CurrentBoard->PDO.index=(databuffer[dataindexer++]);
      // maxCurrent
      for ( j=0; j<4; j++ ) {dw_data.v[j]=databuffer[dataindexer++];}
      CurrentBoard->PDO.maxCurrent_mA=dw_data.Val;
      // targetVoltage
      for ( j=0; j<4; j++ ) {dw_data.v[j]=databuffer[dataindexer++];}
      CurrentBoard->PDO.targetVoltage_mV=dw_data.Val;    

      Info_Add("Comms. Received SetPD command.");
      Info_Add_Fmt("Comms. PDO requested index: #%d.", CurrentBoard->PDO.index);
      if (cCmd != CMD_set_MAXPDO)
      {
        Info_Add_Fmt("Comms. PDO requested current: %dmA.", CurrentBoard->PDO.maxCurrent_mA);
        Info_Add_Fmt("Comms. PDO target voltage: %dmV.", CurrentBoard->PDO.targetVoltage_mV);
      }

      // Always stop the load current before switching PDO's
      // Just to be sure
      load.setOutput(false);

      switch (cCmd)
      {
        case CMD_set_MAXPDO:
        {
          Info_Add("Comms. Set max PDO.");
          
          CurrentBoard->PDO.type = PDO_TYPE_MAX; 
          pd.setMaxPDO(CurrentBoard->PDO.index);

          j = AP33772S_OK;

          break;
        }
        case CMD_set_FIXEDPDO:
        {
          Info_Add("Comms. Set fxed PDO.");

          CurrentBoard->PDO.type = PDO_TYPE_FIXED; 
          j = pd.setFixPDO(CurrentBoard->PDO.index, CurrentBoard->PDO.maxCurrent_mA);
          if (AP33772S_OK != j)
          {
            Info_Add("Comms. Set max PDO returned an error.");
          }

          break;
        }
        case CMD_set_PPSPDO:
        {
          Info_Add("Comms. Set PPS PDO.");

          CurrentBoard->PDO.type = PDO_TYPE_PPS; 
          j = pd.setPPSPDO(CurrentBoard->PDO.index, CurrentBoard->PDO.targetVoltage_mV, CurrentBoard->PDO.maxCurrent_mA);
          if (AP33772S_OK != j)
          {
            Info_Add("Comms. Set PPS PDO returned an error.");
          }

          break;
        }
        case CMD_set_AVSPDO:
        {
          Info_Add("Comms. Set AVS PDO.");

          CurrentBoard->PDO.type = PDO_TYPE_AVS; 
          j = pd.setAVSPDO(CurrentBoard->PDO.index, CurrentBoard->PDO.targetVoltage_mV, CurrentBoard->PDO.maxCurrent_mA);
          if (AP33772S_OK != j)
          {
            Info_Add("Comms. Set AVS PDO returned an error.");
          }

          break;
        }
        default:
        {
          j = AP33772S_ERR_I2C;
        }
      }

      if (j == AP33772S_OK)
      {
        if (pd.readPDO(CurrentBoard->PDO.index, CurrentBoard->PDO))
        {
          if (PDO.valid)
          {
            // Set the PDO with the real values
            CurrentBoard->PDO.maxCurrent_mA    = pd.getRequestedCurrent_mA();
            CurrentBoard->PDO.targetVoltage_mV = pd.getRequestedVoltage_mV();
          }
        }
      }
      break;
    }
    case CMD_get_PDOList:
    {
      PDOCount = pd.readAllPDOs();
      Info_Add_Fmt("Comms. Received GetAllPDO command. PDOs: %d.",PDOCount);
      break;
    }
    case CMD_read_PDOList:
    {
      PDOCount = pd.getValidPDOCount();
      Info_Add_Fmt("Comms. Received read PDO list. PDOs: %d.",PDOCount);
      break;
    } 
    case CMD_set_output:
    {
      Engage = (databuffer[dataindexer++] != 0);
      if (Engage)
      {
        Info_Add("Comms. Output ON.");
      }
      else
      {
        Info_Add("Comms. Output OFF.");
      }
      if (!Engage) load.setOutput(Engage);
      //pd.setOutput(Engage);
      if (Engage) load.setOutput(Engage);
      break;
    }
    case CMD_set_value:
    {
      CurrentBoard->TD.SetStageMode=TStageMode(databuffer[dataindexer++]);
      for ( j=0; j<4; j++ ) {dw_data.v[j]=databuffer[dataindexer++];}
      CurrentBoard->TD.SetStageValue=dw_data.Val;

      PStageData SD = GetCorrectStageSettings(CurrentSettings, CurrentBoard->TD.SetStageMode);
      if (SD != NULL)
      {
        // Adjust the settings with the correct values
        SD->Status = CurrentBoard->TD.SetStageMode;
        SD->SetValue = CurrentBoard->TD.SetStageValue;

        // Reset all triggers
        for(j = (tmNONE+1); j < tmLast; j++)
        {
          TThresholdModes TM = (TThresholdModes)j;
          PThresholdDataItem TS = &SD->ThresholdSettings[TM];
          TS->Triggered = false;
        }
      }

      Info_Add_Fmt("Comms. Received SetValue command. Value: %d.",CurrentBoard->TD.SetStageValue);

      StatusUpdateNeeded = true;
      Engage = false;

      // When off or zero, alsways first switch-off output.
      if ( (CurrentBoard->TD.SetStageMode == smUnknown) || (CurrentBoard->TD.SetStageMode == smZero) || (CurrentBoard->TD.SetStageMode == smDisabled) || (CurrentBoard->TD.SetStageValue == 0) )
      {
        if (CurrentBoard->TD.SetStageValue == 0)
        {
          Info_Add("Comms. SetValue 0. Output off !");      
        } 
        else
        {
          Info_Add("Comms. Status Off. Output off !");
        }
        load.setOutput(false);        
        //pd.setOutput(false);
      }

      switch(CurrentBoard->TD.SetStageMode)
      {
        case smCurrent :
        {
          Info_Add("Comms. SetValue current.");
          load.setCurrent(CurrentBoard->TD.SetStageValue);
          Engage = true;
          break;
        }
        case smPower :
        {
          Info_Add("Comms. SetValue power.");
          load.setPower(CurrentBoard->TD.SetStageValue);
          Engage = true;
          break;
        }
        case smResistor : 
        {
          Info_Add("Comms. SetValue resistor.");
          load.setResistance(CurrentBoard->TD.SetStageValue);
          Engage = true;
          break;
        }
        case smCharge :
        {
          break;
        }
        default :
        {
          break;
        }
      }

      if (Engage) Engage = (CurrentBoard->TD.SetStageValue != 0);
      if (Engage)
      {
        Info_Add("Comms. SetValue active. Output on !");
        //pd.setOutput(true);
        load.setOutput(true);
      } 

      break;
    }
    case CMD_get_data:
    {
      //Info_Add("Comms. Received GetData command.");
      // Collect RotoPD data from hardware
      getRotoPDData(&CurrentBoard->RotoPDData.I,&CurrentBoard->RotoPDData.V,&CurrentBoard->RotoPDData.P,&CurrentBoard->RotoPDData.T);
      GUIUpdateNeeded = true;
      break;
    } 
  }

  // Start processing the collected data !!
  
  // Data start at position 3 !!
  // 0   = Command
  // 1   = BoardNumber
  // 2   = Length
  // 3.. = Data

  // Echo back command and index/number
  resultbuffer[COMMANDPOSITION]=cCmd;
  //resultbuffer[BOARDPOSITION]=BN;
  resultbuffer[BOARDPOSITION]=BoardInfo.getBoardNumber();

  if (BN != BoardInfo.getBoardNumber())
  {
    // Should never happen !!
    Info_Add("Comms. Severe error. Wrong boardnumber !!!");
    Info_Add_Fmt("Comms. Received board %d. BoardInfo has %d.", BN, BoardInfo.getBoardNumber());
  }

  // Skip length
  // Will be set in a later stage
  dataindexer = DATASTART;

  // Return data, if any

  switch (cCmd)
  {
    case CMD_get_data:
    {
      //memcpy(&resultbuffer[i],&CurrentBoard->Voltage, 2);
      //i += 2;
      // RotoPD data
      w_data.Val = CurrentBoard->RotoPDData.V;
      for ( j=0; j<2; j++ ) {resultbuffer[dataindexer++] = w_data.v[j];}
      w_data.Val = CurrentBoard->RotoPDData.I;
      for ( j=0; j<2; j++ ) {resultbuffer[dataindexer++] = w_data.v[j];}
      dw_data.Val = CurrentBoard->RotoPDData.P;
      for ( j=0; j<4; j++ ) {resultbuffer[dataindexer++] = dw_data.v[j];}
      w_data.Val = CurrentBoard->RotoPDData.T;
      for ( j=0; j<2; j++ ) {resultbuffer[dataindexer++] = w_data.v[j];}

      if (load.isConnected())
      {
        load.getData(&CurrentBoard->IT2704Data.I, &CurrentBoard->IT2704Data.V, &CurrentBoard->IT2704Data.P, &CurrentBoard->IT2704Data.C, &CurrentBoard->IT2704Data.E);
        // IT2704 data
        w_data.Val  = CurrentBoard->IT2704Data.V;
        for ( j=0; j<2; j++ ) {resultbuffer[dataindexer++] = w_data.v[j];}
        w_data.Val  = CurrentBoard->IT2704Data.I;
        for ( j=0; j<2; j++ ) {resultbuffer[dataindexer++] = w_data.v[j];}
        dw_data.Val = CurrentBoard->IT2704Data.P;
        for ( j=0; j<4; j++ ) {resultbuffer[dataindexer++] = dw_data.v[j];}
        dw_data.Val = CurrentBoard->IT2704Data.C;
        for ( j=0; j<4; j++ ) {resultbuffer[dataindexer++] = dw_data.v[j];}
        dw_data.Val = CurrentBoard->IT2704Data.E;
        for ( j=0; j<4; j++ ) {resultbuffer[dataindexer++] = dw_data.v[j];}
      }
      break;
    }
    case CMD_set_value:
    {
      resultbuffer[dataindexer++]=(byte)CurrentBoard->TD.SetStageMode;
      dw_data.Val = CurrentBoard->TD.SetStageValue;
      for ( j=0; j<4; j++ ) {resultbuffer[dataindexer++] = dw_data.v[j];}
      break;
    }
    case CMD_get_PDOList:
    case CMD_read_PDOList:
    {
      Info_Add_Fmt("Comms. Processing available [#%d] PDOs.",PDOCount);

      if  ((PDOCount>=0) && (PDOCount<=MAX_PDO_ENTRIES))
      {
        resultbuffer[dataindexer++] = PDOCount;

        if  (PDOCount>0)
        {
          for ( j=1; j<=MAX_PDO_ENTRIES; j++ )
          {
            if (pd.readPDO(j, PDO))
            {
              if (PDO.valid)
              {
                Info_Add_Fmt("Comms. Sending PDO #%d.",PDO.index);

                resultbuffer[dataindexer++] = PDO.index;
                w_data.Val = PDO.raw;
                resultbuffer[dataindexer++] = w_data.bytes.LB;
                resultbuffer[dataindexer++] = w_data.bytes.HB;
              }  
            }
          }
        }

      }
      else
      {
        Info_Add("Comms. Invalid PDO count.");
        resultbuffer[dataindexer++] = 0;
      }

      break;
    }
    case CMD_set_MAXPDO:
    case CMD_set_FIXEDPDO:
    case CMD_set_PPSPDO:
    case CMD_set_AVSPDO:
    {
      resultbuffer[dataindexer++] = CurrentBoard->PDO.index;
      
      w_data.Val = CurrentBoard->PDO.maxCurrent_mA;
      resultbuffer[dataindexer++] = w_data.v[0];
      resultbuffer[dataindexer++] = w_data.v[1];

      w_data.Val = CurrentBoard->PDO.targetVoltage_mV;
      resultbuffer[dataindexer++] = w_data.v[0];
      resultbuffer[dataindexer++] = w_data.v[1];

      w_data.Val = CurrentBoard->PDO.raw;
      resultbuffer[dataindexer++] = w_data.v[0];
      resultbuffer[dataindexer++] = w_data.v[1];

      break;
    }
  }

  if (dataindexer == DATASTART)
  {
    uint8_t length = databuffer[LENGTHPOSITION];
    for ( j=0; j<length; j++ )
    {
      // Just echo back what we got
      resultbuffer[dataindexer]=databuffer[dataindexer];
      dataindexer++;
    }
  }

    // echo back length
  resultbuffer[LENGTHPOSITION]=(dataindexer-DATASTART);

  CurrentBoard->NeedsGUIUpdate |= GUIUpdateNeeded;
  CurrentBoard->NeedsStatusUpdate |= StatusUpdateNeeded;

  // Trivial: return true to indicate we have data to return.
  // Should always be true with this firmware !!
  return (DataToSend);
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
#ifdef ENABLEUSB
#ifdef ARDUINO_ARCH_ESP32
void set_report_callback(uint8_t report_id, uint8_t const* hid_report_out, uint16_t bufsize)
#else
void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* hid_report_out, uint16_t bufsize)
#endif
{
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
      BoardInfo.setValid(true);
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
      hid_report_in[1] = BoardInfo.getBoardNumber();
      //hid_report_in[1] = (BoardInfo.IT2704Channel + 1);
      break;
    }

    case USB_CMD_set_board_number:
    {
      // Do not allow setting of board-number !!
      // This will be done through GUI by setting IT2704 channel !!
      /*
      BoardInfo.setValid(true);
      storePutBoardInfo(&BoardInfo);
      */      
      break;
    }

		default:
		{
      //byte cBat=hid_report_out[1];
      byte cBat=0;
      if (cBat<DAUGHTERBOARDCOUNT)
      {
        SendData = false;

        if (report_type == HID_REPORT_TYPE_OUTPUT)
        {
          HidReportFrame frame;
          frame.report_id = report_id;
          frame.len = (bufsize > HID_INT_OUT_EP_SIZE) ? HID_INT_OUT_EP_SIZE : bufsize;
          memcpy(frame.data, hid_report_out, frame.len);
          // Push into queue non-blockingly (tryPush returns false if queue is full)
          hidQueue.tryPush(frame);
        }
      }
      else
      {
        // Should never happen !!
        Info_Add_Fmt("Comms. Severe error. Wrong battery number:%d.", cBat);
      }
    }
  }

  if (SendData)
  {
     // This delay seems necessary for communication with PC host
     // I do not know why ... :-()
    #ifdef ARDUINO_ARCH_ESP32
    delayMicroseconds(1000U);
    #endif

    // Send report back to host
    #ifdef ARDUINO_ARCH_ESP32
    HID.SendReport(0, hid_report_in, HID_INT_IN_EP_SIZE, /* timeout [ms] = */ 100 );
    #else
    if (HID.ready()) HID.sendReport(0, hid_report_in, HID_INT_IN_EP_SIZE);
    #endif

  }
}
#endif // ENABLEUSB
