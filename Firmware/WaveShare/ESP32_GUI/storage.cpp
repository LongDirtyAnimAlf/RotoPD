#include "storage.h"

void storePutBoardInfo(TBoardInfo * Info)
{
  static DRAM_ATTR char stage[] = "BoardInfo";
  static DRAM_ATTR size_t length = sizeof(TBoardInfo);
  indicator_nvs_write(stage, (void *)Info, length);
}

void storeGetBoardInfo(TBoardInfo * Info)
{
  static DRAM_ATTR char stage[] = "BoardInfo";
  static DRAM_ATTR size_t length = sizeof(TBoardInfo);
  indicator_nvs_read(stage, (void *)Info, &length);
}
