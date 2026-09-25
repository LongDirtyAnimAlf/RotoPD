#define RP2350_PSRAM_CS 47
#define LVGL_TICK_PERIOD_MS 5

//#include "waveshare_rp2350_touch_lcd_4.h"

#include <Arduino.h>

#include "hardware/vreg.h"
#include "hardware/powman.h"
#include "hardware/structs/qmi.h"
#include "hardware/sync.h"
#include "hardware/clocks.h"

#include <lvgl.h>
#include "pico/stdlib.h"
#include "./src/bsp/bsp_i2c.h"
#include "./src/bsp/bsp_st7701.h"
#include "./src/bsp/bsp_buzzer.h"

#include "ui.h"

//#include <WiFi.h>

#include "extras.h"
#include "shared.h"
#include "comms.h"

#include <Ticker.h>

#define HOR_RES 480
#define VER_RES 480

// Default placeholder due to re-use of existing software
#define ActiveBatteryIndex 0

// Must be a global variable !!!
char mySerial[30];
char myFirmware[30];

AP33772S pd((TwoWire*)&WireBattery);
INA238 ina238(INA238_ADDRESS,(TwoWire*)&WireBattery);

TBatteryBoard BatteryBoards[DAUGHTERBOARDCOUNT] = {0};
static TBatterySetting Batteries[DAUGHTERBOARDCOUNT]; // Battery data settings and results

static volatile bool CalcBatteryData = false;
static Ticker dataupdateticker;

static Ticker datagetticker;
static volatile bool GetData = false;

#ifdef STANDALONE
GT911_Lite tp; // touchscreen through TwoWire
static Ticker datacollectticker;
static Ticker datastartticker;
static volatile bool GetBatteryData = false;
static volatile bool StoreSettings = false;
static volatile byte SendCommand[COMMAND_SIZE] = {0};
#endif

void ClearRunData(PRunDatas RDS);
void ClearStageData(PStageData SD);

dword GetMaxVData(PRunDatas RDS);

// ---------------------------------------------------------------
// Automatic PSRAM (QMI M1) timing calculator for RP2350
// Tuned for common APS6404 / similar 133–166 MHz PSRAM chips
// ---------------------------------------------------------------
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

// Callback that returns elapsed ms since boot
static uint32_t my_tick_get_cb(void) {
  return millis();
}

#ifdef STANDALONE
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
#endif

