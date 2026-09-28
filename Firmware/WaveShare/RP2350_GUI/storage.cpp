#include "storage.h"
//#include <EEPROM.h>
#include <LittleFS.h>

constexpr uint32_t MAGIC = 0xDEADBEEF;

/*

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
  //const size_t base = sizeof(MAGIC) + sizeof(TBoardInfo);
  //const size_t entry = sizeof(MAGIC) + sizeof(TStageData);
  //return base + (index * 2 + (isCharge ? 1 : 0)) * entry;
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
*/

// Fixed filenames – keep them short to minimise directory traffic
static const char *BOARD_INFO_FILE = "/board.inf";
static const char *STAGE_FILE_FMT  = "/s%02u%c.dat";   // s00D.dat / s00C.dat …

// ------------------------------------------------------------------
// Internal helpers (all forced into SRAM)
// ------------------------------------------------------------------
static void __not_in_flash_func(lfsWriteBlob)(const char *path,
                                              const void *data, size_t len)
{
  // Core already cleans the XIP cache & saves/restores QMI for us,
  // but we still disable IRQs for the shortest possible window.
  noInterrupts();
  File f = LittleFS.open(path, "w");
  if (f) {
    f.write(reinterpret_cast<const uint8_t *>(data), len);
    f.flush();          // force the page to flash immediately
    f.close();
  }
  interrupts();
}

static bool __not_in_flash_func(lfsReadBlob)(const char *path,
                                             void *data, size_t len)
{
  File f = LittleFS.open(path, "r");
  if (!f || f.size() < len) {
    if (f) f.close();
    return false;
  }
  size_t rd = f.read(reinterpret_cast<uint8_t *>(data), len);
  f.close();
  return rd == len;
}

// ------------------------------------------------------------------
// Public API – identical signatures to the EEPROM version
// ------------------------------------------------------------------
void storeInit(void)
{
  // Mount once; the core takes care of the rest.
  // Format only if the FS is missing / corrupted.
  if (!LittleFS.begin()) {
    LittleFS.format();
    LittleFS.begin();
  }
}

void __not_in_flash_func(storePutBoardInfo)(TBoardInfo *Info)
{
  struct {
    uint32_t magic;
    TBoardInfo info;
  } blob;
  blob.magic = MAGIC;
  blob.info  = *Info;
  lfsWriteBlob(BOARD_INFO_FILE, &blob, sizeof(blob));
}

bool __not_in_flash_func(storeGetBoardInfo)(TBoardInfo *Info)
{
  struct {
    uint32_t magic;
    TBoardInfo info;
  } blob;
  if (!lfsReadBlob(BOARD_INFO_FILE, &blob, sizeof(blob)))
    return false;
  if (blob.magic != MAGIC)
    return false;
  *Info = blob.info;
  return true;
}

void __not_in_flash_func(storePutBatteryDischargeSetting)(uint8_t Index, TStageData *SD)
{
  char path[16];
  snprintf(path, sizeof(path), STAGE_FILE_FMT, Index, 'D');
  struct {
    uint32_t magic;
    TStageData data;
  } blob;
  blob.magic = MAGIC;
  blob.data  = *SD;
  lfsWriteBlob(path, &blob, sizeof(blob));
}

void __not_in_flash_func(storePutBatteryChargeSetting)(uint8_t Index, TStageData *SD)
{
  char path[16];
  snprintf(path, sizeof(path), STAGE_FILE_FMT, Index, 'C');
  struct {
    uint32_t magic;
    TStageData data;
  } blob;
  blob.magic = MAGIC;
  blob.data  = *SD;
  lfsWriteBlob(path, &blob, sizeof(blob));
}

bool __not_in_flash_func(storeGetBatteryDischargeSetting)(uint8_t Index, TStageData *SD)
{
  char path[16];
  snprintf(path, sizeof(path), STAGE_FILE_FMT, Index, 'D');
  struct {
    uint32_t magic;
    TStageData data;
  } blob;
  if (!lfsReadBlob(path, &blob, sizeof(blob)))
    return false;
  if (blob.magic != MAGIC)
    return false;
  *SD = blob.data;
  return true;
}

bool __not_in_flash_func(storeGetBatteryChargeSetting)(uint8_t Index, TStageData *SD)
{
  char path[16];
  snprintf(path, sizeof(path), STAGE_FILE_FMT, Index, 'C');
  struct {
    uint32_t magic;
    TStageData data;
  } blob;
  if (!lfsReadBlob(path, &blob, sizeof(blob)))
    return false;
  if (blob.magic != MAGIC)
    return false;
  *SD = blob.data;
  return true;
}
