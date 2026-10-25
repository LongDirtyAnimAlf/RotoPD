#ifndef _SHARED_H_
#define _SHARED_H_

//#pragma once          // or classic include guards

#include <Arduino.h>
#include "queue.h"

#include "./src/AP33772SS/AP33772S.h"
#include "./src/INA238/INA238.h"
#include "./src/IT2704/IT2704.h"

#define    DEBUG

#define    ENABLEUSB

#ifndef ARDUINO_ARCH_SAMD
// Only SenseCap can be standalone !!
#define    STANDALONE
#endif

#define ROTOPDDATAGETTIME             50 // ms
#define IT2704DATAREQUESTTIME         250 // ms
#define DATACOLLECTTIMEFAST           500 // ms
#define DATACOLLECTTIMENORMAL         (1000u * 10u) // 10 seconds in ms
#define CALCULATIONTIME               100 // ms

#define SAVERTIME                     (1000u * 60u * 1u) // 10 minutes in ms
#define DVTIME                        (1000u * 60u * 10u) // 10 minutes in ms
#define PVTIME                        (1000u * 60u * 10u) // 10 minutes in ms

// RotoPD
#define WITHINA238TEMPERATURE
#define REV11
#ifdef REV11
#define XRS40N10ONRESISTANCE          (2*20) //mOhm 
#define SHUNTRESISTANCE               (5) //mOhm 
#endif

#define    DAUGHTERBOARDCOUNT         1

#define    FW_MAJOR                   56
#define    FW_MINOR                   14

#define    DATASIZE                   10000u

#define INA238_ADDRESS                0x40

#define DEFAULT_IT2704_CHANNEL                0
#define DEFAULT_BOARDNUMBER                   (DEFAULT_IT2704_CHANNEL + 1)
#define DEFAULT_IT2704_NODEID                 1
#define DEFAULT_MASTER_NODEID                 10
#define DEFAULT_LOCAL_NODEID                  20

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
#define BOARDPOSITION     (1)
#define LENGTHPOSITION    (2)
#define DATASTART         (3)

#define COMMAND_SIZE      (64)

#define    USB_CMD_get_serial         0x64
#define    USB_CMD_set_serial         0x65
#define    USB_CMD_get_firmware       0xC8
#define    USB_CMD_get_board_number   0xCA
#define    USB_CMD_set_board_number   0xCB

#define    USB_CMD_error              0xFF

typedef struct
{
  byte HIDEPOUTData[HID_INT_OUT_EP_SIZE];
  byte HIDEPINData[HID_INT_IN_EP_SIZE];
  bool DataReceived;
} THIDData;

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
} DWORD_VAL, DWORD_BITS;

typedef union
{
    qword Val;
    byte v[8];// __PACKED;
    word w[4];// __PACKED;
    dword dw[2];// __PACKED;
} QWORD_VAL, QWORD_BITS;

#define IDDLESTAGENUMBER              0
#define FIXEDDISCHARGESTAGENUMBER     1
#define FIXEDCHARGESTAGENUMBER        2

