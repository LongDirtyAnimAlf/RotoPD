/*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html
 Install: lvgl*/

// This define is sometimes missing when using old ESP32-IDF version
//#define ESP_INTR_CPU_AFFINITY_AUTO 0

#include <Arduino.h>
#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include <PacketSerial.h>
#include "Indicator_Extender.h"
#include "ui.h"
#include "touch.h"
#include "extras.h"
#include "shared.h"
//#include "esp_private/spi_flash_os.h"

//#define    LVGLDEMOS

#ifdef LVGLDEMOS
#ifdef STANDALONE
#undef STANDALONE 
#endif
/*To use the built-in examples and demos of LVGL uncomment the includes below respectively.
 *You also need to copy `lvgl/examples` to `lvgl/src/examples`. Similarly for the demos `lvgl/demos` to `lvgl/src/demos`. */
//#include <examples/lv_examples.h>
#include <demos/lv_demos.h>
#endif

#include <Ticker.h>

#define HOR_RES 480
#define VER_RES 480

#define PACKET_UART_RXD 20
#define PACKET_UART_TXD 19

#define BUTTON_PIN 38

#define DATACOLLECTTIMEFAST 1000 // ms
#define DATACOLLECTTIMENORMAL 10000 // ms
#define CALCULATIONTIME 100 // ms

#define GFX_DEV_DEVICE ESP32_S3_RGB
#define RGB_PANEL
#define GFX_BL 45
static Arduino_DataBus *bus = new Indicator_SWSPI(
    GFX_NOT_DEFINED /* DC */, EXPANDER_IO_LCD_CS /* CS */,
    SPI_SCLK /* SCK */, SPI_MOSI /* MOSI */, GFX_NOT_DEFINED /* MISO */);

// See: https://github.com/esp-arduino-libs/ESP32_Display_Panel/blob/master/src/drivers/lcd/port/esp_lcd_st7701.h#L81
static Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    18 /* DE */, 17 /* VSYNC */, 16 /* HSYNC */, 21 /* PCLK */,
    4 /* R0 */, 3 /* R1 */, 2 /* R2 */, 1 /* R3 */, 0 /* R4 */,
    10 /* G0 */, 9 /* G1 */, 8 /* G2 */, 7 /* G3 */, 6 /* G4 */, 5 /* G5 */,
    15 /* B0 */, 14 /* B1 */, 13 /* B2 */, 12 /* B3 */, 11 /* B4 */,
    #ifndef ALTERNATIVE
    1 /* hsync_polarity */, 20 /* hsync_front_porch */, 10 /* hsync_pulse_width */, 10 /* hsync_back_porch */,
    1 /* vsync_polarity */, 10 /* vsync_front_porch */, 10 /* vsync_pulse_width */, 10 /* vsync_back_porch */,
    #else
    1 /* hsync_polarity */, 10 /* hsync_front_porch */, 8 /* hsync_pulse_width */, 50 /* hsync_back_porch */,
    1 /* vsync_polarity */, 10 /* vsync_front_porch */, 8 /* vsync_pulse_width */, 20 /* vsync_back_porch */,
    #endif
    1 /* pclk_active_neg */, 18000000 /* prefer_speed */, false /* useBigEndian  */,
    0 /* de_idle_high */, 0 /* pclk_idle_high */, 480 * 20 /* bounce_buffer_size_px */);    

static Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    HOR_RES /* width */, VER_RES /* height */, rgbpanel, 0 /* rotation */, false /* auto_flush */,
    bus, GFX_NOT_DEFINED /* RST */, st7701_indicator_init_operations, sizeof(st7701_indicator_init_operations));

static TBatterySetting Batteries[DAUGHTERBOARDCOUNT]; // Battery data settings and results
static volatile byte DRAM_ATTR ActiveBatteryIndex = 0;

static volatile bool CalcBatteryData = false;
static Ticker dataupdateticker;

