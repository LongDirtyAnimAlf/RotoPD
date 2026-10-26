#include <Arduino.h>

//#include <mutex>
//#include <chrono>
//#include <functional>

#include <lvgl.h>
#include <Ticker.h>

#ifdef ARDUINO_ARCH_ESP32

#include <Arduino_GFX_Library.h>
#include "USB.h"
#include "USBHID.h"
#include "esp32-hal-tinyusb.h"
#include "customhid.h"

#define PACKET_UART_RXD 20
#define PACKET_UART_TXD 19

#define CAN_TX GPIO_NUM_6 // Transmit GPIO number for CAN
#define CAN_RX GPIO_NUM_0 // Receive GPIO number for CAN

//#define BUTTON_PIN 38

#define GFX_DEV_DEVICE ESP32_S3_RGB
#define RGB_PANEL
//#define GFX_BL 45

#endif

#ifdef ARDUINO_ARCH_RP2040

#include "hardware/vreg.h"
#include "hardware/powman.h"
#include "hardware/structs/qmi.h"
#include "hardware/sync.h"
#include "hardware/clocks.h"

#include <pico/stdlib.h>
#include <pico/multicore.h>

#include "./src/RP2350/bsp/bsp_i2c.h"
#include "./src/RP2350/bsp/bsp_st7701.h"
#include "./src/RP2350/bsp/bsp_buzzer.h"

bool core1_separate_stack = true;

#endif

//#include "./src/CAN/mcp2515.h"
#include "./src/CANOpen/canOpenNode.h"
#include "./src/CANOpen/sdo.hpp"
#include "./src/CANOpen/message.hpp"

#include "ui.h"

//#include <WiFi.h>

#include "extras.h"
#include "shared.h"
#include "comms.h"

#define HOR_RES 480
#define VER_RES 480

// Default placeholder due to re-use of existing software
#define ActiveBatteryIndex 0

// USB HID object
#ifdef ARDUINO_ARCH_RP2040
#ifdef ENABLEUSB
#ifdef USE_TINYUSB
Adafruit_USBD_HID HID;
#endif
#endif
#endif

#ifdef ARDUINO_ARCH_ESP32
#if defined(ARDUINO_USB_MODE) && (ARDUINO_USB_MODE == 0)  // USB-OTG / TinyUSB
#ifdef ENABLEUSB
USBHID HID;
#endif
USBCDC USBSerial;
#endif
#endif

unsigned long loopStartTime = 0;

// Must be a global variable !!!
char mySerial[30];
char myFirmware[30];

AP33772S pd((TwoWire*)&WireBattery);

//CanOpenNode node(BSP_XL2515_CS_PIN, BSP_XL2515_INT_PIN, CAN_250KBPS, /*NodeID=*/ DEFAULT_LOCAL_NODEID);
CanOpenNode node(-1, -1, CAN_250KBPS, /*NodeID=*/ DEFAULT_LOCAL_NODEID);
IT2704 load(&node, DEFAULT_IT2704_NODEID, DEFAULT_IT2704_CHANNEL);

#ifdef ARDUINO_ARCH_ESP32

Arduino_DataBus *bus = new Arduino_SWSPI(
    GFX_NOT_DEFINED /* DC */, 42 /* CS */,
    2 /* SCK */, 1 /* MOSI */, GFX_NOT_DEFINED /* MISO */);

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    40 /* DE */, 39 /* VSYNC */, 38 /* HSYNC */, 41 /* PCLK */,
    46 /* R0 */, 3 /* R1 */, 8 /* R2 */, 18 /* R3 */, 17 /* R4 */,
    14 /* G0 */, 13 /* G1 */, 12 /* G2 */, 11 /* G3 */, 10 /* G4 */, 9 /* G5 */,
    5 /* B0 */, 45 /* B1 */, 48 /* B2 */, 47 /* B3 */, 21 /* B4 */,
    1 /* hsync_polarity */, 10 /* hsync_front_porch */, 8 /* hsync_pulse_width */, 50 /* hsync_back_porch */,
    1 /* vsync_polarity */, 10 /* vsync_front_porch */, 8 /* vsync_pulse_width */, 20 /* vsync_back_porch */
    #ifndef ALTERNATIVE
    ,0 /* pclk_active_neg */, 20000000 /* prefer_speed */, false /* useBigEndian  */,
    0 /* de_idle_high */, 0 /* pclk_idle_high */, HOR_RES * 20 /* bounce_buffer_size_px */);    
    #else
    );
    #endif

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    #ifndef ALTERNATIVE
    HOR_RES /* width */, VER_RES /* height */, rgbpanel, 0 /* rotation */, false /* auto_flush */,
    bus, GFX_NOT_DEFINED /* RST */, st7701_indicator_init_operations, sizeof(st7701_indicator_init_operations));
    #else
    HOR_RES /* width */, VER_RES /* height */, rgbpanel, 2 /* rotation */, true /* auto_flush */,
    bus, GFX_NOT_DEFINED /* RST */, st7701_type1_init_operations, sizeof(st7701_type1_init_operations));
    #endif
#endif

TBatteryBoard BatteryBoards[DAUGHTERBOARDCOUNT] = {0};
PBatteryBoard const CurrentBoard = BatteryBoards;

static TBatterySetting Batteries[DAUGHTERBOARDCOUNT]; // Battery data settings and results
static PBatterySetting const Battery = Batteries;

static Ticker tickerDataUpdate;
static volatile bool CalcBatteryData = false;

static Ticker tickerCollectRotoPDData;
static volatile bool flagCollectRotoPDData = false;

static Ticker tickerRequestIT2704Data;
static volatile bool flagRequestIT2704Data = false;

static Ticker SoundButtonTicker;
static bool flagSoundButton = false;

static Ticker tickerProcessData;
static volatile bool flagProcessMeasurementData = false;

// Flag flash dirty for storing board info
static volatile bool StoreBoardInfo = false;
static volatile byte pending_screen_index = 0;

//GT911_Lite tp; // touchscreen through TwoWire

#ifdef STANDALONE
// Used for delayed start
static Ticker datastartticker;
// Flag flash dirty for storing settings
static volatile bool StoreSettings = false;
// Used by touch-screen to send data
// Simulates commands coming from USB HID
static byte SendCommand[COMMAND_SIZE] = {0};
#endif

mutex_t can_mutex;

void ClearRunData(PRunDatas RDS);
void ClearStageData(PStageData SD);

// ---------------------------------------------------------------
// Automatic PSRAM (QMI M1) timing calculator for RP2350
// Tuned for common APS6404 / similar 133–166 MHz PSRAM chips
// ---------------------------------------------------------------
#ifdef ARDUINO_ARCH_RP2040
void set_psram_timing_auto()
{
    // Maximum safe SCK frequency for most common PSRAM chips (Hz)
    const uint32_t MAX_PSRAM_SCK_HZ = 133000000;   // conservative; some chips do 144–166 MHz

    // Typical PSRAM timing requirements (from datasheets)
    const uint32_t MAX_SELECT_NS   = 8000;   // max CS low time ≈ 8 µs
    const uint32_t MIN_DESELECT_NS = 50;     // min CS high time ≈ 50 ns

    uint32_t sys_hz = clock_get_hz(clk_sys);

    // 1. Clock divider so that SCK ≤ MAX_PSRAM_SCK_HZ
    uint32_t clkdiv = (sys_hz + MAX_PSRAM_SCK_HZ - 1) / MAX_PSRAM_SCK_HZ;
    if (clkdiv < 1) clkdiv = 1;
    if (clkdiv > 255) clkdiv = 255;          // hardware limit

    // 2. RXDELAY – needs to increase at higher SCK frequencies
    //    Rule of thumb used by many successful ports:
    uint32_t rxdelay = 1;
    if (sys_hz / clkdiv > 100000000) rxdelay = 2;
    if (sys_hz / clkdiv > 130000000) rxdelay = 3;
    if (rxdelay > 7) rxdelay = 7;

    // 3. MAX_SELECT (in units of 64 system clocks)
    //    Keep CS assertion under ~8 µs
    uint32_t cycles_per_us = sys_hz / 1000000;
    uint32_t max_select = (MAX_SELECT_NS * cycles_per_us) / (64 * 1000);
    if (max_select > 63) max_select = 63;    // 6-bit field
    if (max_select < 1)  max_select = 1;

    // 4. MIN_DESELECT (extra system clocks after CS deassert)
    uint32_t min_deselect = (MIN_DESELECT_NS * cycles_per_us + 999) / 1000;
    // subtract the inherent half-SCK already provided by the hardware
    if (min_deselect > (clkdiv + 1) / 2)
        min_deselect -= (clkdiv + 1) / 2;
    else
        min_deselect = 0;
    if (min_deselect > 31) min_deselect = 31; // 5-bit field

    // 5. Other fixed / recommended values
    const uint32_t cooldown    = 1;   // short cooldown
    const uint32_t pagebreak   = 2;   // 1024-byte page break (value 2)
    const uint32_t select_hold = 1;   // 1 extra hold cycle (good default)
    const uint32_t select_setup = 0;

    // Build the register value
    uint32_t timing =
        (cooldown      << 30) |
        (pagebreak     << 28) |
        (select_setup  << 25) |
        (select_hold   << 23) |
        (max_select    << 17) |
        (min_deselect  << 12) |
        (rxdelay       <<  8) |
        (clkdiv        <<  0);

    // Write it safely
    uint32_t irq = save_and_disable_interrupts();
    qmi_hw->m[1].timing = timing;
    restore_interrupts(irq);

    // Optional debug print
    // Serial.printf("PSRAM timing: 0x%08X  (clkdiv=%u rxdelay=%u max_sel=%u min_desel=%u)\n",
    //               timing, clkdiv, rxdelay, max_select, min_deselect);
}
#endif

