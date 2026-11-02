#include "CANESP32.h"

#if defined(ARDUINO_ARCH_ESP32)

#include <string.h>

// Define constructor implementation here
CBUSDevice::CBUSDevice(void) : 
  caninterface()
{
  HandleTxMessages = false;
}

void CBUSDevice::begin(void)
{
  bool success = caninterface.begin(
    TwaiSpeed::TWAI_SPEED_250KBPS,  // twaiSpeed
    TX_GPIO_NUM,                    // txPin
    RX_GPIO_NUM,                    // rxPin
    10,                             // txQueue size
    50,                             // rxQueue size
    nullptr,                        // fConfig (Your custom hardware filter)
    nullptr,                        // gConfig (Let library handle automatically)
    nullptr                         // tConfig (Let library handle automatically)
  );
}

void CBUSDevice::begin(uint8_t MasterID, uint8_t IT2704ID)
{
  twai_filter_config_t myFilter;
  myFilter.single_filter = true;
  // Calculate a mask that allows your Node ID or Master Node ID bit patterns
  // We want the hardware to completely ignore the top bits (base functions) 
  // and only validate the lower Node ID bits.
  uint32_t common_mask = 0x000; // Allow upper function bits to pass through hardware
  // Only require bits to match where IT2704NodeID and MasterNodeID share identical bit values
  // Unmatched bits between the two node IDs must be masked out (ignored by hardware)
  uint32_t node_match_mask = ~(IT2704ID ^ MasterID) & 0x7F; 
  uint32_t combined_mask = common_mask | node_match_mask;
  // Set Code and Mask (Shifted 21 bits left for Standard 11-bit IDs)
  myFilter.acceptance_code = ((IT2704ID & combined_mask) << 21);
  myFilter.acceptance_mask = ~(combined_mask << 21);
  bool success = caninterface.begin(
    TwaiSpeed::TWAI_SPEED_250KBPS,  // twaiSpeed
    TX_GPIO_NUM,                    // txPin
    RX_GPIO_NUM,                    // rxPin
    10,                             // txQueue size
    50,                             // rxQueue size
    &myFilter,                      // fConfig (Your custom hardware filter)
    nullptr,                        // gConfig (Let library handle automatically)
    nullptr                         // tConfig (Let library handle automatically)
  );
}

bool CBUSDevice::checkCANInterrupt(void)
{
  return false;
}

CBUSbase::CANERROR CBUSDevice::abortCANAllPending(bool withqueues)
{
  return ERROR_OK;
}

CBUSbase::CANERROR CBUSDevice::resetCAN(void)
{
  caninterface.restart();
  return ERROR_OK;   
}

bool CBUSDevice::taskCAN(bool single)
{
  return (checkCANRxQueue());
}

CBUSbase::CANERROR CBUSDevice::sendCANMessage(const can_frame *frame)
{
  if (sendTWAIMessage(frame)) {
      return ERROR_OK;
  }
  return ERROR_ALLTXBUSY;
}

CBUSbase::CANERROR CBUSDevice::sendCANMessagePrio(const can_frame *frame)
{
  return sendCANMessage(frame);
}

CBUSbase::CANERROR CBUSDevice::readCANMessage(can_frame *frame)
{
  if (checkCANRxQueue())
  {
    if (readTWAIMessage(frame))
    {
      return ERROR_OK;
    }
  }
  return ERROR_NOMSG;
}

uint32_t CBUSDevice::getCANRxQueueDepth(void)
{
  return caninterface.inRxQueue();
}
uint32_t CBUSDevice::getCANTxQueueDepth(void)
{
  return caninterface.inTxQueue();
}
uint32_t CBUSDevice::getCANTxPrioQueueDepth(void)
{
  return getCANTxQueueDepth();
}
bool CBUSDevice::checkCANRxQueue(void)
{
  return ( getCANRxQueueDepth() != 0 );
}
bool CBUSDevice::getCANTxFull(void)
{
  if (getCANTxQueueDepth() >= caninterface.getTxQueueSize()) return true;
  return ( false );
}
bool CBUSDevice::getCANRxFull(void)
{
  if (getCANRxQueueDepth() >= caninterface.getRxQueueSize()) return true;
  return ( false );
}

bool CBUSDevice::readTWAIMessage(struct can_frame *frame)
{
    CanFrame rxFrame         = {0};

    if (caninterface.readFrame(rxFrame, 0))
    {
        frame->can_id   = rxFrame.identifier;
        bool ext = rxFrame.extd;
        if (ext) frame->can_id += CAN_EFF_FLAG;
        bool rtr = rxFrame.rtr;
        if (rtr) frame->can_id += CAN_RTR_FLAG;

        frame->can_dlc  = rxFrame.data_length_code;
        frame->data[0]  = rxFrame.data[0];
        frame->data[1]  = rxFrame.data[1];
        frame->data[2]  = rxFrame.data[2];
        frame->data[3]  = rxFrame.data[3];
        frame->data[4]  = rxFrame.data[4];
        frame->data[5]  = rxFrame.data[5];
        frame->data[6]  = rxFrame.data[6];
        frame->data[7]  = rxFrame.data[7];

        return true;
    }    

    return false;
}

bool CBUSDevice::sendTWAIMessage(const struct can_frame *frame)
{
    CanFrame txFrame         = {0};

    bool ext = (frame->can_id & CAN_EFF_FLAG); // Extended Frame Format (0 = standard 11-bit, 1 = extended 29-bit)
    bool rtr = (frame->can_id & CAN_RTR_FLAG); // Remote Transmission Request (0 = data frame, 1 = remote frame)

    txFrame.identifier       = (frame->can_id & CAN_ERR_MASK);
    txFrame.extd             = ext ? 1 : 0;; // 0 for standard, 1 for extended
    txFrame.rtr              = rtr ? 1 : 0;; // 0 for standard, 1 for extended
    txFrame.data_length_code = frame->can_dlc;
    txFrame.data[0]          = frame->data[0];
    txFrame.data[1]          = frame->data[1];
    txFrame.data[2]          = frame->data[2];
    txFrame.data[3]          = frame->data[3];
    txFrame.data[4]          = frame->data[4];
    txFrame.data[5]          = frame->data[5];
    txFrame.data[6]          = frame->data[6];
    txFrame.data[7]          = frame->data[7];

    if (caninterface.writeFrame(txFrame))
    {
        return true;
    }
    else
    {
        return false;
    }
}

#endif
