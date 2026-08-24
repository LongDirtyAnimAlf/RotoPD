#define USE_LCD

#include <PacketSerial.h>
#include <Arduino.h>

#ifndef STANDALONE
#include "Adafruit_TinyUSB.h"
#endif

#include <Ticker.h>

#include "shared.h"
#include "comms.h"
#include "storage.h"
#include "extras.h"

#define BUZZER_PIN 19  //Buzzer GPIO
#define PACKET_UART_RXD 16
#define PACKET_UART_TXD 17

TBatteryBoard BatteryBoards[DAUGHTERBOARDCOUNT] = {0};

AP33772S pd(&WireBattery);
INA238 ina238(INA238_ADDRESS,&WireBattery);

char resetReasonText[][24] = { "Unknown", "Power On / Brownout", "Run pin", "Software", "Watchdog Timer", "Debug reset" };

COBSPacketSerial myPacketSerial;
//PacketSerial_<COBS, 0, 1024> myPacketSerial;

static Ticker datagetticker;
static volatile bool GetData = false;

#ifndef STANDALONE
// USB HID object
Adafruit_USBD_HID HID;

// Must be a global variable !!!
char mySerial[30];
char myFirmware[30];
#endif

void playTone(int tone, int duration)
{
    #if defined(__SAMD51__)
    for (long i = 0; i < duration * 1000L; i += tone * 2)
    {
        digitalWrite(BUZZER_PIN, HIGH);
        delayMicroseconds(tone);
        digitalWrite(BUZZER_PIN, LOW);
        delayMicroseconds(tone);
    }
    #endif
}

static void datagetcb()
{
  GetData = true;
}


void InitWire(void)
{
  Info_Add("RP2040. InitWire");

  pinMode(PIN_WIRE_BATT_SDA, INPUT_PULLUP);
  pinMode(PIN_WIRE_BATT_SCL, INPUT_PULLUP);

  WireBattery.setSDA(PIN_WIRE_BATT_SDA);
  WireBattery.setSCL(PIN_WIRE_BATT_SCL);
  WireBattery.setTimeout(50U, /*reset=*/false);     // sets the maximum number of milliseconds to wait and try to reset but in case of timeout !!
  WireBattery.clearTimeoutFlag();            
  WireBattery.setClock(100000UL);
  //WireBattery.setClock(400000UL);
  
  gpio_set_input_hysteresis_enabled(PIN_WIRE_BATT_SDA,true);
  gpio_set_slew_rate(PIN_WIRE_BATT_SDA,GPIO_SLEW_RATE_SLOW);
  gpio_set_drive_strength(PIN_WIRE_BATT_SDA,GPIO_DRIVE_STRENGTH_12MA);
  
  gpio_set_input_hysteresis_enabled(PIN_WIRE_BATT_SCL,true);
  gpio_set_slew_rate(PIN_WIRE_BATT_SCL,GPIO_SLEW_RATE_SLOW);
  gpio_set_drive_strength(PIN_WIRE_BATT_SCL,GPIO_DRIVE_STRENGTH_12MA);

  WireBattery.begin();
}

void sensor_power_on(void) {
  pinMode(18, OUTPUT);
  digitalWrite(18, HIGH);
}

void sensor_power_off(void) {
  pinMode(18, OUTPUT);
  digitalWrite(18, LOW);
}

void beep_off(void) {
  digitalWrite(19, LOW);
}

void beep_on(void) {
  analogWrite(BUZZER_PIN, 127);
  delay(50);
  analogWrite(BUZZER_PIN, 0);
}

