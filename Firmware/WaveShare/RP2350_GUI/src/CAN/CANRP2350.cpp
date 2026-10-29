#include "CANRP2350.h"

//#ifdef ARDUINO_ARCH_RP2040

#include <string.h>

// Define constructor implementation here
CBUSRP2350Device::CBUSRP2350Device() : 
  can(),
  _intPin(BSP_XL2515_INT_PIN)
{
  mutex_init(&can_mutex); 
  HandleTxMessages = false;
}

void CBUSRP2350Device::begin(void)
{
    can.begin();

    if (MCP2515::ERROR_OK == can.reset())
    {
      can.setBitrate(CAN_250KBPS);
      can.setNormalMode();
      //can.enableInterrupt(_intPin, ISRhandler);
      can.enableInterrupt(_intPin, NULL);
      Serial.println("MCP2515 Initialized Successfully.");
    } else {
      Serial.println("MCP2515 Initialize Failure !!!");
    }
}

void CBUSRP2350Device::begin(uint8_t MasterID, uint8_t IT2704ID)
{
    can.begin();

    if (MCP2515::ERROR_OK == can.reset())
    {

        can.setBitrate(CAN_250KBPS);

        if ((MasterID != -1) && (IT2704ID != -1))
        {
          can.setConfigMode();

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

        can.setNormalMode();
        //can.enableInterrupt(_intPin, ISRhandler);
        can.enableInterrupt(_intPin, NULL);
        Serial.println("MCP2515 Initialized Successfully.");
    } else {
        Serial.println("MCP2515 Initialize Failure !!!");
    }
}

bool CBUSRP2350Device::checkInterrupt(void) {
  return can.checkInterrupt();
}

CBUSbase::ERROR CBUSRP2350Device::abortAllPending(bool withqueues)
{
  if (withqueues)
  {
    this->CanTxBuffer.clear();
    this->CanTxPrioBuffer.clear();
  }
  return CBUSbase::ERROR_OK;
}

CBUSbase::ERROR CBUSRP2350Device::reset(void)
{
   return (CBUSbase::ERROR)can.reset();
}

bool CBUSRP2350Device::processTxQueue(void)
{
    struct can_frame frame;

    while (CanTxPrioBuffer.peek(frame)) {
        ERROR result = (CBUSbase::ERROR)can.sendMessagePrio(&frame);
        if (result != ERROR_OK) {
            return false;
        }
        //Serial.println("MCP2515: Send prio message ok.");
        CanTxPrioBuffer.pop(frame);
    }

    while (CanTxBuffer.peek(frame)) {
        ERROR result = (CBUSbase::ERROR)can.sendMessage(&frame);
        if (result != ERROR_OK) {
            return false;
        }
        //Serial.println("MCP2515: Send normal message ok.");        
        CanTxBuffer.pop(frame);
    }

    return ((!CanTxPrioBuffer.isEmpty()) && (!CanTxBuffer.isEmpty()));
}

bool CBUSRP2350Device::run(bool single)
{
  // This perform the real hardware access
  // Pushed receved messages into the Rx buffer
  // Pops Tx messages from Tx buffer to send 

  if (can.checkInterrupt())
  {
    //mutex_enter_blocking(&can_mutex);

    // Some interrupt has happended on the MCP2515
    // First handle read, if any
    uint8_t canintf = can.getInterrupts();
    uint8_t eflg = 0;

    if (canintf & (MCP2515::CANINTF_ERRIF | MCP2515::CANINTF_MERRF))
    {
        if (canintf & MCP2515::CANINTF_MERRF)
        {
            Serial.println("Message error interrupt.");
        }

        if (canintf & MCP2515::CANINTF_ERRIF)
        {
            Serial.println("General Error interrupt.");
        }

        eflg = can.getErrorFlags();

        if (eflg & MCP2515::EFLG_RX0OVR)
        {
            Serial.println("RX0OVR: receive buffer 0 overflow interrupt !!!!!!!!!!!!!!!!!!!!!!!!!");
            canintf |= (MCP2515::CANINTF_RX0IF); // force a read action
        }

        if (eflg & MCP2515::EFLG_RX1OVR)
        {
            Serial.println("RX1OVR: receive buffer 1 overflow interrupt !!!!!!!!!!!!!!!!!!!!!!!!!");
            canintf |= (MCP2515::CANINTF_RX1IF); // force a read action
        }
    }

    while (canintf & (MCP2515::CANINTF_RX0IF | MCP2515::CANINTF_RX1IF))
    {
        can_frame frame;

        if (canintf & MCP2515::CANINTF_RX0IF)
        {
            //Serial.println("RX0 interrupt.");
            // This readMessage function also clears the CANINTF_RX0IF flag !!
            if (can.readMessage(MCP2515::RXB0, &frame) == MCP2515::ERROR_OK)
            {
              CanRxBuffer.push(frame);
            }
        }

        if (canintf & MCP2515::CANINTF_RX1IF)
        {
            //Serial.println("RX1 interrupt.");                
            if (can.readMessage(MCP2515::RXB1, &frame) == MCP2515::ERROR_OK)
            {
              CanRxBuffer.push(frame);
            }
        }

        if (single) break;

        canintf = can.getInterrupts();

        //if (single) break;
    }
    
    if (eflg & (MCP2515::EFLG_RX0OVR | MCP2515::EFLG_RX1OVR))
    {
      can.clearRXnOVRFlags();
    }
    if (canintf & MCP2515::CANINTF_ERRIF)
    {
      can.clearERRIF();
    }
    if (canintf & MCP2515::CANINTF_MERRF)
    {
      can.clearMERR();
    }


    // Second, handle write, if any

    if (canintf & (MCP2515::CANINTF_TX0IF | MCP2515::CANINTF_TX1IF | MCP2515::CANINTF_TX2IF))
    {
        //if (canintf & CANINTF_TX0IF) Serial.println("TX0 interrupt.");
        //if (canintf & CANINTF_TX1IF) Serial.println("TX1 interrupt.");
        //if (canintf & CANINTF_TX2IF) Serial.println("TX2 interrupt.");
        // This flags indicates that we can send a message !!

        // First, clear the interrupts
        can.clearTXInterrupts();

        HandleTxMessages = true;
    }

    //mutex_exit(&can_mutex);  
  }

  if (HandleTxMessages)
  {
    HandleTxMessages = false;
    // Process the message queue
    processTxQueue();
  }

  return (CanRxBuffer.getCount() != 0);
}
CBUSbase::ERROR CBUSRP2350Device::sendMessage(const can_frame *frame)
{
  bool enqueued = CanTxBuffer.push(*frame);

  if (enqueued) {
      HandleTxMessages = true;
      //processTxQueue();
      return ERROR_OK;
  }

  return ERROR_ALLTXBUSY;
}
CBUSbase::ERROR CBUSRP2350Device::sendMessage(const can_frame &frame)
{
  return sendMessage(&frame);
}
CBUSbase::ERROR CBUSRP2350Device::sendMessagePrio(const can_frame *frame)
{
  bool enqueued = CanTxPrioBuffer.push(*frame);

  if (enqueued) {
      HandleTxMessages = true;
      //processTxQueue();
      return ERROR_OK;
  }

  return ERROR_ALLTXBUSY;
}
CBUSbase::ERROR CBUSRP2350Device::sendMessagePrio(const can_frame &frame)
{
  return sendMessagePrio(&frame);
}
CBUSbase::ERROR CBUSRP2350Device::readMessage(can_frame *frame)
{
  //return (CBUSbase::ERROR)can.readMessage(frame);
  if (CanRxBuffer.getCount() != 0)
  {
    if (CanRxBuffer.pop(*frame))
    {
      return CBUSbase::ERROR_OK;
    }
  }
  return CBUSbase::ERROR_NOMSG;
}
CBUSbase::ERROR CBUSRP2350Device::readMessage(can_frame &frame)
{
  return readMessage(&frame);
}
uint32_t CBUSRP2350Device::getRxQueueDepth(void)
{
  return CanRxBuffer.getCount();
}

//CBUSRP2350Device CBUSDevice;

//#endif