#ifdef STANDALONE
static DRAM_ATTR TStageData StageDataTransporter[3];
static Ticker datacollectticker;
static Ticker datastartticker;
static volatile bool GetBatteryData = false;
static volatile bool StoreSettings = false;
static volatile byte SendCommand[COMMAND_SIZE] = {0};
#endif


static COBSPacketSerial myPacketSerial;
//PacketSerial_<COBS, 0, 1024> myPacketSerial;

void onPacketReceived(const uint8_t* buffer, size_t size);
void ClearRunData(PRunDatas RDS);
void ClearStageData(PStageData SD);

dword GetMaxVData(PRunDatas RDS);

/*Read the touchpad*/
//IRAM_ATTR
static void IRAM_ATTR my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
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

#ifdef STANDALONE
static void IRAM_ATTR storeSettings(byte BI)
{
  // At the moment 13-March-2025, this only works with ESP IDF 3.1.1 !!!
  // So, be carefull
  // See: https://github.com/moononournation/Arduino_GFX/issues/638

  // ESP IDF version > 3.2.0 have these enabled
  // CONFIG_GDMA_ISR_IRAM_SAFE=y
  // CONFIG_LCD_RGB_ISR_IRAM_SAFE=y
  // CONFIG_LCD_RGB_RESTART_IN_VSYNC=y

  //(CONFIG_LCD_RGB_ISR_IRAM_SAFE && !(CONFIG_SPIRAM_RODATA && CONFIG_SPIRAM_FETCH_INSTRUCTIONS))

  // See: https://docs.espressif.com/projects/esp-faq/en/latest/software-framework/peripherals/lcd.html#why-do-i-get-drift-overall-drift-of-the-display-when-esp32-s3-is-driving-an-rgb-lcd-screen

  static DRAM_ATTR uint8_t v = 0;
  static DRAM_ATTR uint16_t w = 0;
  static DRAM_ATTR char stage[] = "#stage##";
  static DRAM_ATTR esp_err_t ret = 0;
  static DRAM_ATTR size_t length = sizeof(TStageData);

    // disable flash cache
    //spi_flash_guard_get()->start();
    //esp_rom_delay_us(200000);

    //lvgl_port_lock(-1);

    PBatterySetting SET = &Batteries[BI];

    v = BI;
    stage[7] = '0'+ (uint8_t)(v%10);
    v /= 10;
    stage[6] = '0'+ (uint8_t)(v%10);

    stage[0] = 'd';    
    ret = indicator_nvs_write(stage, (void *)&StageDataTransporter[FIXEDDISCHARGESTAGENUMBER], length);
    //ret = indicator_nvs_write(stage, (void *)&SET->Stages[FIXEDDISCHARGESTAGENUMBER], sizeof(TStageData));

    stage[0] = 'c';
    ret = indicator_nvs_write(stage, (void *)&StageDataTransporter[FIXEDCHARGESTAGENUMBER], length);
    //ret = indicator_nvs_write(stage, (void *)&SET->Stages[FIXEDCHARGESTAGENUMBER], sizeof(TStageData));

    #ifdef DEBUG  
    //if( ret != ESP_OK ) Serial.println("NVM error !"); else Serial.println("NVM ok.");
    #endif

    // enable flash cache
    //spi_flash_guard_get()->end();

    //lvgl_port_unlock();    
}
#endif