// Callback that returns elapsed ms since boot
static uint32_t my_tick_get_cb(void) {
  return millis();
}

#ifdef STANDALONE
static void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
//static void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  if (touch_has_signal())
  {
    if (touch_touched())
    {
      data->state = LV_INDEV_STATE_PRESSED;

      /*Set the coordinates*/
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;
    }
    else if (touch_released())
    {
      data->state = LV_INDEV_STATE_RELEASED;
    }
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}
/*
static void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  bool changed = tp.read();  

  if (changed)
  {
    data->point.x = tp.last_x;
    data->point.y = tp.last_y;
  }

  //if (tp.isTouched)
  if (tp.down)
  {
    data->state = LV_INDEV_STATE_PRESSED;
    //Set the coordinates
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}
*/
#endif

#ifdef STANDALONE

#ifdef ARDUINO_ARCH_RP2040
static void __not_in_flash_func(storeSettings)(byte BI)
{
    PBatterySetting SET = &Batteries[BI];
    storePutBatteryDischargeSetting(BI, &SET->Stages[FIXEDDISCHARGESTAGENUMBER]);
    storePutBatteryChargeSetting(BI, &SET->Stages[FIXEDCHARGESTAGENUMBER]);
}
#endif

#ifdef ARDUINO_ARCH_ESP32
static void IRAM_ATTR storeSettings(byte BI)
{
  //PBatterySetting SET = &Batteries[BI];
  //storePutBatteryDischargeSetting(BI, &SET->Stages[FIXEDDISCHARGESTAGENUMBER]);
  //storePutBatteryChargeSetting(BI, &SET->Stages[FIXEDCHARGESTAGENUMBER]);
}
#endif

#endif

