/*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html
 Install: lvgl*/

// This define is sometimes missing when using old ESP32-IDF version
//#define ESP_INTR_CPU_AFFINITY_AUTO 0

#include <Arduino.h>
#include <lvgl.h>
#include <Arduino_GFX_Library.h>
//#include <SPI.h>
//#include "HWCDC.h"
#include "./src/CH32/WS_CH32_IO.h"
#include "ui.h"

//#include <WiFi.h>

#include "USB.h"
#include "USBHID.h"

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

#define CAN_TX GPIO_NUM_6 // Transmit GPIO number for CAN
#define CAN_RX GPIO_NUM_0 // Receive GPIO number for CAN

#define BUTTON_PIN 38

#define DATAGETTIME 50 // ms
#define DATACOLLECTTIMEFAST 500 // ms
#define DATACOLLECTTIMENORMAL 10000 // ms
#define CALCULATIONTIME 100 // ms

#define GFX_DEV_DEVICE ESP32_S3_RGB
#define RGB_PANEL
//#define GFX_BL 45

//HWCDC USBSerial;
USBCDC USBSerial;
//#define USBSerial Serial

USBHID HID;

CanFrame rxFrame;

TouchDrvGT911 GT911;
int16_t x[5], y[5];
uint8_t gt911_i2c_addr = GT911_SLAVE_ADDRESS_L;
bool gt911_available = false;

AP33772S pd(Wire);
INA238 ina238(INA238_ADDRESS,&Wire);

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
    ,0 /* pclk_active_neg */, 18000000 /* prefer_speed */, false /* useBigEndian  */,
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

static TBatterySetting Batteries[DAUGHTERBOARDCOUNT]; // Battery data settings and results
static volatile byte DRAM_ATTR ActiveBatteryIndex = 0;

static const uint8_t report_descriptor[] = {
  0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
  0x09, 0x04,        // Usage (Joystick)
  0xA1, 0x01,        // Collection (Application)
  0xA1, 0x00,        //   Collection (Physical)
  0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
  0x09, 0x30,        //     Usage (X)
  0x09, 0x31,        //     Usage (Y)
  0x09, 0x32,        //     Usage (Z)
  0x09, 0x33,        //     Usage (Rx)
  0x09, 0x34,        //     Usage (Ry)
  0x09, 0x35,        //     Usage (Rz)
  0x09, 0x36,        //     Usage (Slider)
  0x09, 0x36,        //     Usage (Slider)
  0x15, 0x81,        //     Logical Minimum (-127)
  0x25, 0x7F,        //     Logical Maximum (127)
  0x75, 0x08,        //     Report Size (8)
  0x95, 0x08,        //     Report Count (8)
  0x81, 0x02,        //     Input (Data,Var,Abs)
  0xC0,              //   End Collection
  0xC0,              // End Collection
};

class CustomHIDDevice : public USBHIDDevice {
public:
  CustomHIDDevice(void) {
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      HID.addDevice(this, sizeof(report_descriptor));
    }
  }

  void begin(void) {
    HID.begin();
  }

  // Called by the USB stack to get the report descriptor
  uint16_t _onGetDescriptor(uint8_t *buffer) {
    memcpy(buffer, report_descriptor, sizeof(report_descriptor));
    return sizeof(report_descriptor);
  }

  // Send a report (8 bytes in this example)
  bool send(uint8_t *data) {
    return HID.SendReport(0, data, 8);   // report_id = 0
  }
};

CustomHIDDevice Device;

static volatile bool CalcBatteryData = false;
static Ticker dataupdateticker;

static Ticker datagetticker;
static volatile bool GetData = false;

#ifdef STANDALONE
static DRAM_ATTR TStageData StageDataTransporter[3];
static Ticker datacollectticker;
static Ticker datastartticker;
static volatile bool GetBatteryData = false;
static volatile bool StoreSettings = false;
static volatile byte SendCommand[COMMAND_SIZE] = {0};
#endif

void onPacketReceived(const uint8_t* buffer, size_t size);
void ClearRunData(PRunDatas RDS);
void ClearStageData(PStageData SD);

dword GetMaxVData(PRunDatas RDS);

