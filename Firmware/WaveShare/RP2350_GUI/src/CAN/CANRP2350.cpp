#include "CANRP2350.h"

//#ifdef ARDUINO_ARCH_RP2040

#include <string.h>

// Define constructor implementation here
CBUSRP2350Device::CBUSRP2350Device() : 
  caninterface(),
  _intPin(BSP_XL2515_INT_PIN)
{
  mutex_init(&can_mutex); 
  HandleTxMessages = false;
}

void CBUSRP2350Device::begin(void)
{
    caninterface.begin();

    if (MCP2515::ERROR_OK == caninterface.reset())
    {
      caninterface.setBitrate(CAN_250KBPS);
      caninterface.setNormalMode();
      //caninterface.enableInterrupt(_intPin, ISRhandler);
      caninterface.enableInterrupt(_intPin, NULL);
      Serial.println("MCP2515 Initialized Successfully.");
    } else {
      Serial.println("MCP2515 Initialize Failure !!!");
    }
}

void CBUSRP2350Device::begin(uint8_t MasterID, uint8_t IT2704ID)
{
    caninterface.begin();

    if (MCP2515::ERROR_OK == caninterface.reset())
    {

        caninterface.setBitrate(CAN_250KBPS);

        if ((MasterID != -1) && (IT2704ID != -1))
        {
          caninterface.setConfigMode();

          caninterface.setFilterMask(MCP2515::MASK0, false, 0x7FF);
          caninterface.setFilter(MCP2515::RXF0, false, 0x580 + IT2704ID);
          caninterface.setFilter(MCP2515::RXF1, false, 0x580 + MasterID);

          // Set Mask 1 to require an exact 11-bit ID match
          caninterface.setFilterMask(MCP2515::MASK1, false, 0x7FF);
          caninterface.setFilter(MCP2515::RXF2, false, 0x000);
          caninterface.setFilter(MCP2515::RXF3, false, 0x180 + IT2704ID);
          caninterface.setFilter(MCP2515::RXF4, false, 0x600 + IT2704ID);
          caninterface.setFilter(MCP2515::RXF5, false, 0x600 + MasterID);
        }

        caninterface.setNormalMode();
        //caninterface.enableInterrupt(_intPin, ISRhandler);
        caninterface.enableInterrupt(_intPin, NULL);
        Serial.println("MCP2515 Initialized Successfully.");
    } else {
        Serial.println("MCP2515 Initialize Failure !!!");
    }
}

bool CBUSRP2350Device::checkCANInterrupt(void)
{
  return caninterface.checkInterrupt();
}

CBUSbase::CANERROR CBUSRP2350Device::abortCANAllPending(bool withqueues)
{
  if (withqueues)
  {
    this->CanTxBuffer.clear();
    this->CanTxPrioBuffer.clear();
  }
  return ERROR_OK;
}

CBUSbase::CANERROR CBUSRP2350Device::resetCAN(void)
{
   return (CBUSbase::CANERROR)caninterface.reset();
}

bool CBUSRP2350Device::taskCAN(bool single)
{
  // This perform the real hardware access
  // Pushed receved messages into the Rx buffer
  // Pops Tx messages from Tx buffer to send 

  if (caninterface.checkInterrupt())
  {
    //mutex_enter_blocking(&can_mutex);

    // Some interrupt has happended on the MCP2515
    // First handle read, if any
    uint8_t canintf = caninterface.getInterrupts();
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

        eflg = caninterface.getErrorFlags();

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
            if (caninterface.readMessage(MCP2515::RXB0, &frame) == MCP2515::ERROR_OK)
            {
              CanRxBuffer.push(frame);
            }
        }

        if (canintf & MCP2515::CANINTF_RX1IF)
        {
            //Serial.println("RX1 interrupt.");                
            if (caninterface.readMessage(MCP2515::RXB1, &frame) == MCP2515::ERROR_OK)
            {
              CanRxBuffer.push(frame);
            }
        }

        if (single) break;

        canintf = caninterface.getInterrupts();

        //if (single) break;
    }
    
    if (eflg & (MCP2515::EFLG_RX0OVR | MCP2515::EFLG_RX1OVR))
    {
      caninterface.clearRXnOVRFlags();
    }
    if (canintf & MCP2515::CANINTF_ERRIF)
    {
      caninterface.clearERRIF();
    }
    if (canintf & MCP2515::CANINTF_MERRF)
    {
      caninterface.clearMERR();
    }


    // Second, handle write, if any

    if (canintf & (MCP2515::CANINTF_TX0IF | MCP2515::CANINTF_TX1IF | MCP2515::CANINTF_TX2IF))
    {
        //if (canintf & CANINTF_TX0IF) Serial.println("TX0 interrupt.");
        //if (canintf & CANINTF_TX1IF) Serial.println("TX1 interrupt.");
        //if (canintf & CANINTF_TX2IF) Serial.println("TX2 interrupt.");
        // This flags indicates that we can send a message !!

        // First, clear the interrupts
        caninterface.clearTXInterrupts();

        HandleTxMessages = true;

        //processTxQueue();
    }

    //mutex_exit(&can_mutex);  
  }

  //if (HandleTxMessages)
  {
    //HandleTxMessages = false;
    if ((!CanTxPrioBuffer.isEmpty()) || (!CanTxBuffer.isEmpty()))
    {
      // Process the message queue
      processTxQueue();
    }
  }

  return (CanRxBuffer.getCount() != 0);
}