static void main_event_handler(lv_event_t * e)
{
  static byte screenindex = 1;

  // Re-entrancy Guard
  static bool in_handler = false;
  if (in_handler) return;
  in_handler = true;

  bool GotSettings = false;

  PBatterySetting SET = Battery;
  PRunDatas RDS = &SET->TestData.RunDatas;
  PStageData SD = NULL;  

  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * event_user_data = (lv_obj_t *)lv_event_get_user_data(e);
  lv_obj_t * event_object = lv_event_get_target_obj(e);
  if (event_object == NULL) event_object = lv_event_get_current_target_obj(e);

  lv_obj_t * object_user_data = NULL;

  if (event_object != NULL)
  {
    object_user_data = (lv_obj_t *)lv_obj_get_user_data(event_object);

    #ifdef STANDALONE
    if ( (lv_obj_check_type(event_object, &lv_button_class)) || (lv_obj_check_type(event_object, &lv_list_button_class)) )
    {
      if(code == LV_EVENT_VALUE_CHANGED)
      {
        flagSoundButton = true;

        //bool buttondown = (lv_obj_get_state(btn, LV_BTN_PART_MAIN) & LV_STATE_CHECKED);
        bool buttondown = (lv_obj_get_state(event_object) & LV_STATE_CHECKED);

        if (event_object == outputbutton)
        {
          // Prepare the command to engage the hardware
          memset(SendCommand, 0, COMMAND_SIZE); 
          SendCommand[COMMANDPOSITION] = CMD_set_output;
          SendCommand[BOARDPOSITION] = BoardInfo.getBoardNumber();
          SendCommand[LENGTHPOSITION] = 1U; // length
          SendCommand[DATASTART] = (uint8_t)buttondown;
        }    
        else
        if ( (event_object == testdischargebutton) || (event_object == startdischargebutton) || (event_object == testchargebutton) || (event_object == startchargebutton) )
        {
          dword temp = 0;
          byte i = 0;

          #ifdef DEBUG                      
          Serial.println("Engage buttons");
          #endif

          // We will always start with being idle
          SET->TestData.Active = bmIdle;

          // Reset all trigger settings
          SET->TestData.ThresholdMode = tmNONE;
          SET->TestData.ThresholdValue = 0;
          for(i = (tmNONE+1); i < tmLast; i++)
          {
            Screen1SetThresholdLedEnabled((TThresholdModes)i, false);
          }

          if (!buttondown)
          {
            SET->TestData.SetStageMode = smOff;
            SET->TestData.SetStageValue = 0;
          }
          else
          {
            if ((event_object == testdischargebutton) || (event_object == startdischargebutton))
            {
              SD = &SET->Stages[FIXEDDISCHARGESTAGENUMBER];
            }
            if ((event_object == testchargebutton) || (event_object == startchargebutton))
            {
              SD = &SET->Stages[FIXEDCHARGESTAGENUMBER];
            }

            SET->TestData.SetStageValue = SD->SetValue;  
            SET->TestData.SetStageMode = SD->Status;
            
            if ((event_object == startdischargebutton) || (event_object == startchargebutton))          
            {
              // Set trigger indicators
              for(i = (tmNONE+1); i < tmLast; i++)
              {
                Screen1SetThresholdLedEnabled((TThresholdModes)i, SD->ThresholdSettings[i].Enabled);
              }
              // We are active !!
              SET->TestData.Active = bmActive;

              // Force a very early data measurement to get a start value
              flagProcessMeasurementData = true;

              // Engage after some time !!!!
              // This is needed to dismiss the first few measurements when starting
              //if (SET->TestData.SetStageMode == smCharge) datastartticker.once_ms(1000U, datastartcb, (byte)ActiveBatteryIndex);                  
              //if (SET->TestData.SetStageMode == smCurrent) datastartticker.once_ms(1000U, datastartcb, (byte)ActiveBatteryIndex);                  
            }
          }

          // Prepare the command to engage the hardware
          memset(SendCommand, 0, COMMAND_SIZE);           
          SendCommand[COMMANDPOSITION] = CMD_set_value;
          SendCommand[BOARDPOSITION] = BoardInfo.getBoardNumber();
          SendCommand[LENGTHPOSITION] = 5U; // length
          SendCommand[DATASTART] = (byte)SET->TestData.SetStageMode;
          DWORD_VAL valuedata;
          valuedata.Val = SET->TestData.SetStageValue;
          SendCommand[DATASTART+1] = valuedata.v[0];
          SendCommand[DATASTART+2] = valuedata.v[1];
          SendCommand[DATASTART+3] = valuedata.v[2];
          SendCommand[DATASTART+4] = valuedata.v[3];
        }
        else
        {
          #ifdef DEBUG                      
          Serial.println("Event unhandled: button value changed");
          #endif
       }
      }
      else
      if(code == LV_EVENT_LONG_PRESSED)
      {
        #ifdef DEBUG                      
        Serial.println("Event unhandled: long pressed");
        #endif
      }
      else
      if(code == LV_EVENT_CLICKED)
      {
        flagSoundButton = true;

        if (event_object == sw_savesettings)
        {
          BoardInfo.UIDarkEnabled = GetDarkTheme();
          BoardInfo.BeepEnabled = GetButtonBeep();
          BoardInfo.LocalNodeID = GetNodeAdress();
          BoardInfo.MasterNodeID = GetMasterAdress();
          BoardInfo.IT2704NodeID = GetIT2704Adress();
          BoardInfo.IT2704Channel = GetIT2704Channel();
          StoreBoardInfo = true;
        }

        // Screen navigation
        if ((event_object == backbutton) || (event_object == morebutton))
        {
          if ( (event_object == backbutton) && (screenindex>1) ) screenindex--; // back button
          if ( (event_object == morebutton) && (screenindex<6) ) screenindex++; // forwards button              
          pending_screen_index = screenindex; // Defer screen change
        }
        else
        // Zero buttons
        if ((event_object == zerocapacitybutton) || (event_object == zeroenergybutton) || (event_object == zerotimebutton))
        {
          #ifdef DEBUG                      
          Serial.println("Zero button pressed");
          #endif
          if ((event_object == zeroenergybutton) || (event_object == zerocapacitybutton))
          {
            if (event_object == zerocapacitybutton) RDS->Capacity = 0;
            if (event_object == zeroenergybutton) RDS->Energy = 0;
            // Not the best place to do this !!
            if (mutex_enter_timeout_ms(&can_mutex, 100))
            {
              load.clearEnergy();
              mutex_exit(&can_mutex);  
            }
            else
            {
              #ifdef DEBUG                      
              Serial.println("GUI. CAN mutex timeout.");
              #endif
            }
          }
          if (event_object == zerotimebutton) RDS->Time = 0;          
          Screen1AddEPData(RDS->Energy,0);        
          ScreenPowerAddEPData(RDS->Energy,0);        

        }
        else
        // PDO list requested
        if (event_object == getpdolistbutton)
        {
          #ifdef DEBUG                      
          Serial.println("Request PDO list");
          #endif
          // Prepare the command to engage the hardware
          memset(SendCommand, 0, COMMAND_SIZE); 
          SendCommand[COMMANDPOSITION]   = CMD_get_PDOList;
          SendCommand[BOARDPOSITION]     = BoardInfo.getBoardNumber();
          SendCommand[LENGTHPOSITION]    = 0U; // length
        }
        else
        {
          flagSoundButton = false;
          #ifdef DEBUG                      
          Serial.println("Unknown button pressed");
          #endif
          if (event_user_data == screenpdo)
          {
            #ifdef DEBUG                      
            //Serial.println("Button from screen pdo");
            #endif
            if (object_user_data != NULL)
            {
              flagSoundButton = true;

              // WE got a PDO select click !!
              byte SelectPDOindex = ((byte)(uintptr_t)object_user_data);      
              #ifdef DEBUG
              Serial.printf("PDO button %d pressed.\r\n", SelectPDOindex);
              #endif

              // Prepare the command to engage the hardware
              memset(SendCommand, 0, COMMAND_SIZE); 
              SendCommand[COMMANDPOSITION]   = CMD_set_MAXPDO;
              SendCommand[BOARDPOSITION]     = BoardInfo.getBoardNumber();
              SendCommand[LENGTHPOSITION]    = 1U; // length
              SendCommand[DATASTART]         = SelectPDOindex;
            }
          }
        }

      }
    }

    if ( (lv_obj_check_type(event_object, &lv_keyboard_class)) || (lv_obj_check_type(event_object, &lv_checkbox_class)) )
    {
      flagSoundButton = true;

      #ifdef DEBUG                      
      Serial.println("Event: keyboard/checkbox value event");
      #endif

      TStageMode SM = smOff;
      TThresholdModes Mode = tmNONE;
      SD = NULL;  

      if (lv_obj_check_type(event_object, &lv_keyboard_class))
      {
        if (object_user_data != NULL)
        {
          // Only valid for keyboard data
          if (object_user_data == testdischargebutton) SM = smCurrent;
          if (object_user_data == testchargebutton) SM = smCharge;            
        }
      }

      if (lv_obj_check_type(event_object, &lv_checkbox_class))
      {
        if (object_user_data != NULL)
        {
          // Only valid for checkbox data
          SM = (TStageMode)highByte((word)(uintptr_t)object_user_data);      
          Mode = (TThresholdModes)lowByte((word)(uintptr_t)object_user_data);
        }
      }

      if (SM == smCurrent)
      {
        SD = &SET->Stages[FIXEDDISCHARGESTAGENUMBER];
        #ifdef DEBUG          
        Serial.println("We got a discharge setting !!");
        #endif
      }
      else
      if (SM == smCharge)
      {
        SD = &SET->Stages[FIXEDCHARGESTAGENUMBER];
        #ifdef DEBUG          
        Serial.println("We got a charge setting !!");
        #endif
      }
      else
      {
        #ifdef DEBUG          
        Serial.println("Unknown stagemode. Should never happen !!");
        #endif
      }

      if (SD != NULL)
      {
        if (lv_obj_check_type(event_object, &lv_checkbox_class))
        {
          #ifdef DEBUG          
          Serial.println("Enable or disable a threshold !!");
          #endif
          SD->ThresholdSettings[Mode].Enabled = (lv_obj_get_state(event_object) & LV_STATE_CHECKED); 
          GotSettings = true;
        }
 
        if (lv_obj_check_type(event_object, &lv_keyboard_class))
        {
          if (code == LV_EVENT_READY)
          {
            #ifdef DEBUG                      
            Serial.println("Event: keyboardready event");
            #endif
            const char * txt = lv_textarea_get_text(lv_keyboard_get_textarea(event_object));
            const unsigned long value = strtoul(txt, NULL, 10);
            SD->SetValue = value;  
            GotSettings = true;
          }
        }
      }

    }
    #endif //STANDALONE


    #ifdef STANDALONE
    if (GotSettings)
    {
      GotSettings = false;      
      #ifdef DEBUG
      //Serial.println("Perpare storing settings in NVM !");
      #endif
      //StoreSettings = true;
    }   
    #endif
  }

  in_handler = false;
}

#ifdef STANDALONE
static void cbCollectRotoPDData()
{
  flagCollectRotoPDData = true;
}

static void cbRequestIT2704Data()
{
  flagRequestIT2704Data = true;
}

static void cbProcessData()
{
  flagProcessMeasurementData = true;
}

static void StopClickCB()
{
  bsp_buzzer_enable(false);
}


static void datastartcb(byte index)
{
  PBatterySetting SET = &Batteries[index];  
  PRunDatas RDS = &SET->TestData.RunDatas;
  // Clear the rundatas again, this is the real start !!  
  ClearRunData(RDS); 
  // We are active !!
  SET->TestData.Active = bmActive;
  // Force a very early data measurement to get a start value
  flagProcessMeasurementData = true;
}

#endif

void dataupdatecb()
{
  // Inform the loop to collect the battery data
  CalcBatteryData = true;
}

#ifdef ARDUINO_ARCH_ESP32
void Core0WorkerTask(void *pvParameters) {
  
  // --- THIS REPLACES void setup1() ---
  mutex_enter_blocking(&can_mutex);
  node.getCAN().run();
  mutex_exit(&can_mutex);  

  // --- THIS REPLACES void loop1() ---
  for (;;) {
    bool mutexOk = false;
    mutexOk = mutex_enter_timeout_ms(&can_mutex, 1 /*ms*/);
    if (mutexOk) {
      node.getCAN().run(true);
      mutex_exit(&can_mutex);  
    }  
    vTaskDelay(pdMS_TO_TICKS(1)); 
  }
}
#endif

#ifdef ARDUINO_ARCH_RP2040
// Running on core1
void setup1() {
  //delay(1);
  mutex_enter_blocking(&can_mutex);
  node.getCAN().run();
  mutex_exit(&can_mutex);  
}

void loop1() {
  bool mutexOk = false;
  //mutex_enter_blocking(&can_mutex);
  mutexOk = mutex_enter_timeout_ms(&can_mutex, 1/*ms*/);
  if (mutexOk)
  {
    node.getCAN().run(true);
    mutex_exit(&can_mutex);  
  }  
  if (mutexOk)
  {
    // We might have entered the mutex without any problem
    // So yield here to give other tasks some change
    // If we had a timeout, other tasks have already got their chance
    yield();
  }
}
#endif

