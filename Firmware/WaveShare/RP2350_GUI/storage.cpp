#include "storage.h"

constexpr uint32_t MAGIC = 0xDEADBEEF;
constexpr size_t EEPROM_SIZE = 1024;

void storeInit(void)
{
  EEPROM.begin(EEPROM_SIZE);
}

void __not_in_flash_func(storePutBoardInfo)(TBoardInfo * Info)
{
  EEPROM.put(0, MAGIC);
  EEPROM.put(0 + sizeof(MAGIC), *Info);
  EEPROM.commit();      
}

bool __not_in_flash_func(storeGetBoardInfo)(TBoardInfo * Info)
{
  uint32_t checkMagic;
  EEPROM.get(0, checkMagic);
  if (checkMagic == MAGIC)
  {
    EEPROM.get(0U + sizeof(MAGIC), *Info);
    return true;    
  }
  return false;
}

size_t __not_in_flash_func(stageAddr)(uint8_t index, bool isCharge)
{
  return (sizeof(MAGIC) + sizeof(TBoardInfo)) + ( (index * 2 + (isCharge ? 1 : 0)) * (sizeof(MAGIC) + sizeof(TStageData)) );
  /*
  const size_t base = sizeof(MAGIC) + sizeof(TBoardInfo);
  const size_t entry = sizeof(MAGIC) + sizeof(TStageData);
  return base + (index * 2 + (isCharge ? 1 : 0)) * entry;
  */
}

void __not_in_flash_func(storePutBatteryDischargeSetting)(uint8_t Index, TStageData *SD)
{
  size_t addr = stageAddr(Index, false);
  if (addr + sizeof(MAGIC) + sizeof(TStageData) > EEPROM_SIZE) return;
  EEPROM.put(addr, MAGIC);
  EEPROM.put(addr + sizeof(MAGIC), *SD);
  EEPROM.commit();      
}
void __not_in_flash_func(storePutBatteryChargeSetting)(uint8_t Index, TStageData *SD)
{
  size_t addr = stageAddr(Index, true);
  if (addr + sizeof(MAGIC) + sizeof(TStageData) > EEPROM_SIZE) return;
  EEPROM.put(addr, MAGIC);
  EEPROM.put(addr + sizeof(MAGIC), *SD);
  EEPROM.commit();      
}

bool __not_in_flash_func(storeGetBatteryDischargeSetting)(uint8_t Index, TStageData *SD)
{
  uint32_t checkMagic = 0;
  size_t addr = stageAddr(Index, false);
  EEPROM.get(addr, checkMagic);
  if (checkMagic == MAGIC)
  {
    EEPROM.get(addr + sizeof(MAGIC), *SD);
    return true;    
  }
  return false;
}

bool __not_in_flash_func(storeGetBatteryChargeSetting)(uint8_t Index, TStageData *SD)
{
  uint32_t checkMagic = 0;
  size_t addr = stageAddr(Index, true);
  EEPROM.get(addr, checkMagic);
  if (checkMagic == MAGIC)
  {
    EEPROM.get(addr + sizeof(MAGIC), *SD);
    return true;    
  }
  return false;  
}
