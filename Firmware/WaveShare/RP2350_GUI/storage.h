#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "shared.h"

void storeInit(void);
void storePutBoardInfo(TBoardInfo * Info);
bool storeGetBoardInfo(TBoardInfo * Info);
void storePutBatteryDischargeSetting(uint8_t Index, TStageData *SD);
void storePutBatteryChargeSetting(uint8_t Index, TStageData *SD);
bool storeGetBatteryDischargeSetting(uint8_t Index, TStageData *SD);
bool storeGetBatteryChargeSetting(uint8_t Index, TStageData *SD);

#endif
