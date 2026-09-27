#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "shared.h"

#include <EEPROM.h>

void storeInit(void);
void storePutBoardInfo(TBoardInfo * Info);
void storeGetBoardInfo(TBoardInfo * Info);

#endif
