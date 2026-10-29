#include "CANRP2350.h"

//#ifdef ARDUINO_ARCH_RP2040

// Define constructor implementation here
CBUSRP2350Device::CBUSRP2350Device() : 
  can(10 * 1000 * 1000),
  intPin(0)
{
    // Add initialization logic here if required
}

void CBUSRP2350Device::begin(uint8_t MasterID, uint8_t IT2704ID)
{
    can.begin();

    if (MCP2515::ERROR_OK == can.reset())
    {

        can.setBitrate(CAN_250KBPS);

        if ((MasterID != -1) && (IT2704ID != -1))
        {
          can.setOperatingMode(MCP2515::CAN_MODE_CONFIG);

          can.setFilterMask(MCP2515::MASK0, false, 0x7FF);
          can.setFilter(MCP2515::RXF0, false, 0x580 + IT2704ID);
          can.setFilter(MCP2515::RXF1, false, 0x580 + MasterID);

          // Set Mask 1 to require an exact 11-bit ID match
          can.setFilterMask(MCP2515::MASK1, false, 0x7FF);
          can.setFilter(MCP2515::RXF2, false, 0x000);
          can.setFilter(MCP2515::RXF3, false, 0x180 + IT2704ID);
          can.setFilter(MCP2515::RXF4, false, 0x600 + IT2704ID);
          can.setFilter(MCP2515::RXF5, false, 0x600 + MasterID);
        }

        can.setOperatingMode(MCP2515::CAN_MODE_NORMAL);
        //can.enableInterrupt(intPin, ISRhandler);
        can.enableInterrupt(intPin, NULL);
        Serial.println("MCP2515 Initialized Successfully.");
    } else {
        Serial.println("MCP2515 Initialize Failure !!!");
    }
}

bool CBUSRP2350Device::checkInterrupt(void) {
  return false;
}

CBUSbase::ERROR CBUSRP2350Device::abortAllPending(bool withqueues)
{
  return (CBUSbase::ERROR)can.abortAllPending(withqueues);
}

CBUSbase::ERROR CBUSRP2350Device::reset(void)
{
   return (CBUSbase::ERROR)can.reset();
}

bool CBUSRP2350Device::run(bool single)
{
  return (CBUSbase::ERROR)can.run(single);
}
CBUSbase::ERROR CBUSRP2350Device::sendMessage(const can_frame *frame)
{
  return (CBUSbase::ERROR)can.sendMessage(frame);
}
CBUSbase::ERROR CBUSRP2350Device::sendMessage(const can_frame &frame)
{
  return CBUSbase::ERROR_OK;
}
CBUSbase::ERROR CBUSRP2350Device::sendMessagePrio(const can_frame *frame)
{
  return (CBUSbase::ERROR)can.sendMessagePrio(frame);
}
CBUSbase::ERROR CBUSRP2350Device::sendMessagePrio(const can_frame &frame)
{
  return CBUSbase::ERROR_OK;
}
CBUSbase::ERROR CBUSRP2350Device::readMessage(can_frame *frame)
{
  return (CBUSbase::ERROR)can.readMessage(frame);
}
CBUSbase::ERROR CBUSRP2350Device::readMessage(can_frame &frame)
{
  return CBUSbase::ERROR_OK;
}
uint32_t CBUSRP2350Device::getRxQueueDepth(void)
{
  return 0;
}


CBUSRP2350Device CBUSDevice;


//#endif