void setup()
{
  byte index;
  word tempintcalc;
  char myHex[10] = "";

  // If this throws an error during compilation, you know you need to adjust your padding array size!
  static_assert(sizeof(CircularDataQueue<TMeasurementData, DATASIZE>) * 2 == 88, 
              "Mismatched C/C++ structural padding! Update _cpp_queue_padding size to match.");  

  //delay(250);

  // Start by blocking all CAN activity
  mutex_init(&can_mutex);
  mutex_enter_blocking(&can_mutex);

  #ifdef ARDUINO_ARCH_RP2040

  // 1. STEP THE VOLTAGE UP INCREMENTALLY
  // Stepping avoids sudden current spikes that trip brownout resets.
  //vreg_set_voltage(VREG_VOLTAGE_1_20);
  //delay(20);
  //vreg_set_voltage(VREG_VOLTAGE_1_25);
  //delay(20);
  //vreg_set_voltage(VREG_VOLTAGE_1_30); // Max recommended safe internal voltage
  //delay(20); // Allow LDO rail stabilization

  //set_sys_clock_khz(400000, false);  
  //delay(10);

  // Change system clock to exactly match the PSRAM clock (133MHz) by an integer factor
  // This results in best LCD performance without glitches and noise
  set_sys_clock_khz(266000, true);  
  delay(10);

  // Tune the PSRAM settings to match the system clock
  // This results in best LCD performance without glitches and noise
  set_psram_timing_auto();
  delay(10);
  #endif

  for (index=0; index<12;index++)
  {
    BoardInfo.BoardSerial[index] = DefaultBoardSerial[index];
  }

  storeInit();
  storeGetBoardInfo(&BoardInfo);

  index = 0;
  mySerial[0] = '\0';  
  while (index<12)
  {
    tempintcalc=(BoardInfo.BoardSerial[index]+(BoardInfo.BoardSerial[index+1]*256));
    sprintf(myHex, "%04X", tempintcalc);    
    strcat(mySerial,myHex);    
    index += 2;
    if (index<12) strcat(mySerial,"-");
  }
  mySerial[29] = '\0';  

  myFirmware[0] = '\0';  
  sprintf(myFirmware, "USB-PD-2026 V%02d-%02d", FW_MAJOR, FW_MINOR);
  myFirmware[18] = '\0';   

  #ifdef ENABLEUSB

  #ifdef ARDUINO_ARCH_ESP32

  #if defined(ARDUINO_USB_MODE) && ARDUINO_USB_MODE == 0  // USB-OTG / TinyUSB
  
  USB.firmwareVersion(0x0100);
  USB.manufacturerName("Consulab for pleasure");
  USB.productName("USB PD controller with HID");
  USB.VID(0x04D8);
  USB.PID(0x003F);  
  USB.usbVersion(0x0200);

  tinyusb_add_string_descriptor(myFirmware);

  Device.begin();

  USBSerial.begin();

  USB.begin();

  #endif
  #endif

  #ifndef DEBUG
  // This will remove the unwanted default string descriptor
  // And kill the unwanted (unneeded) serial port.
  //USBSerial.end();
  #endif

  #if defined(ARDUINO_ARCH_RP2040) && defined(USE_TINYUSB)
  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }
  USBDevice.setID(0x04D8,0x003E);
  USBDevice.setVersion(0x0200);        // USB 2.0
  USBDevice.setDeviceVersion(0x0100);  // Device version 1.00  
  USBDevice.setManufacturerDescriptor("Consulab for pleasure");
  USBDevice.setProductDescriptor("USB-C PD controller");
  #endif

  #ifdef ARDUINO_ARCH_ESP32
  #if defined(ARDUINO_USB_MODE) && ARDUINO_USB_MODE == 0  // USB-OTG / TinyUSB
  USB.VID(0x04D8);
  USB.PID(0x003F);  
  USB.usbVersion(0x0200);
  USB.firmwareVersion(0x0100);
  USB.manufacturerName("Consulab for pleasure");
  USB.productName("USB PD controller with HID");
  #endif
  #endif

  #if defined(ARDUINO_ARCH_RP2040) && defined(USE_TINYUSB)
  USBDevice.setSerialDescriptor(mySerial);  
  HID.setReportCallback(NULL, set_report_callback);
  HID.enableOutEndpoint(true);
  HID.setPollInterval(1);
  HID.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  USBDevice.addStringDescriptor(myFirmware);  

  HID.begin();

  // Enable serial (again) for programming and debugging
  #ifdef DEBUG  
  Serial.begin(115200);
  #endif

  #endif

  #endif // ENABLEUSB

  #ifdef DEBUG  
  int cnt = 1000;     // Will wait for up to ~1 second for Serial to connect.
  while (!Serial && cnt--) {delay(1);}
  // Serial.setDebugOutput(true);
  #endif

  #ifdef ARDUINO_ARCH_RP2040
  Serial.println("Starting RP2350 init.");

  WireBattery.setSDA(BSP_I2C_SDA_PIN);
  WireBattery.setSCL(BSP_I2C_SCL_PIN);
  //WireBattery.setClock(400000);   // or even 50000
  #endif

  WireBattery.begin();

  bsp_buzzer_init();

  Info_Add("GUI. Init our LVGL display wonder.");    
  lv_init();

  #ifdef ARDUINO_ARCH_ESP32
  lv_screen_init(gfx, HOR_RES, VER_RES);
  #endif

  #ifdef ARDUINO_ARCH_RP2040
  lv_screen_init(HOR_RES, VER_RES);
  lv_screen_brightness(0);
  #endif

  lv_tick_set_cb(my_tick_get_cb);  // Tell LVGL how to get the current time

  // Create and show flash-screen
  CreateSaveScreen(NULL);
  lv_timer_handler();
  lv_screen_brightness(100);

  CreateBaseScreen(main_event_handler);
  Setup_ScreenLogger(BoardInfo.IT2704Channel,false);

  Info_Add("GUI. Init GUI.");      

  Setup_ScreenSettings(BoardInfo.IT2704Channel,false);
  Setup_ScreenPDO(BoardInfo.IT2704Channel,false);
  Setup_Screen1(BoardInfo.IT2704Channel);

  SetDarkTheme(BoardInfo.UIDarkEnabled);
  SetButtonBeep(BoardInfo.BeepEnabled);
  SetNodeAdress(BoardInfo.LocalNodeID);
  SetMasterAdress(BoardInfo.MasterNodeID);
  SetIT2704Adress(BoardInfo.IT2704NodeID);
  SetIT2704Channel(BoardInfo.IT2704Channel);

  Info_Add("GUI. Controller startup");

  //WiFi.mode(WIFI_OFF);

  PBatterySetting SET;
  PRunDatas RDS;
  PStageData SD;  

  // Get memory for datastore
  // Set some defaults
  for(index = 0; index < DAUGHTERBOARDCOUNT; index++)
  {
    SET = &Batteries[index];

    RDS = &SET->TestData.RunDatas;
    ClearRunData(RDS);
  
    SET->TestData.Active = bmIdle;
    SET->TestData.SetStageMode = smOff;
    SET->TestData.SetStageValue = 0;
    SET->TestData.DataTriggerCounter = 0;
  }

  #ifdef STANDALONE

  memset(SendCommand, 0, COMMAND_SIZE); 
  SendCommand[COMMANDPOSITION] = CMD_unknown;

  // Set and get defaults;
  for(index = 0; index < DAUGHTERBOARDCOUNT; index++)
  {
    SET = &Batteries[index];

    if (false)
    {
      // Default discharge
      SD = &SET->Stages[FIXEDDISCHARGESTAGENUMBER];
      SD->Status = smCurrent; // a CC discharge
      SD->SetValue = 250; // 500mA    
      SD->ThresholdSettings[tmMINV].SetValue = 900; //900mV end value
      SD->ThresholdSettings[tmMINV].Enabled = true;
    }

    SD = &SET->Stages[IDDLESTAGENUMBER];
    ClearStageData(SD);
    SD = &SET->Stages[FIXEDDISCHARGESTAGENUMBER];
    ClearStageData(SD);
    SD = &SET->Stages[FIXEDCHARGESTAGENUMBER];
    ClearStageData(SD);

    SET->Stages[FIXEDDISCHARGESTAGENUMBER].ThresholdSettings[tmMINV].Enabled = true;
    SET->Stages[FIXEDDISCHARGESTAGENUMBER].ThresholdSettings[tmMINV].Mode = tmMINV;

    if (storeGetBatteryDischargeSetting(index, &SET->Stages[FIXEDDISCHARGESTAGENUMBER])) Info_Add("GUI. Reading discharge settings success.");
    if (storeGetBatteryChargeSetting(index, &SET->Stages[FIXEDCHARGESTAGENUMBER])) Info_Add("GUI. Reading charge settings success.");
  }

  Info_Add("GUI. Reading stored presets done.");

  #endif

  // Fill startscreen with presets, if any
  SET = Battery;
  Screen1SetData(SET);

    // INA238 setup
  if (initINA238())
    Info_Add("GUI. INA238 init success.");
  else
    Info_Add("GUI. INA238 init failed !!");

  if (pd.isConnected())
    Info_Add("GUI. RotoPD connected.");
  else
    Info_Add("GUI. RotoPD not connected or not found.");

  Info_Add("GUI. Init timers.");      

  #ifdef STANDALONE

  tickerCollectRotoPDData.attach_ms(ROTOPDDATAGETTIME, cbCollectRotoPDData);
  tickerProcessData.attach_ms(DATACOLLECTTIMEFAST, cbProcessData);
  tickerDataUpdate.attach_ms(CALCULATIONTIME, dataupdatecb);  

  // Init touch device
  Info_Add("GUI. Init touch screen.");      
  touch_init(HOR_RES, VER_RES, 0);
  /*Initialize the input device driver*/
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);
  #endif

  String LVGL_Arduino = "GUI. LVGL " + String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Info_Add(LVGL_Arduino.c_str());

  #ifdef ARDUINO_ARCH_RP2040
  
  Info_Add_Fmt("CPU Frequency: %d MHz.",rp2040.f_cpu() / 1000000);

  Info_Add("GUI. RP2350 SRAM memory info.");
  // Internal SRAM heap
  Info_Add_Fmt("SRAM free  : %u KB.", (rp2040.getFreeHeap() / 1024));
  Info_Add_Fmt("SRAM buffer: %u KB.", (lv_port_get_buffer_heap_usage() / 1024));
  Info_Add_Fmt("SRAM free after lvgl buffer malloc : %u KB.", ( (rp2040.getFreeHeap() - lv_port_get_buffer_heap_usage()) / 1024) );

  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);
  Info_Add_Fmt("LVGL heap usage. Used: %u KB. Free: %u KB. Biggest free: %u KB.",
       (mon.total_size - mon.free_size) / 1024,
       mon.free_size / 1024,
       mon.free_biggest_size / 1024);
  Info_Add_Fmt("LVGL heap fragmentation: %u%%.", mon.frag_pct);
  Info_Add_Fmt("SRAM free after lvgl buffer malloc : %u KB", ( (rp2040.getFreeHeap() - lv_port_get_buffer_heap_usage() - mon.total_size) / 1024) );

  Info_Add("GUI. RP2350 PSRAM memory info.");
  // PSRAM heap
  Info_Add_Fmt("PSRAM free  : %u KB.", rp2040.getFreePSRAMHeap() / 1024);
  Info_Add_Fmt("PSRAM total : %u KB.", rp2040.getTotalPSRAMHeap() / 1024);
  Info_Add_Fmt("PSRAM size  : %u KB.", rp2040.getPSRAMSize() / 1024);

  /*
  for (int i = 0; i < 16; i++) {
      gpio_set_drive_strength(BSP_LCD_DATA0_PIN + i, GPIO_DRIVE_STRENGTH_12MA); // or 12MA
      gpio_set_slew_rate(BSP_LCD_DATA0_PIN + i, GPIO_SLEW_RATE_SLOW);
  }
  gpio_set_drive_strength(BSP_LCD_PLCK_PIN, GPIO_DRIVE_STRENGTH_12MA);
  gpio_set_slew_rate(BSP_LCD_PLCK_PIN, GPIO_SLEW_RATE_SLOW);
  */
  
  #endif

  //delay(2000);

  init_screensaver_controller(SAVERTIME);
  switch_to_dark(BoardInfo.UIDarkEnabled);
  lv_screen_load_anim(screenbase, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
  //lv_timer_handler();

  Info_Add("GUI. Setting up CANOpen.");

  Info_Add_Fmt("GUI. Board-number: %d.",BoardInfo.getBoardNumber());
  Info_Add_Fmt("GUI. Channel-number: %d.",BoardInfo.IT2704Channel);
  Info_Add_Fmt("GUI. Local Node-ID: %d.",BoardInfo.LocalNodeID);
  Info_Add_Fmt("GUI. Master Node-ID: %d.",BoardInfo.MasterNodeID);
  Info_Add_Fmt("GUI. IT2704 Node-ID: %d.",BoardInfo.IT2704NodeID);

  #ifdef ARDUINO_ARCH_ESP32

  // Set pins
  ESP32Can.setPins(CAN_TX, CAN_RX);

  // You can set custom size for the queues - those are default
  ESP32Can.setRxQueueSize(2);
  ESP32Can.setTxQueueSize(5);

  // .setSpeed() and .begin() functions require to use TwaiSpeed enum,
  // but you can easily convert it from numerical value using .convertSpeed()
  ESP32Can.setSpeed(ESP32Can.convertSpeed(250));

  // You can also just use .begin()..
  if(ESP32Can.begin())
    Info_Add("GUI. CAN bus started!");
  else
    Info_Add("GUI. CAN bus failed!");
  #endif

  #ifdef ARDUINO_ARCH_RP2040
  initCANopen();
  #endif

  tickerRequestIT2704Data.attach_ms(IT2704DATAREQUESTTIME, cbRequestIT2704Data);
  Info_Add("GUI. Setting up CANOpen ready.");

  Info_Add("GUI. Init RP2350 ready.");

  // Release the CAN activity
  mutex_exit(&can_mutex);  
}

