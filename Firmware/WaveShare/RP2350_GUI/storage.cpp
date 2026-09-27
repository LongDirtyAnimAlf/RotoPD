#include "storage.h"

void storeInit(void)
{
  EEPROM.begin(512);
}

void storePutBoardInfo(TBoardInfo * Info)
{
  EEPROM.put(0U, *Info);
  EEPROM.commit();      
}

void storeGetBoardInfo(TBoardInfo * Info)
{
  EEPROM.get(0U, *Info);
}