static void main_event_handler(lv_event_t * e)
{
  static byte screenindex = 1;

  bool GotSettings = false;

  PBatterySetting SET = NULL;
  PRunDatas RDS = NULL;
  PStageData SD = NULL;  

  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * event_user_data = (lv_obj_t *)lv_event_get_user_data(e);
  lv_obj_t * event_object = (lv_obj_t *)lv_event_get_target(e);
  if (event_object == NULL) event_object = lv_event_get_current_target_obj(e);

  lv_obj_t * object_user_data = NULL;

  if (event_object != NULL)
  {
    object_user_data = (lv_obj_t *)lv_obj_get_user_data(event_object);

    SET = &Batteries[ActiveBatteryIndex];
    RDS = &SET->TestData.RunDatas;

    #ifdef STANDALONE
    if ( (lv_obj_check_type(event_object, &lv_button_class)) || (lv_obj_check_type(event_object, &lv_list_button_class)) )
    {
      if(code == LV_EVENT_VALUE_CHANGED)
      {
        //bool buttondown = (lv_obj_get_state(btn, LV_BTN_PART_MAIN) & LV_STATE_CHECKED);
        bool buttondown = (lv_obj_get_state(event_object) & LV_STATE_CHECKED);

        if (event_object == outputbutton)
        {
          // Prepare the command to engage the hardware
          SendCommand[COMMANDPOSITION] = CMD_set_output;
          SendCommand[INDEXPOSITION] = BoardInfo.BoardNumber;
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
              SET->TestData.SetStageMode = smCurrent;
            }
            if ((event_object == testchargebutton) || (event_object == startchargebutton))
            {
              SD = &SET->Stages[FIXEDCHARGESTAGENUMBER];
              SET->TestData.SetStageMode = smCharge;
            }
            SET->TestData.SetStageValue = SD->SetValue;              
            if ((event_object == startdischargebutton) || (event_object == startchargebutton))          
            {
              // Clear rundatas and threshold before starting with official (dis)charge !
              ClearRunData(RDS);
              // Set trigger indicators
              for(i = (tmNONE+1); i < tmLast; i++)
              {
                Screen1SetThresholdLedEnabled((TThresholdModes)i, SD->ThresholdSettings[i].Enabled);
              }
              // We are active !!
              SET->TestData.Active = bmActive;
              // Force a very early data measurement to get a start value
              GetBatteryData = true;

              // Engage after some time !!!!
              // This is needed to dismiss the first few measurements when starting
              //if (SET->TestData.SetStageMode == smCharge) datastartticker.once_ms(1000U, datastartcb, (byte)ActiveBatteryIndex);                  
              //if (SET->TestData.SetStageMode == smCurrent) datastartticker.once_ms(1000U, datastartcb, (byte)ActiveBatteryIndex);                  
            }
          }

          // Prepare the command to engage the hardware
          SendCommand[COMMANDPOSITION] = CMD_set_value;
          SendCommand[INDEXPOSITION] = BoardInfo.BoardNumber;
          SendCommand[LENGTHPOSITION] = 5U; // length
          SendCommand[DATASTART] = (byte)SET->TestData.SetStageMode;
          temp = SET->TestData.SetStageValue;
          SendCommand[DATASTART+1] = (temp % 256);
          temp /= 256;
          SendCommand[DATASTART+2] = (temp % 256);
          temp /= 256;
          SendCommand[DATASTART+3] = (temp % 256);
          temp /= 256;
          SendCommand[DATASTART+4] = (temp % 256);
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
        // Screen navigation
        if ((event_object == backbutton) || (event_object == morebutton))
        {
          if ( (event_object == backbutton) && (screenindex>1) ) screenindex--; // back button
          #ifndef STANDALONE
          if ( (event_object == morebutton) && (screenindex<2) ) screenindex++; // forwards button
          #else
          if ( (event_object == morebutton) && (screenindex<4) ) screenindex++; // forwards button              
          #endif

          switch(screenindex)
          {
            case 1: {Setup_Screen1(ActiveBatteryIndex);Screen1SetData(SET);break;}
            case 2: {Setup_Screen2(ActiveBatteryIndex);Screen2SetData(RDS);break;}
            #ifdef STANDALONE
            case 3: {Setup_Screen3(ActiveBatteryIndex,true);break;}
            case 4: {Setup_ScreenLogger(ActiveBatteryIndex,true);break;}
            #endif
          }
        }
        else
        // Zero buttons
        if ((event_object == zerocapacitybutton) || (event_object == zeroenergybutton) || (event_object == zerotimebutton))
        {
          #ifdef DEBUG                      
          Serial.println("Zero button pressed");
          #endif
          if (event_object == zerocapacitybutton) RDS->Capacity = 0;
          if (event_object == zeroenergybutton)   RDS->Energy = 0;
          if (event_object == zerotimebutton)     RDS->Time = 0;          
          Screen1AddEPData(0,0);        
        }
        else
        // PDO list requested
        if (event_object == getpdolistbutton)
        {
          #ifdef DEBUG                      
          Serial.println("Request PDO list");
          #endif
          // Prepare the command to engage the hardware
          SendCommand[COMMANDPOSITION]   = CMD_get_PDOList;
          SendCommand[INDEXPOSITION]     = BoardInfo.BoardNumber;
          SendCommand[LENGTHPOSITION]    = 0U; // length
        }
        else
        {
          #ifdef DEBUG                      
          Serial.println("Unknown button pressed");
          #endif
          if (event_user_data == screen3)
          {
            #ifdef DEBUG                      
            //Serial.println("Button from screen 3");
            #endif
            if (object_user_data != NULL)
            {
              // WE got a PDO select click !!
              byte SelectPDOindex = ((byte)(uintptr_t)object_user_data);      
              #ifdef DEBUG
              Serial.printf("PDO button %d pressed.\r\n", SelectPDOindex);
              #endif
              bsp_buzzer_enable(true);
              // Prepare the command to engage the hardware
              SendCommand[COMMANDPOSITION]   = CMD_set_MAXPDO;
              SendCommand[INDEXPOSITION]     = BoardInfo.BoardNumber;
              SendCommand[LENGTHPOSITION]    = 1U; // length
              SendCommand[DATASTART]         = SelectPDOindex;
            }
          }
        }

      }
    }

    if ( (lv_obj_check_type(event_object, &lv_keyboard_class)) || (lv_obj_check_type(event_object, &lv_checkbox_class)) )
    {

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
      Serial.println("Perpare storing settings in NVM !");
      #endif
      StoreSettings = true;
    }   
    #endif
  }
}