//IRAM_ATTR
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
        #ifdef DEBUG                      
        Serial.println("Event: button value changed");
        #endif

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

          //bool buttondown = (lv_obj_get_state(btn, LV_BTN_PART_MAIN) & LV_STATE_CHECKED);
          bool buttondown = (lv_obj_get_state(event_object) & LV_STATE_CHECKED);

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
          SendCommand[INDEXPOSITION] = ActiveBatteryIndex;
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
      }
      else
      if(code == LV_EVENT_LONG_PRESSED)
      {
        #ifdef DEBUG                      
        Serial.println("Event: long pressed");
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
          if ( (event_object == morebutton) && (screenindex<3) ) screenindex++; // forwards button              
          #endif
          switch(screenindex)
          {
            case 1: {Setup_Screen1(ActiveBatteryIndex);Screen1SetData(SET);break;}
            case 2: {Setup_Screen2(ActiveBatteryIndex);Screen2SetData(RDS);break;}
            #ifdef STANDALONE
            case 3: {Setup_Screen3(ActiveBatteryIndex);break;}
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
          SendCommand[INDEXPOSITION]     = ActiveBatteryIndex;
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
              // Prepare the command to engage the hardware
              SendCommand[COMMANDPOSITION]   = CMD_set_MAXPDO;
              SendCommand[INDEXPOSITION]     = ActiveBatteryIndex;
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
    #endif STANDALONE


    #ifdef STANDALONE
    if (GotSettings)
    {
      GotSettings = false;      
      #ifdef DEBUG
      Serial.println("Perpare storing settings in NVM !");
      #endif
      StageDataTransporter[IDDLESTAGENUMBER] = SET->Stages[IDDLESTAGENUMBER];
      StageDataTransporter[FIXEDDISCHARGESTAGENUMBER] = SET->Stages[FIXEDDISCHARGESTAGENUMBER];
      StageDataTransporter[FIXEDCHARGESTAGENUMBER] = SET->Stages[FIXEDCHARGESTAGENUMBER];
      // Signal main loop to store data in flash
      // This methods prevents screen quirks
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

  #ifdef DEBUG
  Serial.begin(115200);
  int cnt = 5000;     // Will wait for up to ~1 second for Serial to connect.
  while (!Serial && cnt--) {delay(1);}
  // Serial.setDebugOutput(true);
  Serial.println("SenseCap Indicator startup");
  #endif

  PBatterySetting SET;
  PRunDatas RDS;
  PStageData SD;  

  // Get memory for datastore
  // Set some defaults
  for(index = 0; index < DAUGHTERBOARDCOUNT; index++)
  {
    SET = &Batteries[index];

    RDS = &SET->TestData.RunDatas;
    RDS->BatteryDatas = (TMeasurementData*)malloc(DATASIZE * sizeof(TMeasurementData));
    ClearRunData(RDS);
  
    SET->TestData.Active = bmIdle;
    SET->TestData.SetStageMode = smOff;
    SET->TestData.SetStageValue = 0;
    SET->TestData.DataTriggerCounter = 0;
  }

  #ifdef STANDALONE

  SendCommand[COMMANDPOSITION] = CMD_unknown;

  esp_err_t ret = 0;  

  ret = indicator_nvs_init();
  #ifdef DEBUG  
  if( ret != ESP_OK )
  {
    Serial.println("Partition init error !");
  }
  else
  {
    Serial.println("Partition init ok.");
  }
  #endif

  char stagetext[] = "#stage##";

  // Set and get defaults;
  for(index = 0; index < DAUGHTERBOARDCOUNT; index++)
  {
    SET = &Batteries[index];

    SD = &SET->Stages[IDDLESTAGENUMBER];
    ClearStageData(SD);
    SD = &SET->Stages[FIXEDDISCHARGESTAGENUMBER];
    ClearStageData(SD);
    SD = &SET->Stages[FIXEDCHARGESTAGENUMBER];
    ClearStageData(SD);

    SET->Stages[FIXEDDISCHARGESTAGENUMBER].ThresholdSettings[tmMINV].Enabled = true;
    SET->Stages[FIXEDDISCHARGESTAGENUMBER].ThresholdSettings[tmMINV].Mode = tmMINV;

    size_t length;

    stagetext[6] = '0'+(uint8_t)(index/10);
    stagetext[7] = '0'+(uint8_t)(index%10);

    stagetext[0] = 'd';
    length = sizeof(SET->Stages[FIXEDDISCHARGESTAGENUMBER]);    
    ret = indicator_nvs_read(stagetext, &SET->Stages[FIXEDDISCHARGESTAGENUMBER], &length);

    stagetext[0] = 'c';
    length = sizeof(SET->Stages[FIXEDCHARGESTAGENUMBER]);    
    ret = indicator_nvs_read(stagetext, &SET->Stages[FIXEDCHARGESTAGENUMBER], &length);
  }

  #ifdef DEBUG
  Serial.println("Reading stored presets done.");
  #endif

  if (false)
  {
    // Store defaults
    for(index = 0; index < DAUGHTERBOARDCOUNT; index++)
    {
      SET = &Batteries[index];

      stagetext[6] = '0'+(uint8_t)(index/10);
      stagetext[7] = '0'+(uint8_t)(index%10);
     
      // Default discharge
      SD = &SET->Stages[FIXEDDISCHARGESTAGENUMBER];
      SD->Status = smCurrent; // a CC discharge
      SD->SetValue = 250; // 500mA    
      SD->ThresholdSettings[tmMINV].SetValue = 900; //900mV end value
      SD->ThresholdSettings[tmMINV].Enabled = true;
      stagetext[0] = 'd';
      ret = indicator_nvs_write(stagetext, SD, sizeof(SET->Stages[FIXEDDISCHARGESTAGENUMBER]));

      // Default charge
      SD = &SET->Stages[FIXEDCHARGESTAGENUMBER];
      stagetext[0] = 'c';
      ret = indicator_nvs_write(stagetext, SD, sizeof(SET->Stages[FIXEDCHARGESTAGENUMBER]));
    }
    #ifdef DEBUG    
    Serial.println("Storing default presets done.");
    #endif
  }

  #endif

  // Init Indicator hardware

  pinMode(BUTTON_PIN, INPUT);

  #ifdef DEBUG  
  Serial.println("Init extender.");  
  #endif
  extender_init();

  myPacketSerial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, PACKET_UART_RXD, PACKET_UART_TXD);
  myPacketSerial.setStream(&Serial1);
  myPacketSerial.setPacketHandler(&onPacketReceived);

  // Init Display
  #ifdef DEBUG  
  Serial.println("Init display.");
  #endif
  if (!gfx->begin())
  {
    #ifdef DEBUG    
    Serial.println("gfx->begin() failed!");
    Serial.println("Expect sever errors !!!");    
    #endif
  }

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  #ifdef DEBUG
  String LVGL_Arduino = "Init LVGL " + String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);
  #endif
  lv_init();

  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb([](){ 
    //return (uint32_t) (esp_timer_get_time() / 1000LL);
    return (xTaskGetTickCount());    
    //return ((uint32_t)millis());        
  });

  #ifdef DEBUG
  Serial.println("Init our lvgl task and refresh.");    
  #endif
  lv_screen_init(gfx, HOR_RES, VER_RES);
  //lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_0);
  //lv_display_set_antialiasing(disp,false);

  // Init touch device
  Serial.println("Init touch screen.");      
  touch_init(HOR_RES, VER_RES, 0); // rotation will be handled by lvgl
  /*Initialize the input device driver*/
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);

  ActiveBatteryIndex = 0;

  #ifndef LVGLDEMOS
  #ifdef DEBUG
  Serial.println("Init GUI.");      
  #endif
  CreateBaseScreen(main_event_handler);
  lv_screen_load(screenbase);
  Setup_Screen1(ActiveBatteryIndex);
  //SET = &Batteries[ActiveBatteryIndex];
  //Screen1SetData(SET);
  #endif

  #ifdef DEBUG  
  Serial.println("Init timers.");      
  #endif

  #ifdef STANDALONE
  datacollectticker.attach_ms(DATACOLLECTTIMEFAST, datacollectcb);
  #endif
  dataupdateticker.attach_ms(CALCULATIONTIME, dataupdatecb);  
  
  #ifdef DEBUG
  Serial.println("Init done");
  #endif

  #ifdef LVGLDEMOS
  //lv_demo_widgets();  
  lv_demo_benchmark();    
  #endif
}

