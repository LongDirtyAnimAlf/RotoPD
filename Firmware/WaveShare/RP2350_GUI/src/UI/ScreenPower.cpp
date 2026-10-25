#include "Screenbase.h"
#include "ScreenPower.h"
#include "powermeter.h"

lv_obj_t * screenpower = NULL;

void Setup_ScreenPower(byte index)
{
  lv_obj_t * obj = NULL;
  lv_obj_t * cont = NULL;

  obj = GetInfoObject();
  if (obj != NULL) lv_label_set_text(obj,"IT2704");

  obj = GetButtonLabelObject();
  if (obj != NULL) lv_label_set_text(obj,"Chart");

  SetContentObject(screenpower,true);
  cont = screenpower;

  if ((cont != NULL) && (lv_obj_get_child_count(cont) == 0))
  {
    create_3d_gauge(cont);
  }
}

void ScreenPowerSetVoltageData(word V)
{
  static word oldV = -1;
  if (!lv_obj_is_hidden(screenpower))
  {
    if (V != oldV)
    {
      gauge_set_voltage(V);
      oldV = V;
    }
  }
}
void ScreenPowerSetCurrentData(word I)
{
  static word oldI = -1;
  if (!lv_obj_is_hidden(screenpower))
  {
    if (I != oldI)
    {
      gauge_set_current(I);
      oldI = I;
    }

    gauge_set_current(I);
  }
}
void ScreenPowerSetPowerData(dword P)
{
  static dword oldP = -1;
  dword localP;
  if (!lv_obj_is_hidden(screenpower))
  {
    localP = (P / 1000);
    if (localP != oldP)
    {
      gauge_set_power(localP);
      oldP = localP;
    }
  }
}
void ScreenPowerSetEnergyData(dword E)
{
  static dword oldE = -1;
  if (!lv_obj_is_hidden(screenpower))
  {
    if (E != oldE)
    {
      gauge_set_energy(E);
      oldE = E;
    }
  }
}

void ScreenPowerAddEPData(dword E, dword P)
{
  ScreenPowerSetEnergyData(E);  
  ScreenPowerSetPowerData(P);
}

byte ScreenPowerGetActive(void)
{
  return 0;
}

void ScreenPowerSetData(PMeasurementData MAD)
{
  ScreenPowerAddEPData(MAD->E, (MAD->P));
  ScreenPowerSetVoltageData(MAD->V);
  ScreenPowerSetCurrentData(MAD->I);
}