void AddMeasurementData(byte index, word V, word I, dword P, word T)
{
  static bool GoAround[DAUGHTERBOARDCOUNT] = {false};

  if (index<DAUGHTERBOARDCOUNT)
  {
    PRunDatas RDS  = &Batteries[index].TestData.RunDatas;

    // Reset GoAround in needed
    if ((RDS->Head == -1) && (RDS->Tail == -1)) GoAround[index] = false;

    if (GoAround[index])
    {
      RDS->Tail++;
      if (RDS->Tail >= DATASIZE) RDS->Tail = 0;
    }

    RDS->Head++;
    if (RDS->Head >= DATASIZE)
    {
      RDS->Head = 0;
      if (!GoAround[index]) RDS->Tail = 1; // Preset tail to last added value
      GoAround[index] = true;
    }

    //RDS->Temperature = T;    

    PMeasurementData MD = &RDS->BatteryDatas[RDS->Head];

    MD->V = V;
    MD->I = I;
    MD->P = P;
    MD->T = T;    
  }
}

#ifdef STANDALONE
static void datagetcb()
{
  GetData = true;
}

static void datacollectcb()
{
  GetBatteryData = true;
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
  GetBatteryData = true;
}

#endif

void dataupdatecb()
{
  // Inform the loop to collect the battery data
  CalcBatteryData = true;
}


void setup()
{
  byte index;
  char myHex[10] = "";

  //vreg_set_voltage(VREG_VOLTAGE_1_20);   // or 1.25 / 1.30
  //sleep_ms(5);

  set_sys_clock_khz(266000, true);  
  //sleep_ms(5);

  WireBattery.setSDA(BSP_I2C_SDA_PIN);
  WireBattery.setSCL(BSP_I2C_SCL_PIN);
  WireBattery.begin();
  //WireBattery.begin(BSP_I2C_NUM,BSP_I2C_SDA_PIN,BSP_I2C_SCL_PIN);

  //if (set_sys_clock_khz(266000, true)) {
  //      // Clock configured successfully
  //}

  Serial.begin();
  int cnt = 1500;     // Will wait for up to ~5 second for Serial to connect.
  while (!Serial && cnt--) {delay(1);}
  Serial.println("Starting RP2350 init.");

  bsp_buzzer_init();
  //bsp_buzzer_enable(true);

  Info_Add("GUI. Init our LVGL display wonder.");    
  lv_init();
  lv_screen_init(HOR_RES, VER_RES);
  lv_tick_set_cb(my_tick_get_cb);  // Tell LVGL how to get the current time

  PBatterySetting SET;

  #ifndef LVGLDEMOS
  CreateBaseScreen(main_event_handler);
  lv_screen_load(screenbase);
  Setup_ScreenLogger(ActiveBatteryIndex,false);
  Info_Add("GUI. Init GUI.");      
  Setup_Screen3(ActiveBatteryIndex,false);
  Setup_Screen1(ActiveBatteryIndex);
  SET = &Batteries[ActiveBatteryIndex];
  Screen1SetData(SET);
  #endif

  Info_Add("GUI. Controller startup");
  
  //WiFi.mode(WIFI_OFF);

  PRunDatas RDS;
  PStageData SD;  

  // Get memory for datastore
  // Set some defaults
  for(index = 0; index < DAUGHTERBOARDCOUNT; index++)
  {
    SET = &Batteries[index];

    RDS = &SET->TestData.RunDatas;
    RDS->BatteryDatas = (TMeasurementData*)pmalloc(DATASIZE * sizeof(TMeasurementData));
    ClearRunData(RDS);
  
    SET->TestData.Active = bmIdle;
    SET->TestData.SetStageMode = smOff;
    SET->TestData.SetStageValue = 0;
    SET->TestData.DataTriggerCounter = 0;
  }

  #ifdef STANDALONE

  SendCommand[COMMANDPOSITION] = CMD_unknown;

  #endif
  
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
  datagetticker.attach_ms(DATAGETTIME, datagetcb);
  datacollectticker.attach_ms(DATACOLLECTTIMEFAST, datacollectcb);

  // Init touch device
  Info_Add("GUI. Init touch screen.");      

  //tp.begin(&Wire1);
  tp.begin(&Wire1,10,480,480);
  tp.setRotate180(true,480,480);

  /*Initialize the input device driver*/
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);
  #endif
  
  dataupdateticker.attach_ms(CALCULATIONTIME, dataupdatecb);  

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

  bsp_buzzer_enable(false);

  set_psram_timing_auto();

  Info_Add("GUI. Init RP2350 ready.");
}