typedef enum {
    CMD_unknown          = 0x00,
    CMD_logdata          = 0x30,
    CMD_error            = 0x40,
    CMD_get_data         = 0x50,
    CMD_get_status,
    CMD_get_hardware,
    CMD_get_firmware,
    CMD_set_value,
    CMD_set_energy,
    CMD_set_capacity,
    CMD_set_time,
    CMD_get_PDOList,
    CMD_read_PDOList,
    CMD_set_FIXEDPDO,
    CMD_set_PPSPDO,
    CMD_set_AVSPDO,
    CMD_set_MAXPDO,
    CMD_set_output,
    CMD_get_curcal,
    CMD_set_curcal,
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

typedef struct
{
  TStageMode Status;
  dword SetValue;
  TThresholdDataItem ThresholdSettings[tmLast]; // all threshold possibilities
} TStageData, *PStageData;

typedef struct
{
  word  V;   // voltage
  word  I;   // current
  dword P;   // power
  word  T;   // temperature
  dword C;   // Ah energy
  dword E;   // Wh energy
  dword S;   // timestamp ms
} TMeasurementData,*PMeasurementData;

typedef struct
{
  // Actual board data
  TMeasurementData RotoPDData;
  TMeasurementData IT2704Data;
  TStageData BM;
  // PDO stuff
  AP33772S_PDO PDO;
  //TPDOMode pdoMode;
  //int pdoIndex;
  //int pdoTargetVoltage;
  //int pdoMaxCurrent;
  // Board hardware info
  byte Serial[8];
  word Firmware;
  // Miscellaneous
  bool NeedsGUIUpdate;
  bool NeedsStatusUpdate;
  bool NeedsDataUpdate;
} TBatteryBoard,*PBatteryBoard;

static void ClearMeasurementData(PMeasurementData MAD)
{
  MAD->V = 0;
  MAD->I = 0;  
  MAD->P = 0;
  MAD->T = 0;
  MAD->C = 0;
  MAD->E = 0;
  MAD->S = 0;
}

typedef struct
{
  #ifdef __cplusplus          // only compile the class when using a C++ compiler
  CircularDataQueue<TMeasurementData, DATASIZE> NewerRotoPDDatas;
  CircularDataQueue<TMeasurementData, DATASIZE> NewerIT2704Datas;
  #endif
  // Cumulative datas
  qword Capacity; // in nAh
  qword Energy; // in nWh
  qword Time;  // in ms
  byte CurrentStageNumber;
  TThresholdDataItem ThresholdResult[tmLast]; // all threshold possibilities
} TRunDatas,*PRunDatas;

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

extern TwoWire WireBattery;
extern AP33772S pd;
extern IT2704 load;
extern CanOpenNode node;

extern TBatteryBoard BatteryBoards[];
extern PBatteryBoard const CurrentBoard;

#ifdef __cplusplus          // only compile the class when using a C++ compiler

class TBoardInfo {
private:
  // Board-specific validity flag (hidden)
  #ifdef ARDUINO_SEEED_INDICATOR_RP2040
  bool InValid;
  #endif

  #if defined(ARDUINO_ARCH_SAMD)
  bool Valid;
  #endif

  #ifdef ARDUINO_ARCH_ESP32          // note: matches your latest list
  bool Valid;
  #endif

  #ifdef ARDUINO_ARCH_RP2040
  bool InValid;
  #endif

public:
  // Common board information
  byte BoardSerial[12];
  byte BoardCalDate[4];   // YYMD
  byte LocalNodeID;
  byte MasterNodeID;
  byte IT2704NodeID;
  byte IT2704Channel;
  int16_t shuntcorrection;
  bool UIDarkEnabled;
  bool BeepEnabled;

  // ---------- Default constructor (uses the values from your list) ----------
  TBoardInfo() {
    // Arrays
    memset(BoardSerial, 0, sizeof(BoardSerial));
    memset(BoardCalDate, 0, sizeof(BoardCalDate));

    // Other members
    LocalNodeID     = DEFAULT_LOCAL_NODEID;
    MasterNodeID    = DEFAULT_MASTER_NODEID;
    IT2704NodeID    = DEFAULT_IT2704_NODEID;
    IT2704Channel   = DEFAULT_IT2704_CHANNEL;
    shuntcorrection = 44;           // Default INA238 shunt correction value (in uOhm, signed, int16_t)
    UIDarkEnabled   = true;         // Dark
    BeepEnabled     = true;         // Beep

    // Use the helper instead of setting the flag directly
    setValid(false);
  }

  // ---------- Validity helpers ----------
  bool isValid() const {
    #ifdef ARDUINO_SEEED_INDICATOR_RP2040
    return !InValid;
    #elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP32)
    return Valid;
    #elif defined(ARDUINO_ARCH_RP2040)
    return !InValid;
    #else
    return true;
    #endif
  }

  void setValid(bool value) {
    #ifdef ARDUINO_SEEED_INDICATOR_RP2040
    InValid = !value;
    #elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP32)
    Valid = value;
    #elif defined(ARDUINO_ARCH_RP2040)
    InValid = !value;
    #endif
  }

  // Always returns LocalNodeID + 1
  byte getBoardNumber() const {
    return (IT2704Channel + 1);
  }
};

#endif // __cplusplus

#endif /*_SHARED_H_*/