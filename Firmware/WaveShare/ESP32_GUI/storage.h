#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "shared.h"
#include "./src/Storage/indicator_storage_nvs.h"

void storePutBoardInfo(TBoardInfo * Info);
void storeGetBoardInfo(TBoardInfo * Info);

#endif
