#include "receiver.hpp"

static struct can_frame canmessageframe;

// Update read message buffer
bool Receiver::read(void) {
  if (CAN->checkReceive())
  {
    Serial.println("Got frame.");
    active = true;
    if (CAN->readMessage(&canmessageframe) == MCP2515::ERROR_OK)
    {
      Serial.println("Processing frame.");
      id = canmessageframe.can_id;
      dlc = canmessageframe.can_dlc;
      memcpy(data, canmessageframe.data, 8);
      #ifdef PRINT
        printBuf(); 
      #endif
    }
    else
    {
      Serial.println("Processing frame error !!");
      active = false;
    }
  }
  else
  {
    active = false; 
  } 
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