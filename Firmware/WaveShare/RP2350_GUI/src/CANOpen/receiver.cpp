#include "receiver.hpp"

static struct can_frame canmessageframe;

// Update read message buffer
void Receiver::read(volatile bool *flagRecv) {

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
    }
  }
  else active = false; 

  if (*flagRecv)
  {
    *flagRecv = 0;
    //active = true;
    /*
    while (CAN_MSGAVAIL == CAN->checkReceive())
    {
      CAN->readMessage(&frame); // Pass address of local frame
      id = frame.can_id;
      dlc = frame.can_dlc;
      memcpy(data, frame.data, 8);
      
      #ifndef PRINT
        printBuf(); 
      #endif
    }
    */
  }
  //else active = false; 
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