void loop()
{
  uint8_t j;

  loopStartTime = millis();

  static unsigned long startTime = millis();

  // We do not know when IT2704 data will arrive
  // Data-order is also unknown
  // So use a static to store latest data
  static TMeasurementData LatestIT2704Data;
  // For uniformity, also use this method for RotoPD data
  static TMeasurementData LatestRotoPDData;

  bool DataOk = false;

  static byte INData[COMMAND_SIZE] = {0};
  static byte hid_report_in[HID_INT_IN_EP_SIZE] = {0};  

  byte dataindexer = 0;

  WORD_VAL w_data;
  DWORD_VAL dw_data;
  QWORD_VAL qw_data;

  PBatterySetting SET = Battery;
  PRunDatas RDS = &SET->TestData.RunDatas; 

  int8_t PDOCount = 0;
  static AP33772S_PDO PDO;

  #if defined(ARDUINO_ARCH_ESP32)
  // Pin and run your setup1/loop1 emulation function on Core 0
  xTaskCreatePinnedToCore(
    Core0WorkerTask,   // The worker function above
    "CAN_Worker",      // Debug name of the task
    4096,              // Stack size (4KB is usually safe for CAN operations)
    NULL,              // Task input parameters
    2,                 // Priority (Set slightly higher than 1 to ensure responsive CAN updates)
    NULL,              // Task handle
    0                  // Core ID (0 = The background core)
  );
  #endif

  if (node.getCAN().checkRxQueue())
  {
    //mutex_enter_blocking(&can_mutex);
    if (mutex_enter_timeout_ms(&can_mutex, 100))
    {

      unsigned long maxCANTime = millis();

      while (node.run())
      {
        // We did receive a message that was not handled by ourselves
        const Message& rxmsg = node.getMessage();

        // ---- process the message ----
        if (load.checkConnection(&rxmsg)) // check if we are connected with the IT2704
        {

          // Check TPDO from load messages, if any
          if (load.getTPDOEnergyData(&rxmsg, &LatestIT2704Data.E))
          {
            //Serial.println("GUI [IT2704]. Received correct TPDO message.");
            ScreenPowerSetEnergyData(LatestIT2704Data.E);
            telemetry_screensaver_update_E(LatestIT2704Data.E);
          }

          // Check SDO message from load
          if (rxmsg.id == (CO_CAN_ID_SDO_SRV + load.getID()))
          {
            //Serial.println("GUI [IT2704]. Receiving SDO response.");

            if (rxmsg.getCommand() == READ_REQ_4BYTE_CMD)
            {
              // Check for meter data that has the meaurement results from our meter data request
              if ( rxmsg.getID() == ((IT2704_METER + (load.getChannel() * 0x100))) )
              {
                // Get the correct IT2704 data in the correct datastore
                load.getData(&rxmsg, &LatestIT2704Data.I, &LatestIT2704Data.V, &LatestIT2704Data.P, &LatestIT2704Data.C, &LatestIT2704Data.E);

                switch(rxmsg.getSubID())
                {
                  case IT2704_METER_VOLTAGE : {ScreenPowerSetVoltageData(LatestIT2704Data.V); telemetry_screensaver_update_V(LatestIT2704Data.V); break;}
                  case IT2704_METER_CURRENT : {ScreenPowerSetCurrentData(LatestIT2704Data.I); telemetry_screensaver_update_I(LatestIT2704Data.I); break;}
                  case IT2704_METER_POWER   : {ScreenPowerSetPowerData(LatestIT2704Data.P);   telemetry_screensaver_update_P(LatestIT2704Data.P); break;}
                  case IT2704_METER_WHOUR   : {ScreenPowerSetEnergyData(LatestIT2704Data.E);  telemetry_screensaver_update_E(LatestIT2704Data.E); break;}            
                }
              }
            }

            if (rxmsg.getCommand() == WRITE_RESP_SUCCESS_CMD)
            {
              #ifdef DEBUG                      
              //Serial.println("GUI [IT2704]. Receiving response from writing data.");
              //Serial.print("GUI [IT2704]. Writing data success. ");
              //Serial.print("Index 0x");
              //Serial.printf("%04X",rxmsg.getID());
              //Serial.printf(". SubIndex %d.\r\n",rxmsg.getSubID());
              #endif
            }
          }
        }

        node.clearMessage();

        // Limit the amount of time inside this loop
        if (millis() - maxCANTime > 100)
        {
          #ifdef DEBUG                      
          Serial.println("GUI. CAN node.run() timeout.");
          #endif
          break;
        }
      }
      mutex_exit(&can_mutex);  
    }
    else
    {
      #ifdef DEBUG                      
      Serial.println("GUI. CAN mutex timeout.");
      #endif
    }
  }

  #ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
  #endif

  if (flagCollectRotoPDData)
  {
    flagCollectRotoPDData = false;
    // Get RotoPD measurement results for later averaging
    // Runs every 100ms
    collectRotoPDData();

    // If we do not have TPDOs with energy, this might be a quick energy data solution
    //load.collectDataEnergyOnly();
  }

  if (flagRequestIT2704Data)
  {
    if (mutex_enter_timeout_ms(&can_mutex, 100))
    {
      flagRequestIT2704Data = false;
      // Ask load IT2704 to send measurement results
      // Result (when available) will be collected later by node.run()
      load.collectData();
      mutex_exit(&can_mutex);
    }
    else
    {
      #ifdef DEBUG                      
      Serial.println("GUI. CAN mutex timeout.");
      #endif
    }

  }

  if (flagSoundButton)
  {
    flagSoundButton = false;
    bsp_buzzer_enable(true);
    SoundButtonTicker.once_ms(100U, StopClickCB);
  }

  if (millis() - startTime >= 1000)
  {
    // This runs every second !!
    startTime = millis();

    #ifdef DEBUG                      
    static uint32_t loopcounter = 0;
    Serial.printf("Loop: %d\n",loopcounter++);
    #endif

    if (!load.isConnected())
    {
      #ifdef DEBUG
      Serial.println("GUI. IT2704 disconnected !");
      #endif
    }

    // Test connection with load IT2704
    if (mutex_enter_timeout_ms(&can_mutex, 100 /*ms*/))
    {
      load.queryConnection();
      mutex_exit(&can_mutex);  
    }
    else
    {
      #ifdef DEBUG                      
      Serial.println("GUI. CAN mutex timeout.");
      #endif
    }


    PDOCount = taskRotoPDInit();

    if (PDOCount != -1)
    {
      // Got PDO data from plugin of PD source
      Info_Add("GUI. Plugin of PD source !!");

      if (PDOCount == 0) Info_Add("GUI. No new PDO's !!");

      // We have a newly connected RotoPD or new PDO's
      if (PDOCount>0)
      {
        Info_Add_Fmt("GUI. Process new PDO's !! PDO count: %d",PDOCount);
       
        ScreenPDOClearPDOList();

        memset(hid_report_in, 0, HID_INT_IN_EP_SIZE);

        hid_report_in[COMMANDPOSITION] = CMD_get_PDOList;
        hid_report_in[BOARDPOSITION] = BoardInfo.getBoardNumber();

        dataindexer = DATASTART;

        hid_report_in[dataindexer++] = PDOCount;

        for ( j=1; j<=MAX_PDO_ENTRIES; j++ )
        {
          memset(&PDO, 0, sizeof(PDO));

          if (pd.readPDO(j, PDO))
          {
            if (PDO.valid)
            {
              if (PDO.isEPR)
                Info_Add_Fmt("GUI. EPR PDO received ! PDO voltage: %dmV.", PDO.maxVoltage_mV);
              else
                Info_Add_Fmt("GUI. PDO received ! PDO voltage: %dmV.", PDO.maxVoltage_mV);
              #ifdef STANDALONE
              ScreenPDOSetPDO(PDO.index,PDO.valid,PDO.isEPR,PDO.type,PDO.minVoltage_mV,PDO.maxVoltage_mV,PDO.maxCurrent_mA);
              #endif
              hid_report_in[dataindexer++] = PDO.index;
              w_data.Val = PDO.raw;
              hid_report_in[dataindexer++] = w_data.bytes.LB;
              hid_report_in[dataindexer++] = w_data.bytes.HB;
            }  
          }
        }
        // Send PDO data to USB host
        hid_report_in[LENGTHPOSITION]=dataindexer;   

        #ifdef ENABLEUSB
        #ifdef ARDUINO_ESP32S3_DEV
        HID.SendReport(0, hid_report_in, HID_INT_IN_EP_SIZE);
        #else
        HID.sendReport(0, hid_report_in, HID_INT_IN_EP_SIZE);
        #endif      
        #endif

      }
    }
  }  

  if (pending_screen_index != 0)
  {
    byte target = pending_screen_index;
    pending_screen_index = 0;
    switch(target)
    {
      case 1: {Setup_Screen1(BoardInfo.IT2704Channel);Screen1SetData(SET);break;}
      case 2:
        {
          TMeasurementData LastData;
          //LastData = RDS->NewerRotoPDDatas.getLastData();
          LastData  = RDS->NewerIT2704Datas.getLastData();
          Setup_ScreenPower(BoardInfo.IT2704Channel);ScreenPowerSetData(&LastData);
          break;
        }
      case 3: {Setup_ScreenChart(BoardInfo.IT2704Channel);ScreenChartSetDataQueue(RDS->NewerRotoPDDatas);break;}
      case 4: {Setup_ScreenPDO(BoardInfo.IT2704Channel,true);break;}
      case 5: {Setup_ScreenSettings(BoardInfo.IT2704Channel,true);break;}
      case 6: {Setup_ScreenLogger(BoardInfo.IT2704Channel,true);break;}
    }
  }

  THIDData* PLocalHD;
  THIDData LocalHDCopy;

  memset(INData,0,COMMAND_SIZE);  

  // Do we have a valid command ?
  // This is a command coming from the USB HID interface
  // Most likely, this command originates from a desktop PC
  if (!DataOk)
  {
    if (HIDData[ActiveBatteryIndex].DataReceived)
    {
      #ifndef TINYUSB_NEED_POLLING_TASK
      // Make a local copy of the data 
      // This is needed due to the fact that the USB HID interrupt may update the data when in this loop 
      noInterrupts();
      for ( j=0; j<HID_INT_OUT_EP_SIZE; j++ ) LocalHDCopy.HIDEPOUTData[j] = HIDData[ActiveBatteryIndex].HIDEPOUTData[j];
      LocalHDCopy.DataReceived = HIDData[ActiveBatteryIndex].DataReceived;
      HIDData[ActiveBatteryIndex].DataReceived = false;
      interrupts();
      PLocalHD = &LocalHDCopy; 
      #else
      PLocalHD = (THIDData*)&HIDData[ActiveBatteryIndex];
      PLocalHD->DataReceived = false;
      #endif

      //Now perform the Data update  
      DataOk |= process_command(&PLocalHD->HIDEPOUTData,&PLocalHD->HIDEPINData);
      if (DataOk)
      {
        //ARDUINO_GENERIC_RP2350
        //ARDUINO_ARCH_RP2040

        // Send report back to host
        #ifdef ENABLEUSB
        #ifdef ARDUINO_ESP32S3_DEV
        HID.SendReport(0, &PLocalHD->HIDEPINData, HID_INT_IN_EP_SIZE);
        #else
        HID.sendReport(0, &PLocalHD->HIDEPINData, HID_INT_IN_EP_SIZE);
        #endif
        #endif

        // Copy the result into the INData buffer for later processing !
        for (j=0; j<HID_INT_IN_EP_SIZE; j++) INData[j] = PLocalHD->HIDEPINData[j];
      }
    }  
  }

  #ifdef STANDALONE
  if (!DataOk)
  {
    // Do we have a valid command ?
    // This is a command coming from the touch-screen logic = main_event_handler()
    if ( (SendCommand[COMMANDPOSITION] != CMD_unknown) && (SendCommand[COMMANDPOSITION] != USB_CMD_error) )
    {
      // Process the command and 
      DataOk |= process_command(&SendCommand,&INData);
      // Reset the command
      SendCommand[COMMANDPOSITION] = CMD_unknown;
  
    }
  }
  #endif //STANDALONE


  if (DataOk)
  {
    DataOk = false;

    // We have received a command from the screen or through USB-HID or the standalone measurement timer.
    // The command has been processed.
    // Now we can or might use the results of this processed command.
    // This is mostly for GUI updates !

    CommandType_t cCmd = (CommandType_t)INData[COMMANDPOSITION];
    byte BoardNumber = INData[BOARDPOSITION];
    byte Length = INData[LENGTHPOSITION];
    byte counter = DATASTART;

    if (BoardNumber != BoardInfo.getBoardNumber())
    {
      // Should never happen !!
      Info_Add("GUI. Severe error. Wrong boardnumber !!!");
      Info_Add_Fmt("GUI. Received board %d. BoardInfo has %d.", BoardNumber, BoardInfo.getBoardNumber());
    }

    switch(cCmd)
    {

      case CMD_set_value:
      {
        Screen1SetStageData(&CurrentBoard->BM);
        break;
      }

      case CMD_set_energy:
      case CMD_set_capacity:
      case CMD_set_time:
      {
        qw_data.Val = 0;

        for ( j=0; j<Length; j++ ) {qw_data.v[j] = INData[counter++];}

        if (cCmd == CMD_set_energy) Info_Add("GUI. CMD_set_energy received.");
        if (cCmd == CMD_set_capacity) Info_Add("GUI. CMD_set_capacity received.");
        if (cCmd == CMD_set_time) Info_Add("GUI. CMD_set_time received.");
        Info_Add_Fmt("GUI. DataValue [length %d]: %d.", Length, qw_data.Val);

        if (cCmd == CMD_set_energy)
        {
          RDS->Energy = qw_data.Val; // in nAh
          Screen1AddEData(RDS->Energy / 1000000ULL);
          if (RDS->Energy == 0)
          {
            if (mutex_enter_timeout_ms(&can_mutex, 100))
            {
              load.clearEnergy();
              mutex_exit(&can_mutex);  
            }
            else
            {
              #ifdef DEBUG                      
              Serial.println("GUI. CAN mutex timeout.");
              #endif
            }
            ClearRunData(RDS); 
            ClearMeasurementData(&LatestRotoPDData);
            ClearMeasurementData(&LatestIT2704Data);
          }
        }
        if (cCmd == CMD_set_capacity)
        {
          RDS->Capacity = qw_data.Val; // in nWh
          //Screen1AddCData(RDS->Capacity);
        }

        if (cCmd == CMD_set_time)
        {
          RDS->Time = qw_data.Val;  // in ms
          Screen1AddTData(RDS->Time);
        }

        break;
      }

      case CMD_set_MAXPDO:
      case CMD_set_FIXEDPDO:
      case CMD_set_PPSPDO:
      case CMD_set_AVSPDO:
      {
        // Do we need to do someting with the new PDO ?
        // Yes, we might set the discharge current stage values with it !
        SET->Stages[FIXEDDISCHARGESTAGENUMBER].SetValue = CurrentBoard->PDO.maxCurrent_mA;
        SET->Stages[FIXEDDISCHARGESTAGENUMBER].Status = smCurrent;
        // Update the GUI also to reflect this change
        Screen1SetStageData(&SET->Stages[FIXEDDISCHARGESTAGENUMBER]);        

        break;
      }

      case CMD_get_PDOList:
      case CMD_read_PDOList:
      {
        // We need to update the GUI with the received PDO's !!

        PDO_DATA_T raw;

        // Got PDO data from plugin of PD source
        Info_Add("GUI. PDO list request by user !!");

        ScreenPDOClearPDOList();

        PDOCount = INData[counter++];

        Info_Add_Fmt("GUI. Process new PDO's !! PDO count: %d",PDOCount);

        if (PDOCount)
        {
          while ((PDOCount--)>0)
          {
            memset(&PDO, 0, sizeof(PDO));      

            j = INData[counter++];

            //Info_Add_Fmt("GUI. PDO received ! PDO index: #%d.", j);

            if (j)
            {
              raw.byte0 = INData[counter++];
              raw.byte1 = INData[counter++];
              // This fuction is index zero based !!
              AP33772S::decodePDONew(j-1, raw, PDO);

              if (PDO.valid)
              {
                if (PDO.isEPR)
                  Info_Add_Fmt("GUI. EPR PDO received ! PDO voltage: %dmV.", PDO.maxVoltage_mV);
                else
                  Info_Add_Fmt("GUI. PDO received ! PDO voltage: %dmV.", PDO.maxVoltage_mV);

                ScreenPDOSetPDO(PDO.index,PDO.valid,PDO.isEPR,PDO.type,PDO.minVoltage_mV,PDO.maxVoltage_mV,PDO.maxCurrent_mA);
              }
            }
          }

        }
        break;
      }
    }
  }

  
  #ifdef STANDALONE
  if (flagProcessMeasurementData)
  {
    flagProcessMeasurementData = false;

    switch(SET->TestData.Active)
    {
      case bmActive:
        // Battery is active. Slowdown the data acquisition to get accurate data into a datastore
        if (SET->TestData.DataTriggerCounter > 0) SET->TestData.DataTriggerCounter--;
        // Do not engage the screensaver when active
        reset_screensaver_idle_timer();
        break;
      case bmReady:
        SET->TestData.DataTriggerCounter = 0;
        break;
      case bmIdle:
        SET->TestData.DataTriggerCounter = 0;
        // Do not engage the screensaver when idle
        //reset_screensaver_idle_timer();
        break;
      default:
        #ifdef DEBUG  
        Serial.print("Invalid battery mode !! Number: ");
        Serial.println(SET->TestData.Active);          
        #endif
        break;
    }

    if (SET->TestData.DataTriggerCounter == 0)
    {
      // Request the measurement data
      memset(INData,0,COMMAND_SIZE);  
      memset(hid_report_in, 0, HID_INT_IN_EP_SIZE);

      // Simulate get_data command to get the RotoPD data
      hid_report_in[COMMANDPOSITION] = CMD_get_data;
      hid_report_in[BOARDPOSITION] = BoardInfo.getBoardNumber();

      process_command(&hid_report_in,&INData);

      // Get the measurement data results from the command above
      dataindexer = DATASTART;
      for ( j=0; j<2; j++ ) {w_data.v[j] = INData[dataindexer++];}
      LatestRotoPDData.V = w_data.Val;
      for ( j=0; j<2; j++ ) {w_data.v[j] = INData[dataindexer++];}
      LatestRotoPDData.I = w_data.Val;
      for ( j=0; j<4; j++ ) {dw_data.v[j] = INData[dataindexer++];}
      LatestRotoPDData.P = dw_data.Val;
      for ( j=0; j<2; j++ ) {w_data.v[j] = INData[dataindexer++];}
      LatestRotoPDData.T = w_data.Val;

      //Append the data in RotoPD buffer
      RDS->NewerRotoPDDatas.push(LatestRotoPDData);

      telemetry_screensaver_update_box5(node.getCAN().getRxQueueDepth()*10);
      telemetry_screensaver_update_box6(RDS->NewerRotoPDDatas.getCount()*10);      

      if (load.isConnected())
      {
        //Also append the IT2704 data in IT2704 buffer
        RDS->NewerIT2704Datas.push(LatestIT2704Data);
      }
      else
      {
        // if we are not connected, reset the static measurement data !!
        ClearMeasurementData(&LatestIT2704Data);
      }

      //telemetry_screensaver_update_i2c(&RDS->LastBatteryData);
      //telemetry_screensaver_update_i2c(&LatestIT2704Data);

      // Append data into graphs
      ScreenChartAddData(LatestRotoPDData.V, LatestRotoPDData.I);
      //ScreenChartAddData(LatestIT2704Data.V, LatestIT2704Data.I);
      
      // Show data on screen 1
      Screen1AddVIData(LatestRotoPDData.V, LatestRotoPDData.I);

      dataindexer = DATASTART;

      // Send the RotoPD data
      w_data.Val  = LatestRotoPDData.V;
      for ( j=0; j<2; j++ ) {hid_report_in[dataindexer++] = w_data.v[j];}
      w_data.Val  = LatestRotoPDData.I;
      for ( j=0; j<2; j++ ) {hid_report_in[dataindexer++] = w_data.v[j];}
      dw_data.Val = LatestRotoPDData.P;
      for ( j=0; j<4; j++ ) {hid_report_in[dataindexer++] = dw_data.v[j];}
      w_data.Val  = LatestRotoPDData.T;
      for ( j=0; j<2; j++ ) {hid_report_in[dataindexer++] = w_data.v[j];}

      if (load.isConnected())
      {
        // Send the IT2704 data
        w_data.Val  = LatestIT2704Data.V;
        for ( j=0; j<2; j++ ) {hid_report_in[dataindexer++] = w_data.v[j];}
        w_data.Val  = LatestIT2704Data.I;
        for ( j=0; j<2; j++ ) {hid_report_in[dataindexer++] = w_data.v[j];}
        dw_data.Val = LatestIT2704Data.P;
        for ( j=0; j<4; j++ ) {hid_report_in[dataindexer++] = dw_data.v[j];}
        dw_data.Val = LatestIT2704Data.C;
        for ( j=0; j<4; j++ ) {hid_report_in[dataindexer++] = dw_data.v[j];}
        dw_data.Val = LatestIT2704Data.E;
        for ( j=0; j<4; j++ ) {hid_report_in[dataindexer++] = dw_data.v[j];}
      }

      hid_report_in[LENGTHPOSITION]=dataindexer;

      // Send measurement data to host
      // This is an automated and autonomous process
      // Akes it easy to use a single controller for testing through a PC
      #ifdef ENABLEUSB
      #ifdef ARDUINO_ESP32S3_DEV
      HID.SendReport(0, hid_report_in, HID_INT_IN_EP_SIZE);
      #else
      HID.sendReport(0, hid_report_in, HID_INT_IN_EP_SIZE);
      #endif      
      #endif

      if (SET->TestData.Active == bmActive)
      {
        // Battery is active. Slowdown the data acquisition to get accurate data into a small datastore
        SET->TestData.DataTriggerCounter = (DATACOLLECTTIMENORMAL / DATACOLLECTTIMEFAST);
      }
    }
  }
  #endif

  if (CalcBatteryData)
  {
    CalcBatteryData = false;

    dword dcalc;
    qword qcalc;

    if (SET->TestData.SetStageMode != smOff)
    {
      // CALCULATIONTIME = 100, so every tick [increase] is 100ms
      if (SET->TestData.Active != bmReady)
      {
        //RDS->Time++;
        RDS->Time += CALCULATIONTIME;

        if (LatestRotoPDData.I != 0)
        {
          // Capacity calculations
          qcalc = LatestRotoPDData.I * 1000ULL;
          // qcalc is now uA
          qcalc *= (CALCULATIONTIME);
          RDS->Capacity += (qcalc / (3600ULL)); // this is nAh !!      

          if (LatestRotoPDData.V != 0)
          {
            // Energy calculations
            dcalc = LatestRotoPDData.V;
            // dcalc is now mV
            qcalc *= dcalc; // this is now mV * nAs = pWs
            qcalc /= (1000ULL); // this is nWs !!            
            RDS->Energy += (qcalc / 3600ULL); // this is nWh !!      
          }
        }
      } 
    }

    Screen1AddEPData((RDS->Energy / 1000000),(LatestRotoPDData.P));
    Screen1AddTData(RDS->Time);

    //ScreenPowerAddEPData((RDS->LastIT2704Data.E / 1000),(RDS->LastIT2704Data.P / 1000));
  }

  check_screensaver_timeout();

  lv_timer_periodic_handler();

  #ifdef STANDALONE
  if (StoreSettings)
  {
    StoreSettings = false;
    #ifdef DEBUG
    Serial.println("Storing setting in NVM !");
    #endif
    // Must disable interrupts, as interrupt handlers may be in flash, and we
    // are about to temporarily disable flash access!
    //uint32_t flags = save_and_disable_interrupts();    
    storeSettings(ActiveBatteryIndex);
    //restore_interrupts(flags);
  }   
  if (StoreBoardInfo)
  {
    StoreBoardInfo = false;
    #ifdef DEBUG
    Serial.println("Storing boardinfo in NVM !");
    #endif
    CreateSaveScreen("Saving board settings");
    lv_timer_handler();
    delay(500);
    lv_screen_brightness(0);
    delay(200);
    storePutBoardInfo(&BoardInfo);
    delay(200);
    lv_screen_load_anim(screenbase, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
    //lv_obj_delete();
    lv_timer_handler();
    lv_screen_brightness(100);
  }   
  #endif
}

unsigned long TicksBetween(unsigned long InitTicks, unsigned long EndTicks)
{
  unsigned long Result;
  Result = (EndTicks - InitTicks);
  if ((long)(~Result) < Result) Result = (long)(~Result);
  return (Result);
}

void ClearRunData(PRunDatas RDS)
{
  RDS->CurrentStageNumber = 0;
  RDS->Capacity = 0;
  RDS->Energy = 0;
  RDS->Time = 0;

  for(byte i=tmNONE; i<tmLast; i++)
  {
    RDS->ThresholdResult[i].Enabled = false;
    RDS->ThresholdResult[i].Triggered = false;      
    RDS->ThresholdResult[i].Mode = tmNONE;
    RDS->ThresholdResult[i].SetValue = 0;
    RDS->ThresholdResult[i].GetValue = 0;      
  }

  RDS->NewerRotoPDDatas.clear();  
  RDS->NewerIT2704Datas.clear();  
}

void ClearStageData(PStageData SD)
{
  SD->Status = smOff;
  SD->SetValue = 0;
  for(byte i=tmNONE; i<tmLast; i++)
  {
    SD->ThresholdSettings[i].Enabled = false;
    SD->ThresholdSettings[i].Triggered = false;      
    SD->ThresholdSettings[i].Mode = tmNONE;
    SD->ThresholdSettings[i].SetValue = 0;
    SD->ThresholdSettings[i].GetValue = 0;      
  }
}

/*
dword GetMaxVData(PRunDatas RDS)
{
  dword tempvcalc = 0;

  if (RDS->Head != -1)
  {
    word i,j,k;
    qword tempvcalcsum;
    int start,stop,runner;
    PMeasurementData BD;

    // We measure every DATACOLLECTTIMENORMAL ms
    // We need DVTIME ms of data
    #define DVTIMESIZE  (DVTIME / DATACOLLECTTIMENORMAL) 

    stop = (RDS->Head + DATASIZE);
    if (RDS->Tail == -1) start = DATASIZE; start = (RDS->Tail + DATASIZE);

    if (start>stop) stop += DATASIZE;
    if ((stop-start)>=DVTIMESIZE)  start = (stop - DVTIMESIZE);

    tempvcalcsum = 0;
    k = 0;
    for(runner = start; runner <= stop; runner++)
    {
      j = runner % DATASIZE;
      BD = &RDS->BatteryDatas[j];

      if (BD->V > tempvcalc) tempvcalc = BD->V; 
      //tempvcalc = BD->V * MAXVOLTAGE;
      //tempvcalc /= (dword)((1u << BITS)-1u);

      k++;
      tempvcalcsum += BD->V;
    }
  }
  return (tempvcalc);
}
*/