CBUSbase::CANERROR CBUSRP2350Device::sendCANMessage(const can_frame *frame)
{
  bool enqueued = CanTxBuffer.push(*frame);

  if (enqueued) {
      //Serial.println("CBUSRP2350Device: Normal message enqueue ok.");        

      HandleTxMessages = true;
      //processTxQueue();
      return ERROR_OK;
  }

  return ERROR_ALLTXBUSY;
}
CBUSbase::CANERROR CBUSRP2350Device::sendCANMessage(const can_frame &frame)
{
  return sendCANMessage(&frame);
}
CBUSbase::CANERROR CBUSRP2350Device::sendCANMessagePrio(const can_frame *frame)
{
  bool enqueued = CanTxPrioBuffer.push(*frame);

  if (enqueued) {
      //Serial.println("CBUSRP2350Device: Prio message enqueue ok.");        
      HandleTxMessages = true;
      //processTxQueue();
      return ERROR_OK;
  }

  return ERROR_ALLTXBUSY;
}

CBUSbase::CANERROR CBUSRP2350Device::sendCANMessagePrio(const can_frame &frame)
{
  return sendCANMessagePrio(&frame);
}

CBUSbase::CANERROR CBUSRP2350Device::readCANMessage(can_frame *frame)
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

CBUSbase::CANERROR CBUSRP2350Device::readCANMessage(can_frame &frame)
{
  return readCANMessage(&frame);
}

bool CBUSRP2350Device::processTxQueue(void)
{
    struct can_frame frame;

    if (CanTxBuffer.getCount() > 20)
    {
      Serial.println("MCP2515: Normal message buffer filling up.");
    }
    if (CanTxPrioBuffer.getCount() > 20)
    {
      Serial.println("MCP2515: Prio message buffer filling up.");
    }

    if (CanTxPrioBuffer.peek(frame))
    {
        MCP2515::ERROR result = caninterface.sendMessagePrio(&frame);
        if (result == MCP2515::ERROR_OK)
        {
          //Serial.println("MCP2515: Send prio message ok.");
          CanTxPrioBuffer.pop(frame);
        }
        else
        {
          Serial.println("MCP2515: Send prio message failure.");
        }
    }

    if (CanTxBuffer.peek(frame))
    {
      MCP2515::ERROR result = caninterface.sendMessage(&frame);
      if (result == MCP2515::ERROR_OK)
      {
        //Serial.println("MCP2515: Send normal message ok.");        
        CanTxBuffer.pop(frame);
      }
      else
      {
        // We might use prio as roll-over !!
        if (CanTxPrioBuffer.isEmpty())
        {
          MCP2515::ERROR result = caninterface.sendMessagePrio(&frame);
          if (result == MCP2515::ERROR_OK)
          {
            //Serial.println("MCP2515: Send prio message ok.");
            CanTxBuffer.pop(frame);
          }
          else
          {
            Serial.println("MCP2515: Send normal -> prio message failure.");
          }
        }
        else
        {
          Serial.println("MCP2515: Send normal message failure.");
        }
      }
    }
    return (!((CanTxPrioBuffer.isEmpty()) && (CanTxBuffer.isEmpty())));
}

//CBUSRP2350Device CBUSDevice;

//#endif
