// CBUS library
#include "./CBUS.h"

CBUSbase::CBUSbase()
{
  CanRxBuffer.clear();
  CanTxBuffer.clear();
  CanTxPrioBuffer.clear();
}
