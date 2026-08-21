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
  #ifdef DEBUG
  Serial.println("InitWire");
  #endif  

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

// ─────────────────────────────────────────────────────────────────────────────
//  Telemetry — only printed in RUNNING state
// ─────────────────────────────────────────────────────────────────────────────
void printTelemetry()
{
    static uint32_t last = 0;
    if (millis() - last < 1000) return;
    last = millis();

    Serial.printf("AP33772S data. T: %d°C. VREQ: %5umV. IREQ: %5umA.",
                  pd.getTemperature_C(),
                  pd.getRequestedVoltage_mV(),
                  pd.getRequestedCurrent_mA());

    if (pd.isDerating()) Serial.print(F("  [DR]"));
    if (pd.isFault())    Serial.printf("  [%s]", pd.getFaultString().c_str());

    Serial.println();
    Serial.print("INA238 data. ");

    Serial.print("I: ");
    Serial.print(ina238.getMilliAmpere(), 0);
    Serial.print("mA. ");

    Serial.print("V: ");
    Serial.print(ina238.getBusMilliVolt(), 0);
    Serial.print("mV. ");

    Serial.print("P: ");
    Serial.print(ina238.getMilliWatt(), 0);
    Serial.print("mW. ");

    Serial.print("T: ");
    Serial.print(ina238.getTemperature(), 3);
    Serial.println("°C.");
    Serial.println();


    //Serial.println(F("\n[INFO] Register dump:"));
    //pd.dumpRegisters(Serial); 
    Serial.println();
}

bool InitROTOPD(void)
{
  // RotoPD Pro setup

  // Required for RotoPD Pro. Prevent UVP from issuing hard reset.
  // VOUT connected to +5V
  pd.clearConfig(CONFIG_UVP_EN);

  if (pd.begin() != AP33772S_OK)
  {
    delay(500);
    if (pd.begin() != AP33772S_OK)
    {
      Serial.println(F("[INIT] AP33772S failed !"));
      pd.dumpRegisters(Serial);
      return (false);
    }
  }
  // Protection thresholds
  // Temperature only for RotoPD Pro
  // VOUT ISENSP AND VCC are connected to +5V
  //pd.setOVPOffset_mV(2000);
  //pd.setUVPThreshold(UVP_80PCT);
  //pd.setOCPThreshold_mA(0);      // auto = 110% of PDO
  pd.setOTPThreshold_C(85);
  pd.setConfig(CONFIG_OTP_EN);
  pd.setDeratingThreshold_C(75);
  pd.setConfig(CONFIG_DR_EN);

  // Interrupts
  //pd.setInterruptMask(MASK_ALL);
  //pd.attachInterruptCallback(pdISR);

  return (true);
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
  
  for (i=0; i < DAUGHTERBOARDCOUNT;i++)
  {
    BatteryBoards[i].Voltage            = 0;
    BatteryBoards[i].Current            = 0;
    BatteryBoards[i].Power              = 0;
    BatteryBoards[i].Temperature        = 0;
    BatteryBoards[i].BM.Status          = smOff;
    BatteryBoards[i].pdoMode         = pmFixed;
    BatteryBoards[i].targetVoltage   = 0;
    BatteryBoards[i].maxCurrent      = 0;
  }

  #ifndef STANDALONE

  //Serial.println("InitHID logic");

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
  Serial.println("RP2040 Indicator starting up.");
  #endif

  InitWire();

  // INA238 setup
  if(!ina238.begin())
  {
    Serial.println("Cannot find INA238 on RotoPD Pro.");
  }
  else
  {
    ina238.reset();
    ina238.setADCRange(1);
    if ((BoardInfo.shuntcorrection<1000) && (BoardInfo.shuntcorrection>-1000))
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
    ina238.setDiagnoseAlertBit(INA238_DIAG_ALERT_LATCH); //Set to Alert latch
  }


  // Get the reset reason
  RP2040::resetReason_t rr = rp2040.getResetReason(); 
  Serial.printf("RP2040 reset !!!!! Reset Reason %i: %s\r\n", rr, resetReasonText[rr]);

  // We might want to do someting with the WDT
  //rp2040.wdt_begin(1000u);

  Serial1.setRX(PACKET_UART_TXD);
  Serial1.setTX(PACKET_UART_RXD);
  Serial1.begin(115200);
  myPacketSerial.setStream(&Serial1);
  #ifdef STANDALONE  
  myPacketSerial.setPacketHandler(&onPacketReceived);
  #endif
  sensor_power_on();

  datagetticker.attach_ms(DATAGETTIME, datagetcb);

  Serial.println("Datalogger ready for use !!");
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
  byte PDOCount = 0;

  bool DataOk = false;
  byte INData[COMMAND_SIZE] = {0};

  rp2040.wdt_reset();

  //pd.task();          // Keep-alive for PPS/AVS — essential!
  //printTelemetry();   // Print measurements when running

  static unsigned long startTime = millis();
  if (millis() - startTime >= 1000)
  {
    startTime = millis();

    if (pd.isConnected())
    {
      j = pd.getStatus();
      if (j & STATUS_STARTED)
      {
        Serial.printf("Status:= 0x%02X (%d).\r\n", (uint8_t)(j<0?0xFF:j), (uint8_t)(j<0?0:j));

        PDOCount = pd.getValidPDOCount();
        #ifdef DEBUG
        Serial.printf("Initial PDO count: %d\r\n",PDOCount);
        #endif

        // We have a power up !!
        // Init the AP33772S / RotoPD
        if (InitROTOPD())
        {

          // Check if we already have the new PDO's
          if (((j & STATUS_NEWPDO)  && (j & STATUS_READY)) || (PDOCount>0) || (pd.waitForPDOs(2000)==AP33772S_OK))
          {
            // Request / read list of PDOs
            if (PDOCount == 0) PDOCount = pd.readAllPDOs();

            // Request / read list of PDOs
            byte INData[HID_INT_IN_EP_SIZE] = {0};        
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
                SendBatteryDataNew(INData, INData[2]);
              }
            }
          }
        }
        else
        {
          Serial.println(F("[INIT] AP33772S error !"));
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
    if (process_command(&OUTData,&INData))
    {
      // We might send some data back towards the SenseCap LCD/ESP32
      CommandType_t cCmd=(CommandType_t)INData[0];
      if ( (cCmd == CMD_get_PDOList) || (cCmd == CMD_read_PDOList) || (cCmd == CMD_set_MAXPDO))
      {
        // Bit tricky
        // Send back the PDO[s] !!
        SendBatteryDataNew(INData, INData[2]);
      }
      #ifdef USE_LCD
      SendBatteryData(0);
      #endif
    }
    delayMicroseconds(1000U);
  }
}
