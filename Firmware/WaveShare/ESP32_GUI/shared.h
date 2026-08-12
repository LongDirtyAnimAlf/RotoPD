#ifndef _SHARED_H_
#define _SHARED_H_

#include <Arduino.h>
//#include "longunion.h"

#define    DEBUG

#ifndef ARDUINO_ARCH_SAMD
// Only SenseCap can be standalone !!
#define    STANDALONE
#endif

#define    DAUGHTERBOARDCOUNT         1

#define    FW_MAJOR                   56
#define    FW_MINOR                   14

#define    DATASIZE                   600u

#define    DVTIME                     (1000u * 60u * 10u) // 10 minutes in ms
#define    PVTIME                     (1000u * 60u * 10u) // 10 minutes in ms

#define INA238_ADDRESS                0x40

#ifdef CFG_TUD_ENDPOINT0_SIZE
#undef CFG_TUD_ENDPOINT0_SIZE
#endif

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE 64
#endif

#define HID_INT_IN_EP_SIZE     CFG_TUD_ENDPOINT0_SIZE
#define HID_INT_OUT_EP_SIZE    CFG_TUD_ENDPOINT0_SIZE

#define COMMANDPOSITION   (0)
#define INDEXPOSITION     (1)
#define LENGTHPOSITION    (2)
#define DATASTART         (3)

#define COMMAND_SIZE      (64)

#define    USB_CMD_get_serial         0x64
#define    USB_CMD_set_serial         0x65
#define    USB_CMD_get_firmware       0xC8
#define    USB_CMD_get_board_number   0xCA
#define    USB_CMD_set_board_number   0xCB

#define    USB_CMD_error              0xFF

//typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;

typedef union
{
    word Val;
    byte v[2];// __PACKED;
    struct //__PACKED
    {
        byte LB;
        byte HB;
    } bytes;
} WORD_VAL, WORD_BITS;

typedef union
{
    dword Val;
    byte v[4];// __PACKED;
    word w[2];// __PACKED;
    struct //__PACKED
    {
        byte LB;
        byte HB;
        byte UB;
        byte MB;
    } bytes;
} DWORD_VAL, DWORD_BITS;

#define IDDLESTAGENUMBER              0
#define FIXEDDISCHARGESTAGENUMBER     1
#define FIXEDCHARGESTAGENUMBER        2

typedef enum {
  	CMD_unknown          = 0x00,  
  	CMD_error            = 0x40,  
  	CMD_get_data         = 0x50,
  	CMD_get_status,
  	CMD_get_hardware,
  	CMD_get_firmware,
    CMD_set_value,
  	CMD_get_PDOList,
  	CMD_read_PDOList,
  	CMD_set_FIXEDPDO,
  	CMD_set_PPSPDO,
  	CMD_set_AVSPDO,
  	CMD_set_MAXPDO,
    CMD_controller_reset = 0xB0
} CommandType_t;

typedef enum 
{
  smOff = 0,
  smCurrent,
  smPower,
  smResistor,
  smReserve,
  smCharge,
  smZero,
  smDisabled
} TStageMode;

typedef enum
{
  tmNONE=0,
  tmMAXV,
  tmMINV,
  tmDELTAV,
  tmPLATEAUV,
  tmABST,
  tmDELTAT,
  tmDVT,
  tmTIME,
  tmMAXC,
  tmLast
} TThresholdModes;

typedef struct
{
  TThresholdModes Mode;
  word SetValue;
  bool Enabled;
  bool Triggered;
  word GetValue;
} TThresholdDataItem;

typedef struct
{
  word V;
  word I;
  dword P;
  //dword E;
  word T;
} TMeasurementData,*PMeasurementData;

typedef struct
{
  TMeasurementData *BatteryDatas;
  TMeasurementData LastBatteryData;
  qword Capacity; // in nAh
  qword Energy; // in nWh
  qword Time;  // in deci-seconds = 100ms
  //word Temperature; // in deci-Celcius = 100mC
  int16_t Tail;
  int16_t Head;
  byte CurrentStageNumber;
  TThresholdDataItem ThresholdResult[tmLast]; // all threshold possibilities
} TRunDatas,*PRunDatas;

typedef enum
{
  ieOk = 0,
  // Standard I2C errors from Arduino Wire Library
  ieDataLong,       //  1 : Data too long
  ieAddressNACK,    //  2 : NACK on transmit of address
  ieDataNACK,       //  3 : NACK on transmit of data
  ieTransmit,       //  4 : Other error : could not send the required amount of data bytes
  ieTimeOut,        //  5 : Timeout [on RP2040]
  // Custom I2C errors
  ieStuckWire,
  ieBusyWire,
  ieEmptyWire,
  ieDataLength,
  ieDataMismatch,
  ieCRCError,  
} TI2CError;

typedef struct
{
  TI2CError I2CError;
  byte I2CBytesReceived;
  byte I2CAddress;
} TI2CStatus,*PI2CStatus;

typedef enum
{
  bmIdle = 0,
  bmActive,
  bmOff,
  bmReady,
  bmError
} TBatteryMode;

typedef enum
{
  pmFixed = 0,
  pmPPS,
  pmAVS,
} TPDOMode;



typedef struct
{
  TStageMode Status;
  TPDOMode pdoMode;
  int pdoIndex;
  int targetVoltage;
  int maxCurrent;
  dword SetValue;
  TThresholdDataItem ThresholdSettings[tmLast]; // all threshold possibilities
} TStageData, *PStageData;

typedef struct
{
  TBatteryMode         Active;
  TStageMode           SetStageMode;
  dword                SetStageValue;
  TThresholdModes      ThresholdMode;
  dword                ThresholdValue;
  byte                 DataTriggerCounter;
  TRunDatas            RunDatas;
} TTestData;

typedef struct {
  #ifdef ARDUINO_SEEED_INDICATOR_RP2040
  bool InValid;
  #endif
  #if defined(ARDUINO_ARCH_SAMD)  
  bool Valid;
  #endif
  byte BoardSerial[12];
  byte BoardNumber;
} TBoardInfo;

typedef struct
{
  word Voltage;
  word Current;
  dword Power;
  word Temperature;  
  TStageData BM;
  word Firmware;
  byte Serial[8];
  bool NeedsGUIUpdate;
  bool NeedsStatusUpdate;
  bool NeedsDataUpdate;
} TBatteryBoard,*PBatteryBoard;

typedef struct
{
  char code[10];
  char brand[30];
  char type[30];
  #ifdef STANDALONE  
  TStageData Stages[3]; // at max three : iddle, discharge and charge to keep it simple
  #endif
  TTestData TestData;
} TBatterySetting,*PBatterySetting;

typedef struct
{
  byte HIDEPOUTData[HID_INT_OUT_EP_SIZE];
  byte HIDEPINData[HID_INT_IN_EP_SIZE];
  bool DataReceived;
} THIDData;

#endif