// the setup function runs once when you press reset or power the board
void setup()
{
  byte i;
  word y1,tempintcalc;
  char myHex[10] = "";

  // This will remove the unwanted default string descriptor
  // And kill the unwanted (unneeded) serial port.
  #ifndef DEBUG
  Serial.end();
  #endif

  Serial1.setRX(PACKET_UART_TXD);
  Serial1.setTX(PACKET_UART_RXD);
  Serial1.begin(115200);
  myPacketSerial.setStream(&Serial1);

  for (i=0; i < DAUGHTERBOARDCOUNT;i++)
  {
    BatteryBoards[i].Voltage            = 0;
    BatteryBoards[i].Current            = 0;
    BatteryBoards[i].Power              = 0;
    BatteryBoards[i].Temperature        = 0;
    BatteryBoards[i].BM.Status          = smOff;
    BatteryBoards[i].pdoMode            = pmFixed;
    BatteryBoards[i].targetVoltage      = 0;
    BatteryBoards[i].maxCurrent         = 0;
  }

  #ifndef STANDALONE

  //Info_Add("RP2040. InitHID logic");

  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }
  
  USBDevice.setID(0x04D8,0x003F);
  USBDevice.setVersion(0x0002);
  USBDevice.setDeviceVersion(0x0002);

  USBDevice.setManufacturerDescriptor("Consulab for pleasure");
  USBDevice.setProductDescriptor("USB PD controller with HID");

  storeInit();
  storeGetBoardInfo(&BoardInfo);

  if (BoardInfo.InValid)
  {
    for (i=0; i<12;i++)
    {
      BoardInfo.BoardSerial[i] = DefaultBoardSerial[i];
    }
  }

  i = 0;
  mySerial[0] = '\0';  
  while (i<12)
  {
    tempintcalc=(BoardInfo.BoardSerial[i]+(BoardInfo.BoardSerial[i+1]*256));
    sprintf(myHex, "%04X", tempintcalc);    
    strcat(mySerial,myHex);    
    i += 2;
    if (i<12) strcat(mySerial,"-");
  }
  mySerial[29] = '\0';  

  USBDevice.setSerialDescriptor(mySerial);  
  //USBDevice.addStringDescriptor(mySerial);

  /*Init USB Device*/
  //HID.setStringDescriptor("HIDI2C BATT_CTRL");

  //HID.setReportCallback(get_report_callback, set_report_callback);
  HID.setReportCallback(NULL, set_report_callback);

  HID.enableOutEndpoint(true);
  HID.setPollInterval(1);
  HID.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));

  myFirmware[0] = '\0';  
  sprintf(myFirmware, "USB-PD-2026 V%02d-%02d", FW_MAJOR, FW_MINOR);
  myFirmware[16] = '\0';   
  //USBDevice.setSerialDescriptor(myFirmware);
  USBDevice.addStringDescriptor(myFirmware);  

  HID.begin();

  #endif // STANDALONE

  // Enable serial (again) for programming and debugging
  #ifdef DEBUG  
  Serial.begin(115200);
  int cnt = 5000;     // Will wait for up to ~1 second for Serial to connect.
  while (!Serial && cnt--) {delay(1);}
  // Serial.setDebugOutput(true);
  #endif

  Info_Add("RP2040. Indicator starting up.");

  InitWire();

  // Get the reset reason
  RP2040::resetReason_t rr = rp2040.getResetReason(); 
  Info_Add_Fmt("RP2040. Reset !!!!! Reset reason %i: %s.", rr, resetReasonText[rr]);

  // We might want to do someting with the WDT
  //rp2040.wdt_begin(1000u);

  Info_Add("RP2040. Sensors on.");
  sensor_power_on();

    // INA238 setup
  if (initINA238())
    Info_Add("RP2040. INA238 init success.");
  else
    Info_Add("RP2040. INA238 init failed !!");

  if (pd.isConnected())
    Info_Add("RP2040. RotoPD connected.");
  else
    Info_Add("RP2040. RotoPD not connected or not found.");

  Info_Add("RP2040. Datalogger ready for use !!");
  Info_Add("");

  datagetticker.attach_ms(DATAGETTIME, datagetcb);

  #ifdef STANDALONE  
  myPacketSerial.setPacketHandler(&onPacketReceived);
  #endif
}

