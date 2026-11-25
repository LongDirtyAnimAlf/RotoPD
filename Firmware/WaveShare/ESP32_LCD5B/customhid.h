#include <Arduino.h>

#ifdef ENABLEUSB

#include "comms.h"

USBHID HID;

class CustomHIDDevice : public USBHIDDevice {
private:
  // Declare the HID report descriptor array as a private static member
  // USB HID report descriptor.  
  inline static const uint8_t desc_hid_report_esp32[] = {TUD_HID_REPORT_DESC_GENERIC_INOUT(HID_INT_OUT_EP_SIZE)}; 
  //static const uint8_t desc_hid_report_esp32[]; 
public:
  CustomHIDDevice(void) {
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      HID.addDevice(this, sizeof(desc_hid_report_esp32));
    }
  }

  void begin(void) {
    HID.begin();
  }

  void end(void) {
    HID.end();
  }

  void ready(void) {
    HID.ready();
  }

  // Called by the USB stack to get the report descriptor
  uint16_t _onGetDescriptor(uint8_t *buffer) {
    memcpy(buffer, desc_hid_report_esp32, sizeof(desc_hid_report_esp32));
    return sizeof(desc_hid_report_esp32);
  }

  // Called by the USB stack on set report 
  void _onOutput(uint8_t report_id, const uint8_t *buffer, uint16_t len) {
    set_report_callback(report_id, buffer, len);
  }
};

/*
// Define and initialize the array outside the class body
const uint8_t CustomHIDDevice::desc_hid_report_esp32[] = {
  TUD_HID_REPORT_DESC_GENERIC_INOUT(HID_INT_OUT_EP_SIZE)
};
*/

CustomHIDDevice Device;

#endif