void loop()
{
  static TickType_t xLastWakeTime = xTaskGetTickCount();
  
  unsigned long startTime = millis();
  while (digitalRead(BUTTON_PIN) == LOW)
  {
    if (millis() - startTime >= 2000)
    {
      ESP.restart();
      //esp_restart();
    }
    vTaskDelay(pdMS_TO_TICKS(100));      
  }

  byte i;
  PBatterySetting SET = NULL;
  PRunDatas RDS = NULL;

  #ifdef STANDALONE

  PBatteryBoard BB = NULL;

  byte j;
  uint8_t data_buf[COMMAND_SIZE];

  // Do we have a valid command ?
  if ( (SendCommand[COMMANDPOSITION] != CMD_unknown) && (SendCommand[COMMANDPOSITION] != USB_CMD_error) )
  {
    // Fill the data
    for (i=0; i<(SendCommand[LENGTHPOSITION]+DATASTART); i++) data_buf[i] = SendCommand[i];
    // Reset command
    SendCommand[COMMANDPOSITION] = CMD_unknown; 
    // Send the data request
    myPacketSerial.send(data_buf, i);
  }

  if (GetBatteryData)
  {
    GetBatteryData = false;

    for (i=0; i<DAUGHTERBOARDCOUNT; i++ )
    {
      SET = &Batteries[i];

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
        j = 0;
        data_buf[j++] = CMD_get_data;
        data_buf[j++] = i;
        // Send the data request
        myPacketSerial.send(data_buf, j);
      }

      if (SET->TestData.DataTriggerCounter == 0)
      {
        if (SET->TestData.Active == bmActive)
        {
          // Battery is active. Slowdown the data acquisition to get accurate data into a small datastore
           SET->TestData.DataTriggerCounter = (DATACOLLECTTIMENORMAL / DATACOLLECTTIMEFAST);
        }
      }

    }
  }

  #endif STANDALONE

  if (CalcBatteryData)
  {
    CalcBatteryData = false;

    dword dcalc;
    qword qcalc;

    for (i=0; i<DAUGHTERBOARDCOUNT; i++ )
    {
      SET = &Batteries[i];
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

      if (ActiveBatteryIndex == i)
      {
        Screen1AddEPData((RDS->Energy / 1000000),(RDS->LastBatteryData.P));
        Screen1AddTData(RDS->Time);
      }
    }
  }

  uint32_t task_delay_ms = lv_timer_handler_run_in_period(5);
  //uint32_t task_delay_ms = lv_task_handler();
  //vTaskDelay( pdMS_TO_TICKS(task_delay_ms) );
  
  //vTaskDelayUntil( &xLastWakeTime, ( 5 / portTICK_PERIOD_MS ) );

  myPacketSerial.update();
  // Check for a receive buffer overflow (optional).
  if (myPacketSerial.overflow())
  {
    // Send an alert via a pin (e.g. make an overflow LED) or return a
    // user-defined packet to the sender.
  }

  #ifdef STANDALONE
  if (StoreSettings)
  {
    StoreSettings = false;
    #ifdef DEBUG
    Serial.println("Storing setting in NVM !");
    #endif
    storeSettings(ActiveBatteryIndex);
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

void onPacketReceived(const uint8_t* buffer, size_t size)
{
  if (size < 1) {
    return;
  }

  byte counter;
  PBatterySetting SET;  
  PRunDatas RDS;  
  PStageData SD = NULL;  
  AP33772S_PDO dec;
  PDO_DATA_T raw;
  uint8_t  index;

  CommandType_t Command = (CommandType_t)buffer[COMMANDPOSITION];
  byte BatteryIndex = buffer[INDEXPOSITION];
  byte Length = buffer[LENGTHPOSITION];

  counter = DATASTART;

  switch(Command)
  {
    case CMD_get_PDOList:
    {

      byte PDOCount = buffer[counter++];

      if (PDOCount)
      {
        while ((PDOCount--)>0)
        {
          memset(&dec, 0, sizeof(dec));      

          index = buffer[counter++];

          Serial.printf("PDO received ! PDO index: #%d.\r\n", index);

          if (index)
          {
            raw.byte0 = buffer[counter++];
            raw.byte1 = buffer[counter++];
            // This fuction is index zero based !!
            AP33772S::decodePDONew(index-1, raw, dec);

            if (dec.valid)
            {
              Serial.printf("PDO received ! PDO voltage : #%dmV.\r\n", dec.maxVoltage_mV);
              Screen3SetPDO(dec.index,dec.valid,dec.isEPR,dec.type,dec.minVoltage_mV,dec.maxVoltage_mV,dec.maxCurrent_mA);
            }
          }
        }

      }
      break;
    }
    case CMD_get_data:
    case CMD_set_value:
    {

      SET = &Batteries[BatteryIndex];
      RDS = &SET->TestData.RunDatas;      

      // Did we receive battery data ?
      if (Command == CMD_get_data)
      {
        dword dcalc;
        word Volt = 0;
        word MaxVolt = 0;
        word Amps = 0; 
        dword Power = 0; 
        word Temperature = 0; 

        // Get the data from the buffer.
        memcpy(&Volt, &buffer[counter], 2);
        counter += 2;
        memcpy(&Amps, &buffer[counter], 2);
        counter += 2;
        memcpy(&Power, &buffer[counter], 4);
        counter += 4;
        memcpy(&Temperature, &buffer[counter], 2);
        counter += 2;

        // Store the data
        RDS->LastBatteryData.V = Volt;
        RDS->LastBatteryData.I = Amps;    
        RDS->LastBatteryData.P = Power;    
        RDS->LastBatteryData.T = Temperature;    

        // Show data on screen 1
        Screen1AddVIData(Volt, Amps);
        
        if (SET->TestData.Active == bmActive)
        {
          //Append the data in storage
          AddMeasurementData(BatteryIndex, Volt, Amps, Power, Temperature);

          // Append data into graphs if visible
          if (ActiveBatteryIndex == BatteryIndex) Screen2AddData(Volt,Amps);
        }
      }

      if (Command == CMD_set_value)
      {
        lv_color_t c = LV_COLOR_MAKE(0,0,0); 

        // Get the StageMode
        SET->TestData.SetStageMode = (TStageMode)buffer[counter++];
        // Get the StageValue
        memcpy(&SET->TestData.SetStageValue, &buffer[counter], 4);
        counter += 4;

        switch (SET->TestData.SetStageMode)
        {
          case smCurrent:
          case smPower:
          case smResistor:
          {
            c = lv_palette_main(LV_PALETTE_DEEP_ORANGE);
            ClearRunData(RDS);
            SET->TestData.Active = bmActive;
            break;
          }
          case smCharge:
          {
            c = lv_palette_main(LV_PALETTE_LIGHT_GREEN);
            ClearRunData(RDS);
            SET->TestData.Active = bmActive;
            break;
          }  
          case smOff:
          case smZero:
          case smDisabled:
          {
            c = LV_COLOR_MAKE(0X00,0xFF,0xFF);
            SET->TestData.Active = bmIdle;
            break;
          }
          default:
          {
            c = lv_palette_main(LV_PALETTE_YELLOW);
            SET->TestData.Active = bmIdle;
            break;
          }
        }
        //SetLedScreen1(BatteryIndex,c);
        //if (ActiveBatteryIndex == BatteryIndex) Screen1SetData(SET);      
      }
      break;
    }
  }
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