/*Read the touchpad*/
//IRAM_ATTR
static void IRAM_ATTR my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
//static void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  uint8_t touched = GT911.getPoint(x, y, GT911.getSupportTouchPoint());
  if (touched > 0) {
    USBSerial.print(millis());
    USBSerial.print("ms ");
    for (int i = 0; i < touched; ++i) {
      int16_t touchX = x[i];
      int16_t touchY = y[i];
      switch (gfx->getRotation()) {
        case 0:
          touchX = gfx->width() - x[i];
          touchY = gfx->height() - y[i];
          break;
        case 1:
          touchX = gfx->width() - y[i];
          touchY = x[i];
          break;
        case 2:
          break;
        case 3:
          touchX = y[i];
          touchY = gfx->height() - x[i];
          break;
      }
      data->state = LV_INDEV_STATE_PRESSED;

      /*Set the coordinates*/
      data->point.x = touchX;
      data->point.y = touchY;
    }
    USBSerial.println();
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
    //if( ret != ESP_OK ) USBSerial.println("NVM error !"); else USBSerial.println("NVM ok.");
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
        USBSerial.println("Event: button value changed");
        #endif

        if ( (event_object == testdischargebutton) || (event_object == startdischargebutton) || (event_object == testchargebutton) || (event_object == startchargebutton) )
        {
          dword temp = 0;
          byte i = 0;

          #ifdef DEBUG                      
          USBSerial.println("Engage buttons");
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
        USBSerial.println("Event: long pressed");
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
          USBSerial.println("Zero button pressed");
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
          USBSerial.println("Request PDO list");
          #endif
          // Prepare the command to engage the hardware
          SendCommand[COMMANDPOSITION]   = CMD_get_PDOList;
          SendCommand[INDEXPOSITION]     = ActiveBatteryIndex;
          SendCommand[LENGTHPOSITION]    = 0U; // length
        }
        else
        {
          #ifdef DEBUG                      
          USBSerial.println("Unknown button pressed");
          #endif
          if (event_user_data == screen3)
          {
            #ifdef DEBUG                      
            //USBSerial.println("Button from screen 3");
            #endif
            if (object_user_data != NULL)
            {
              // WE got a PDO select click !!
              byte SelectPDOindex = ((byte)(uintptr_t)object_user_data);      
              #ifdef DEBUG
              USBSerial.printf("PDO button %d pressed.\r\n", SelectPDOindex);
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
      USBSerial.println("Event: keyboard/checkbox value event");
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
        USBSerial.println("We got a discharge setting !!");
        #endif
      }
      else
      if (SM == smCharge)
      {
        SD = &SET->Stages[FIXEDCHARGESTAGENUMBER];
        #ifdef DEBUG          
        USBSerial.println("We got a charge setting !!");
        #endif
      }
      else
      {
        #ifdef DEBUG          
        USBSerial.println("Unknown stagemode. Should never happen !!");
        #endif
      }

      if (SD != NULL)
      {
        if (lv_obj_check_type(event_object, &lv_checkbox_class))
        {
          #ifdef DEBUG          
          USBSerial.println("Enable or disable a threshold !!");
          #endif
          SD->ThresholdSettings[Mode].Enabled = (lv_obj_get_state(event_object) & LV_STATE_CHECKED); 
          GotSettings = true;
        }
 
        if (lv_obj_check_type(event_object, &lv_keyboard_class))
        {
          if (code == LV_EVENT_READY)
          {
            #ifdef DEBUG                      
            USBSerial.println("Event: keyboardready event");
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
      USBSerial.println("Perpare storing settings in NVM !");
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

void sendObdFrame(uint8_t obdId) {
    CanFrame obdFrame         = {0};
    obdFrame.identifier       = 0x7DF; // Default OBD2 address;
    obdFrame.extd             = 0;
    obdFrame.data_length_code = 8;
    obdFrame.data[0]          = 2;
    obdFrame.data[1]          = 1;
    obdFrame.data[2]          = obdId;
    obdFrame.data[3]          = 0xAA; // Best use 0xAA (0b10101010) instead of 0
    obdFrame.data[4]          = 0xAA; // TWAI / CAN works better this way, as it
    obdFrame.data[5]          = 0xAA; // needs to avoid bit-stuffing
    obdFrame.data[6]          = 0xAA;
    obdFrame.data[7]          = 0xAA;
    // Accepts both pointers and references
    ESP32Can.writeFrame(obdFrame); // timeout defaults to 1 ms
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


  USB.firmwareVersion(0x0002);
  USB.manufacturerName("Consulab for pleasure");
  USB.productName("USB PD controller with HID");
  USB.PID(0x04D8);
  USB.VID(0x003F);  
  USB.serialNumber("FFFF-FFFF");
  USB.usbVersion(0x0002);

  Device.begin();

  USBSerial.begin();

  USB.begin();


  #ifdef DEBUGGGG
  USBSerial.begin(115200);
  int cnt = 5000;     // Will wait for up to ~1 second for Serial to connect.
  while (!Serial && cnt--) {delay(1);}
  // USBSerial.setDebugOutput(true);
  USBSerial.println("SenseCap Indicator startup");
  #endif

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
    USBSerial.println("Partition init error !");
  }
  else
  {
    USBSerial.println("Partition init ok.");
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
  USBSerial.println("Reading stored presets done.");
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
    USBSerial.println("Storing default presets done.");
    #endif
  }

  #endif

  // Init hardware

  //pinMode(BUTTON_PIN, INPUT);

  // Init buf /  expander / i2c
  // This also runs initDisplayPower !!

  if (!WS_CH32_IO::begin(Wire, WS_CH32_IO::DEFAULT_I2C_SDA, WS_CH32_IO::DEFAULT_I2C_SCL,
                           WS_CH32_IO::DEFAULT_I2C_FREQ, &USBSerial)) {
        USBSerial.println("CH32V003 init failed, continuing for display debug");
  }

  // Touch !!
  GT911.setPins(-1, -1);
  if (GT911.begin(Wire, gt911_i2c_addr, WS_CH32_IO::DEFAULT_I2C_SDA, WS_CH32_IO::DEFAULT_I2C_SCL)) {
    USBSerial.print("GT911 initialized successfully at address 0x");
    USBSerial.println(gt911_i2c_addr, HEX);
    gt911_available = true;
  } else {
    USBSerial.print("Failed to initialize GT911 at address 0x");
    USBSerial.println(gt911_i2c_addr, HEX);
    gt911_available = false;
  }

  if (gt911_available) {
    GT911.setHomeButtonCallback([](void *user_data) {
      USBSerial.println("Home button pressed!");
    },
    NULL);
    GT911.setMaxTouchPoint(1); // max is 5
  } else {
    USBSerial.println("GT911 not found; running in LCD-only mode for ESP32-S3-LCD-4.");
  }


  // CAN !!!!

  // Set pins
  ESP32Can.setPins(CAN_TX, CAN_RX);

  // You can set custom size for the queues - those are default
  ESP32Can.setRxQueueSize(5);
  ESP32Can.setTxQueueSize(5);

  // .setSpeed() and .begin() functions require to use TwaiSpeed enum,
  // but you can easily convert it from numerical value using .convertSpeed()
  ESP32Can.setSpeed(ESP32Can.convertSpeed(500));

  // You can also just use .begin()..
  if(ESP32Can.begin()) {
      USBSerial.println("CAN bus started!");
  } else {
      USBSerial.println("CAN bus failed!");
  }

  // Init Display
  #ifdef DEBUG  
  USBSerial.println("Init display.");
  #endif
  if (!gfx->begin())
  {
    #ifdef DEBUG    
    USBSerial.println("gfx->begin() failed!");
    USBSerial.println("Expect sever errors !!!");    
    #endif
  }

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  #ifdef DEBUG
  String LVGL_Arduino = "Init LVGL " + String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  USBSerial.println(LVGL_Arduino);
  #endif
  lv_init();

  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb([](){ 
    //return (uint32_t) (esp_timer_get_time() / 1000LL);
    return (xTaskGetTickCount());    
    //return ((uint32_t)millis());        
  });

  #ifdef DEBUG
  USBSerial.println("Init our lvgl task and refresh.");    
  #endif
  lv_screen_init(gfx, HOR_RES, VER_RES);
  //lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_0);
  //lv_display_set_antialiasing(disp,false);

  // Init touch device
  Serial.println("Init touch screen.");      
  //touch_init(HOR_RES, VER_RES, 0); // rotation will be handled by lvgl
  /*Initialize the input device driver*/
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);

  ActiveBatteryIndex = 0;

  #ifndef LVGLDEMOS
  #ifdef DEBUG
  USBSerial.println("Init GUI.");      
  #endif
  CreateBaseScreen(main_event_handler);
  lv_screen_load(screenbase);
  Setup_Screen1(ActiveBatteryIndex);
  SET = &Batteries[ActiveBatteryIndex];
  Screen1SetData(SET);
  #endif

  #ifdef DEBUG  
  USBSerial.println("Init timers.");      
  #endif

  #ifdef STANDALONE
  datagetticker.attach_ms(DATAGETTIME, datagetcb);
  datacollectticker.attach_ms(DATACOLLECTTIMEFAST, datacollectcb);
  #endif
  dataupdateticker.attach_ms(CALCULATIONTIME, dataupdatecb);  
  
  #ifdef DEBUG
  USBSerial.println("Init done");
  #endif

  #ifdef LVGLDEMOS
  //lv_demo_widgets();  
  lv_demo_benchmark();    
  #endif
}

void loop()
{
  static float ina_mA_c       = 0;
  static float ina_mV_c       = 0;
  static float ina_mW_c       = 0;
  static float ina_T_c        = 0;
  static int   ina_counter_c  = 0;

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

  byte BatteryIndex;
  PBatterySetting SET = NULL;
  PRunDatas RDS = NULL;

  if (GetData)
  {
    GetData = false;

    ina_mA_c      += ina238.getMilliAmpere();
    ina_mV_c      += ina238.getBusMilliVolt();
    ina_mW_c      += ina238.getMilliWatt();
    ina_T_c       += ina238.getTemperature();
    ina_counter_c++;
  }

  #ifdef STANDALONE

  PBatteryBoard BB = NULL;

  byte j;
  uint8_t data_buf[COMMAND_SIZE];

  // Do we have a valid command ?
  if ( (SendCommand[COMMANDPOSITION] != CMD_unknown) && (SendCommand[COMMANDPOSITION] != USB_CMD_error) )
  {
    // Fill the data
    for (j=0; j<(SendCommand[LENGTHPOSITION]+DATASTART); j++) data_buf[j] = SendCommand[j];
    // Reset command
    SendCommand[COMMANDPOSITION] = CMD_unknown; 
    // Send the data request
    //myPacketSerial.send(data_buf, j);
  }

  if (GetBatteryData)
  {
    GetBatteryData = false;

    /*
    sendObdFrame(5); // For coolant temperature
    // You can set custom timeout, default is 1000
    if(ESP32Can.readFrame(rxFrame, 100)) {
        // Comment out if too many frames
        USBSerial.printf("Received frame: %03X  \r\n", rxFrame.identifier);
        if(rxFrame.identifier == 0x7E8) {                                    // Standard OBD2 frame responce ID
            USBSerial.printf("Collant temp: %3d°C \r\n", rxFrame.data[3] - 40); // Convert to °C
        }
    }
    */

    #ifdef DEBUG
    //USBSerial.println("Getting data");
    #endif

    for (BatteryIndex=0; BatteryIndex<DAUGHTERBOARDCOUNT; BatteryIndex++ )
    {
      SET = &Batteries[BatteryIndex];

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
          USBSerial.print("Invalid battery mode !! Number: ");
          USBSerial.println(SET->TestData.Active);          
          #endif
          break;
      }

      if (SET->TestData.DataTriggerCounter == 0)
      {

        RDS = &SET->TestData.RunDatas;        
        
        USBSerial.printf("AP33772S data. T: %d°C. VREQ: %5umV. IREQ: %5umA.",
                      pd.getTemperature_C(),
                      pd.getRequestedVoltage_mV(),
                      pd.getRequestedCurrent_mA());

        if (pd.isDerating()) USBSerial.print(F("  [DR]"));
        if (pd.isFault())    USBSerial.printf("  [%s]", pd.getFaultString().c_str());

        USBSerial.println();

        if (ina_counter_c == 0)
        {
          // Get latest data
          RDS->LastBatteryData.I = lroundf(ina238.getMilliAmpere());
          RDS->LastBatteryData.V = lroundf(ina238.getBusMilliVolt());
          RDS->LastBatteryData.P = lroundf(ina238.getMilliWatt());
          RDS->LastBatteryData.T = lroundf(ina238.getTemperature() * 10);
        }
        else
        {
          // Get average data
          RDS->LastBatteryData.V = lroundf(ina_mV_c / ina_counter_c);
          RDS->LastBatteryData.I = lroundf(ina_mA_c / ina_counter_c);
          RDS->LastBatteryData.P = lroundf(ina_mW_c / ina_counter_c);
          RDS->LastBatteryData.T = lroundf(((ina_T_c *10) / ina_counter_c));

          ina_mA_c      = 0;
          ina_mV_c      = 0;
          ina_mW_c      = 0;
          ina_T_c       = 0;
          ina_counter_c = 0;
        }

        // Show data on screen 1
        Screen1AddVIData(RDS->LastBatteryData.V, RDS->LastBatteryData.I);
        
        if (SET->TestData.Active == bmActive)
        {
          //Append the data in storage
          AddMeasurementData(BatteryIndex, RDS->LastBatteryData.V, RDS->LastBatteryData.I, RDS->LastBatteryData.P, RDS->LastBatteryData.T);

          // Append data into graphs if visible
          if (ActiveBatteryIndex == BatteryIndex) Screen2AddData(RDS->LastBatteryData.V, RDS->LastBatteryData.I);

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

    for (BatteryIndex=0; BatteryIndex<DAUGHTERBOARDCOUNT; BatteryIndex++ )
    {
      SET = &Batteries[BatteryIndex];
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

      if (ActiveBatteryIndex == BatteryIndex)
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

  #ifdef STANDALONE
  if (StoreSettings)
  {
    StoreSettings = false;
    #ifdef DEBUG
    USBSerial.println("Storing setting in NVM !");
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

          USBSerial.printf("PDO received ! PDO index: #%d.\r\n", index);

          if (index)
          {
            raw.byte0 = buffer[counter++];
            raw.byte1 = buffer[counter++];
            // This fuction is index zero based !!
            AP33772S::decodePDONew(index-1, raw, dec);

            if (dec.valid)
            {
              USBSerial.printf("PDO received ! PDO voltage : #%dmV.\r\n", dec.maxVoltage_mV);
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

