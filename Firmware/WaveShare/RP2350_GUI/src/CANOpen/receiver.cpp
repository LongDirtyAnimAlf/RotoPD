#include "receiver.hpp"

static struct can_frame canmessageframe;

// Handle canbus activity
bool Receiver::run(void) {
  
  bool active = false;

  if (CAN->checkCANRxQueue())
  {
    active = true;
    if (CAN->readCANMessage(&canmessageframe) == CBUSbase::ERROR_OK)
    {
      id = canmessageframe.can_id;
      dlc = canmessageframe.can_dlc;
      memcpy(data, canmessageframe.data, 8);
      //#ifdef PRINT
      //printBuf(); 
      //#endif
    }
    else
    {
      active = false;
      Serial.println("Very strange receiver reception error !!!");
    }
  }
  else active = false; 
  
  return active;
}

// Print read message buffer to serial
void Receiver::printBuf() {
  char msgString[128] = {0}; 

  snprintf(msgString, sizeof(msgString), "Standard ID: 0x%.3lX       DLC: %1d  Data:", id, dlc);

  Serial.print(msgString);

  // Print buffer contents
  for (uint8_t i = 0; i <dlc; i++)
  {
    snprintf(msgString, sizeof(msgString), " 0x%.2X", data[i]);
    Serial.print(msgString);
  }

  Serial.println();
}