void SendBatteryData(byte index)
{
  byte j,k;
  PBatteryBoard LocalBatteryBoard;

  uint8_t data_buf[32];
  WORD_VAL data;
  
  LocalBatteryBoard= &BatteryBoards[index];

  if ((LocalBatteryBoard->NeedsGUIUpdate) || (LocalBatteryBoard->NeedsDataUpdate) || (LocalBatteryBoard->NeedsStatusUpdate))
  {
    if (LocalBatteryBoard->NeedsGUIUpdate)
    {
      LocalBatteryBoard->NeedsGUIUpdate = false;

      j = 0;

      data_buf[j++] = CMD_get_data;
      data_buf[j++] = index;
      // Make space for length info
      j++;

      // Voltage
      data.Val = LocalBatteryBoard->Voltage;
      data_buf[j++] = data.bytes.LB;
      data_buf[j++] = data.bytes.HB;

      // Current            
      data.Val = LocalBatteryBoard->Current;
      data_buf[j++] = data.bytes.LB;
      data_buf[j++] = data.bytes.HB;

      // Power
      //data.Val = (word)(LocalBatteryBoard->Power % 0x10000UL);
      data.Val = (word)(LocalBatteryBoard->Power & 0xFFFF);
      data_buf[j++] = data.bytes.LB;
      data_buf[j++] = data.bytes.HB;

      data.Val = (word)(LocalBatteryBoard->Power >> 16);
      data_buf[j++] = data.bytes.LB;
      data_buf[j++] = data.bytes.HB;

      // Temperature
      data.Val = LocalBatteryBoard->Temperature;
      data_buf[j++] = data.bytes.LB;
      data_buf[j++] = data.bytes.HB;

      // Add length
      data_buf[2] = j;

      // Send the data
      myPacketSerial.send(data_buf, j);
    }

    if (LocalBatteryBoard->NeedsStatusUpdate)
    {
      LocalBatteryBoard->NeedsStatusUpdate = false;

      j = 0;

      data_buf[j++] = CMD_set_value;
      data_buf[j++] = index;
      // Make space for length info
      j++;


      // Send status
      data_buf[j++] = (byte)LocalBatteryBoard->BM.Status;

      // Send setvalue
      memcpy(&data_buf[j], &LocalBatteryBoard->BM.SetValue, 4);
      j += 4;

      /*
      data.Val = (word)(LocalBatteryBoard->BM.SetValue & 0xFFFF);
      data_buf[j++] = data.bytes.LB;
      data_buf[j++] = data.bytes.HB;

      data.Val = (word)(LocalBatteryBoard->BM.SetValue >> 16);
      data_buf[j++] = data.bytes.LB;
      data_buf[j++] = data.bytes.HB;
      */

      // Add length
      data_buf[2] = j;

      // Send the data
      myPacketSerial.send(data_buf, j);
    }

    /*
    if (LocalBatteryBoard->NeedsStatusUpdate)
    {
      LocalBatteryBoard->NeedsStatusUpdate = false;

      j = 0;

      data_buf[j++] = CMD_get_status;
      data_buf[j++] = index;
      // Make space for length info
      j++;

      // Send status
      data_buf[j++] = (byte)LocalBatteryBoard->BM.Status;

      // Send setvalue
      memcpy(&data_buf[j], &LocalBatteryBoard->BM.targetVoltage, 4);
      j += 4;

      // Send setvalue
      memcpy(&data_buf[j], &LocalBatteryBoard->BM.maxCurrent, 4);
      j += 4;

      // Send pdoMode
      data_buf[j++] = (byte)LocalBatteryBoard->BM.pdoMode;

      // Send pdoIndex
      data_buf[j++] = LocalBatteryBoard->BM.pdoIndex;

      // Add length
      data_buf[2] = j;
      
      // Send the data
      myPacketSerial.send(data_buf, j);
    }
    */
    
    if (LocalBatteryBoard->NeedsDataUpdate)
    {
      LocalBatteryBoard->NeedsDataUpdate = false;

      j = 0;

      data_buf[j++] = CMD_get_firmware;
      data_buf[j++] = index;
      // Make space for length info
      j++;

      // Firmware version
      data_buf[j++] = LocalBatteryBoard->Firmware;

      // Serial of daughterboard
      for (byte k=0; k<8; k++ )
      {
        data_buf[j++] = LocalBatteryBoard->Serial[k];
      }

      // Add length
      data_buf[2] = j;

      // Send the data
      myPacketSerial.send(data_buf, j);
    }
  }
}

void SendBatteryDataNew(const uint8_t* buffer, size_t size)
{
  myPacketSerial.send(buffer, size);
}


