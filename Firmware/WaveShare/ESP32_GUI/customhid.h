#include <Arduino.h>

#ifdef ENABLEUSB

#include "USB.h"
#include "USBHID.h"

USBHID HID;

class CustomHIDDevice : public USBHIDDevice {
private:
  // USB HID report descriptor.
  uint8_t const desc_hid_report_esp32[] = {TUD_HID_REPORT_DESC_GENERIC_INOUT(HID_INT_OUT_EP_SIZE)};
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

CustomHIDDevice Device;

#endif