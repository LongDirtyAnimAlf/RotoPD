#include "CANESP32.h"

//#if defined(ARDUINO_ARCH_ESP32)

#include <string.h>

// Define constructor implementation here
CBUSDevice::CBUSDevice(void) : 
  caninterface()
{
  mutex_init(&can_mutex); 
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
    TwaiSpeed::TWAI_SPEED_250KBPS, // twaiSpeed
    CAN_TX,                     // txPin
    CAN_RX,                     // rxPin
    10,                         // txQueue size
    50,                         // rxQueue size
    &myFilter,                  // fConfig (Your custom hardware filter)
    nullptr,                    // gConfig (Let library handle automatically)
    nullptr                     // tConfig (Let library handle automatically)
  );
}

bool CBUSDevice::checkCANInterrupt(void)
{
  return false;
}

CBUSbase::CANERROR CBUSDevice::abortCANAllPending(bool withqueues)
{
  if (withqueues)
  {
    this->CanTxBuffer.clear();
    this->CanTxPrioBuffer.clear();
  }
  return ERROR_OK;
}

CBUSbase::CANERROR CBUSDevice::resetCAN(void)
{
  caninterface.restart();
  return ERROR_OK;   
}

bool CBUSDevice::taskCAN(bool single)
{
  can_frame frame;

  // This perform the real hardware access
  // Pushed receved messages into the Rx buffer
  // Pops Tx messages from Tx buffer to send 

  while (caninterface.inRxQueue() != 0)
  {
    if (readTWAIMessage(&frame))
    {
      CanRxBuffer.push(frame);
    }
    else
    {
      Serial.println("TWAI: Read message failure.");
      break;
    }
  }

  if (HandleTxMessages)
  {
    HandleTxMessages = false;
    if ((!CanTxPrioBuffer.isEmpty()) || (!CanTxBuffer.isEmpty()))
    {
      // Process the prio message queue
      while (CanTxPrioBuffer.peek(frame))
      {
          if (sendTWAIMessage(&frame))
          {
            //Serial.println("TWAI: Send prio message ok.");
            CanTxPrioBuffer.pop(frame);
          }
          else
          {
            Serial.println("TWAI: Send prio message failure.");
            break;
          }
      }

      while (CanTxBuffer.peek(frame))
      {
          if (sendTWAIMessage(&frame))
          {
            //Serial.println("TWAI: Send normal message ok.");
            CanTxBuffer.pop(frame);
          }
          else
          {
            Serial.println("TWAI: Send normal message failure.");
            break;
          }
      }
    }
  }

  return (!CanRxBuffer.isEmpty());
}

CBUSbase::CANERROR CBUSDevice::sendCANMessage(const can_frame *frame)
{
  bool enqueued = CanTxBuffer.push(*frame);

  if (enqueued) {
      //Serial.println("CBUSDevice: Normal message enqueue ok.");        

      HandleTxMessages = true;
      //processTxQueue();
      return ERROR_OK;
  }

  return ERROR_ALLTXBUSY;
}
CBUSbase::CANERROR CBUSDevice::sendCANMessage(const can_frame &frame)
{
  return sendCANMessage(&frame);
}
CBUSbase::CANERROR CBUSDevice::sendCANMessagePrio(const can_frame *frame)
{
  bool enqueued = CanTxPrioBuffer.push(*frame);

  if (enqueued) {
      //Serial.println("CBUSDevice: Prio message enqueue ok.");        
      HandleTxMessages = true;
      //processTxQueue();
      return ERROR_OK;
  }

  return ERROR_ALLTXBUSY;
}

CBUSbase::CANERROR CBUSDevice::sendCANMessagePrio(const can_frame &frame)
{
  return sendCANMessagePrio(&frame);
}

CBUSbase::CANERROR CBUSDevice::readCANMessage(can_frame *frame)
{
  //return (CBUSbase::ERROR)caninterface.readMessage(frame);
  if (CanRxBuffer.getCount() != 0)
  {
    if (CanRxBuffer.pop(*frame))
    {
      return ERROR_OK;
    }
  }
  return ERROR_NOMSG;
}

CBUSbase::CANERROR CBUSDevice::readCANMessage(can_frame &frame)
{
  return readCANMessage(&frame);
}

bool CBUSDevice::processTxQueue(void)
{
    struct can_frame frame;
    return false; // Added return statement to match function signature
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


//CBUSDevice CBUSDevice;

//#endif