void loop()
{
  #ifdef LVGLDEMOS
  uint32_t task_delay_ms = lv_timer_handler_run_in_period(5);
  #else

  uint8_t j;

  static unsigned long startTime = millis();

  bool DataOk = false;
  byte INData[COMMAND_SIZE] = {0};
  byte hid_report_in[HID_INT_IN_EP_SIZE] = {0};  
  byte dataindexer = 0;

  PDO_DATA_T raw;
  WORD_VAL wv;
  DWORD_VAL dwv;

  int8_t PDOCount = 0;
  AP33772S_PDO PDO;

  if (millis() - startTime >= 1000)
  {
    startTime = millis();

    Serial.println("Loop");

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

        #ifdef STANDALONE
        // Already done in setup
        //Setup_Screen3(ActiveBatteryIndex,false);
        #endif
        
        Screen3ClearPDOList();

        memset(&hid_report_in, 0, HID_INT_IN_EP_SIZE);

        hid_report_in[COMMANDPOSITION] = CMD_get_PDOList;
        hid_report_in[INDEXPOSITION] = BoardInfo.BoardNumber;

        dataindexer = DATASTART;

        hid_report_in[dataindexer++] = PDOCount;

        for ( j=1; j<=MAX_PDO_ENTRIES; j++ )
        {
          if (pd.readPDO(j, PDO))
          {
            if (PDO.valid)
            {
              if (PDO.isEPR)
                Info_Add_Fmt("GUI. EPR PDO received ! PDO voltage: %dmV.", PDO.maxVoltage_mV);
              else
                Info_Add_Fmt("GUI. PDO received ! PDO voltage: %dmV.", PDO.maxVoltage_mV);
              #ifdef STANDALONE
              Screen3SetPDO(PDO.index,PDO.valid,PDO.isEPR,PDO.type,PDO.minVoltage_mV,PDO.maxVoltage_mV,PDO.maxCurrent_mA);
              #endif
              hid_report_in[dataindexer++] = PDO.index;
              wv.Val = PDO.raw;
              hid_report_in[dataindexer++] = wv.bytes.LB;
              hid_report_in[dataindexer++] = wv.bytes.HB;
            }  
          }
        }
        // Send PDO data
        hid_report_in[LENGTHPOSITION]=dataindexer;        
      }
    }
  }  

  PBatterySetting SET = NULL;
  PRunDatas RDS = NULL;

  if (GetData)
  {
    GetData = false;
    collectRotoPDData();
  }

  THIDData* PLocalHD;
  THIDData LocalHDCopy;

  #ifdef STANDALONE

  PBatteryBoard BB = NULL;

  byte OUTData[COMMAND_SIZE] = {0};

  // Do we have a valid command ?
  if ( (SendCommand[COMMANDPOSITION] != CMD_unknown) && (SendCommand[COMMANDPOSITION] != USB_CMD_error) )
  {
    // Fill the data
    for (j=0; j<(SendCommand[LENGTHPOSITION]+DATASTART); j++) OUTData[j] = SendCommand[j];
    // Reset command
    SendCommand[COMMANDPOSITION] = CMD_unknown;

    DataOk = process_command(&OUTData,&INData);
  }

  #endif //STANDALONE

  if (DataOk)
  {
    CommandType_t cCmd = (CommandType_t)INData[COMMANDPOSITION];
    byte BoardNumber = INData[INDEXPOSITION];
    byte Length = INData[LENGTHPOSITION];
    byte counter = DATASTART;

    switch(cCmd)
    {
      case CMD_set_energy:
      case CMD_set_capacity:
      case CMD_set_time:
      {
        QWORD_VAL qw;

        qw.Val = 0;

        SET = &Batteries[ActiveBatteryIndex];
        RDS = &SET->TestData.RunDatas; 

        for ( j=0; j<Length; j++ ) {qw.v[j] = INData[counter++];}

        Info_Add_Fmt("GUI. Setdata [%d] received ! %d.", Length, qw.Val);

        if (cCmd == CMD_set_energy) RDS->Energy = qw.Val; // in nAh
        if (cCmd == CMD_set_capacity) RDS->Capacity = qw.Val; // in nWh
        if (cCmd == CMD_set_time) RDS->Time = qw.Val;  // in deci-seconds = 100ms

        if (cCmd == CMD_set_energy) Screen1AddEData(RDS->Energy / 1000000ULL);
        if (cCmd == CMD_set_time) Screen1AddTData(RDS->Time);

        break;
      }

      case CMD_set_MAXPDO:
      case CMD_set_FIXEDPDO:
      case CMD_set_PPSPDO:
      case CMD_set_AVSPDO:
      {
        j = INData[counter++];

        wv.bytes.LB = INData[counter++];
        wv.bytes.HB = INData[counter++];
        PDO.maxCurrent_mA = wv.Val;
        Info_Add_Fmt("GUI. PDO requested current: %dmA.", PDO.maxCurrent_mA);

        wv.bytes.LB = INData[counter++];
        wv.bytes.HB = INData[counter++];
        PDO.maxVoltage_mV = wv.Val;
        Info_Add_Fmt("GUI. PDO requested voltage: %dmV.", PDO.maxVoltage_mV);

        raw.byte0 = INData[counter++];
        raw.byte1 = INData[counter++];

        // This fuction is index zero based !!
        AP33772S::decodePDONew(j-1, raw, PDO);

        if (PDO.valid)
        {
          Info_Add_Fmt("GUI. PDO [%d] received ! PDO V/I: %dmV/%dmA.", j, PDO.maxVoltage_mV, PDO.maxCurrent_mA);
        }

        bsp_buzzer_enable(false);

        break;
      }

      case CMD_get_PDOList:
      case CMD_read_PDOList:
      {
        // We need to update the GUI with the received PDO's !!

        // Got PDO data from plugin of PD source
        Info_Add("GUI. PDO list request by user !!");

        Screen3ClearPDOList();

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

                Screen3SetPDO(PDO.index,PDO.valid,PDO.isEPR,PDO.type,PDO.minVoltage_mV,PDO.maxVoltage_mV,PDO.maxCurrent_mA);
              }
            }
          }

        }
        break;
      }
    }
  }

  #ifdef STANDALONE
  if (GetBatteryData)
  {
    GetBatteryData = false;

    /*
    sendObdFrame(5); // For coolant temperature
    // You can set custom timeout, default is 1000
    if(ESP32Can.readFrame(rxFrame, 100)) {
        // Comment out if too many frames
        Serial.printf("Received frame: %03X  \r\n", rxFrame.identifier);
        if(rxFrame.identifier == 0x7E8) {                                    // Standard OBD2 frame responce ID
            Serial.printf("Collant temp: %3d°C \r\n", rxFrame.data[3] - 40); // Convert to °C
        }
    }
    */

    #ifdef DEBUG
    //Serial.println("Getting data");
    #endif

    SET = &Batteries[ActiveBatteryIndex];

    switch(SET->TestData.Active)
    {
      case bmActive:
        // Battery is active. Slowdown the data acquisition to get accurate data into a small datastore
        if (SET->TestData.DataTriggerCounter > 0) SET->TestData.DataTriggerCounter--;
        break;
      case bmReady:
      case bmIdle:
        SET->TestData.DataTriggerCounter = 0;
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

      RDS = &SET->TestData.RunDatas;        

      getRotoPDData(&RDS->LastBatteryData.I,&RDS->LastBatteryData.V,&RDS->LastBatteryData.P,&RDS->LastBatteryData.T);        

      //Serial.println("Got RotoPD data");        
      
      // Show data on screen 1
      Screen1AddVIData(RDS->LastBatteryData.V, RDS->LastBatteryData.I);

      WORD_VAL  w_data;
      DWORD_VAL dw_data;

      memset(&hid_report_in, 0, HID_INT_IN_EP_SIZE);

      hid_report_in[COMMANDPOSITION] = CMD_get_data;
      hid_report_in[INDEXPOSITION] = BoardInfo.BoardNumber;

      dataindexer = DATASTART;

      w_data.Val = RDS->LastBatteryData.V;
      for ( j=0; j<2; j++ ) {hid_report_in[dataindexer++] = w_data.v[j];}
      w_data.Val = RDS->LastBatteryData.I;
      for ( j=0; j<2; j++ ) {hid_report_in[dataindexer++] = w_data.v[j];}
      dw_data.Val = RDS->LastBatteryData.P;
      for ( j=0; j<4; j++ ) {hid_report_in[dataindexer++] = dw_data.v[j];}
      w_data.Val = RDS->LastBatteryData.T;
      for ( j=0; j<2; j++ ) {hid_report_in[dataindexer++] = w_data.v[j];}

      hid_report_in[LENGTHPOSITION]=dataindexer;        
      
      if (SET->TestData.Active == bmActive)
      {
        //Append the data in storage
        AddMeasurementData(ActiveBatteryIndex, RDS->LastBatteryData.V, RDS->LastBatteryData.I, RDS->LastBatteryData.P, RDS->LastBatteryData.T);

        // Append data into graphs
        Screen2AddData(RDS->LastBatteryData.V, RDS->LastBatteryData.I);

        // Battery is active. Slowdown the data acquisition to get accurate data into a small datastore
        SET->TestData.DataTriggerCounter = (DATACOLLECTTIMENORMAL / DATACOLLECTTIMEFAST);
      }
    }
  }

  #endif //STANDALONE

  if (CalcBatteryData)
  {
    CalcBatteryData = false;

    dword dcalc;
    qword qcalc;

    SET = &Batteries[ActiveBatteryIndex];
    RDS = &SET->TestData.RunDatas;  

    if (SET->TestData.SetStageMode != smOff)
    {
      // CALCULATIONTIME = 100, so every tick [increase] is 100ms
      if (SET->TestData.Active != bmReady)
      {
        RDS->Time++;

        if (RDS->LastBatteryData.I != 0)
        {
          // Capacity calculations
          qcalc = RDS->LastBatteryData.I * 1000ULL;
          // qcalc is now uA
          qcalc *= (CALCULATIONTIME);
          RDS->Capacity += (qcalc / (3600ULL)); // this is nAh !!      

          if (RDS->LastBatteryData.V != 0)
          {
            // Energy calculations
            dcalc = RDS->LastBatteryData.V;
            // dcalc is now mV
            qcalc *= dcalc; // this is now mV * nAs = pWs
            qcalc /= (1000ULL); // this is nWs !!            
            RDS->Energy += (qcalc / 3600ULL); // this is nWh !!      
          }
        }
      } 
    }

    Screen1AddEPData((RDS->Energy / 1000000),(RDS->LastBatteryData.P));
    Screen1AddTData(RDS->Time);
  }

  uint32_t task_delay_ms = lv_timer_handler_run_in_period(5);
  //uint32_t task_delay_ms = lv_task_handler();
  //vTaskDelay( pdMS_TO_TICKS(task_delay_ms) );
  
  //vTaskDelayUntil( &xLastWakeTime, ( 5 / portTICK_PERIOD_MS ) );
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
  memset(RDS->BatteryDatas, 0, DATASIZE * sizeof(TMeasurementData));
  RDS->CurrentStageNumber = 0;
  RDS->Capacity = 0;
  RDS->Energy = 0;
  RDS->Time = 0;
  //RDS->Temperature = 0;
  RDS->LastBatteryData.V = 0;
  RDS->LastBatteryData.I = 0;
  RDS->LastBatteryData.P = 0;
  RDS->LastBatteryData.T = 0;
  RDS->Head = -1;
  RDS->Tail = -1;  

  for(byte i=tmNONE; i<tmLast; i++)
  {
    RDS->ThresholdResult[i].Enabled = false;
    RDS->ThresholdResult[i].Triggered = false;      
    RDS->ThresholdResult[i].Mode = tmNONE;
    RDS->ThresholdResult[i].SetValue = 0;
    RDS->ThresholdResult[i].GetValue = 0;      
  }
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