void loop()
{
  byte i,j;
  int8_t PDOCount = 0;

  bool DataOk = false;
  byte INData[COMMAND_SIZE] = {0};

  rp2040.wdt_reset();

  //pd.task();          // Keep-alive for PPS/AVS — essential!
  //printTelemetry();   // Print measurements when running

  static unsigned long startTime = millis();
  if (millis() - startTime >= 1000)
  {
    startTime = millis();

    PDOCount = taskRotoPDInit();

    if (PDOCount != -1)
    {
      // We have a newly connected RotoPD or new PDO's
      if (PDOCount>0)
      {
        Info_Add_Fmt("RP2040. Process new PDO's !! PDO count: %d.",PDOCount);

        memset(&INData, 0, COMMAND_SIZE);

        // Request / read list of PDOs
        //byte INData[HID_INT_IN_EP_SIZE] = {0};        

        byte OUTData[HID_INT_OUT_EP_SIZE] = {0};
        OUTData[0] = CMD_get_PDOList;
        if (process_command(&OUTData,&INData))
        {
          // We might send some PDO data back towards the SenseCap LCD/ESP32
          if (INData[0] == CMD_get_PDOList)
          {
            Serial.println("PDO list below.");
            pd.printPDOs(Serial);
            Serial.println("Done.");
            SendBatteryDataNew(INData, INData[2]+DATASTART);
          }
        }
      }
    }
  }  

  #ifndef STANDALONE
  #ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
  #endif
  #endif

  if (GetData)
  {
    GetData = false;
    collectRotoPDData();
  }


  THIDData* PLocalHD;
  THIDData LocalHDCopy;

  #ifndef STANDALONE
  for (i=0; i<DAUGHTERBOARDCOUNT; i++ )
  {
    if (HIDData[i].DataReceived)
    {
      #ifndef TINYUSB_NEED_POLLING_TASK
      // Make a local copy of the data 
      // This is needed due to the fact that the USB HID interrupt may update the data when in this loop 
      noInterrupts();
      for ( j=0; j<HID_INT_OUT_EP_SIZE; j++ ) LocalHDCopy.HIDEPOUTData[j] = HIDData[i].HIDEPOUTData[j];
      LocalHDCopy.DataReceived = HIDData[i].DataReceived;
      HIDData[i].DataReceived = false;
      interrupts();
      PLocalHD = &LocalHDCopy; 
      #else
      PLocalHD = (THIDData*)&HIDData[i];
      PLocalHD->DataReceived = false;
      #endif

      // Rough I2C traffic indicator
      #ifndef ARDUINO_SEEED_INDICATOR_RP2040
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      #endif

      memset(PLocalHD->HIDEPINData, 0, sizeof(PLocalHD->HIDEPINData));

      //Now perform the Data update  
      DataOk = process_command(&PLocalHD->HIDEPOUTData,&PLocalHD->HIDEPINData);
      if (DataOk)
      {
        // Send report back to host
        HID.sendReport(0, &PLocalHD->HIDEPINData, HID_INT_IN_EP_SIZE);

        //for (j=0; j<HID_INT_IN_EP_SIZE; j++) INData[j] = PLocalHD->HIDEPINData[j];

        #ifdef USE_LCD
        SendBatteryData(0);
        #endif
      }
    }
  }
  #endif // !STANDALONE

  myPacketSerial.update();
  if (myPacketSerial.overflow())
  {
  }

}

void onPacketReceived(const uint8_t *buffer, size_t size)
{
  // This is data we receive from the SenseCap LCD/ESP32 itself !!

  byte OUTData[HID_INT_OUT_EP_SIZE] = {0};
  byte INData[HID_INT_IN_EP_SIZE] = {0};

  if (size < 1) {
    return;
  }

  if (size <= HID_INT_OUT_EP_SIZE)
  {
    for (byte i=0; i<size; i++ ) OUTData[i]=buffer[i];

    // The SenseCap LCD does not know the boardnumber
    // So set in here explicit
    OUTData[INDEXPOSITION] = BoardInfo.BoardNumber;

    if (process_command(&OUTData,&INData))
    {
      // We might send some data back towards the SenseCap LCD/ESP32
      CommandType_t cCmd=(CommandType_t)INData[0];

      if ( (cCmd == CMD_get_PDOList) || (cCmd == CMD_read_PDOList) || (cCmd == CMD_set_MAXPDO))
      {
        // Bit tricky
        // Send back the PDO[s] !!
        SendBatteryDataNew(INData, INData[2]+DATASTART);
      }
      #ifdef USE_LCD
      SendBatteryData(0);
      #endif
    }
    delayMicroseconds(1000U);
  }
}
