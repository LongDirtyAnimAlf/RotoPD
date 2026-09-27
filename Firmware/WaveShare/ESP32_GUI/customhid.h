#include <Arduino.h>

#include "USB.h"
#include "USBHID.h"
//#include "esp32-hal-tinyusb.h"

USBHID HID;

class CustomHIDDevice : public USBHIDDevice {
public:
  CustomHIDDevice(void) {
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      HID.addDevice(this, sizeof(desc_hid_report));
    }
  }

  void begin(void) {
    HID.begin();
  }

  // Called by the USB stack to get the report descriptor
  uint16_t _onGetDescriptor(uint8_t *buffer) {
    memcpy(buffer, desc_hid_report, sizeof(desc_hid_report));
    return sizeof(desc_hid_report);
  }

  // Called by the USB stack on set report 
  void _onOutput(uint8_t report_id, const uint8_t *buffer, uint16_t len) {
    set_report_callback(report_id, buffer, len);
  }
};

CustomHIDDevice Device;