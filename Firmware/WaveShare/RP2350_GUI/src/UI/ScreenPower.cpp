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
  //if (obj != NULL) lv_label_set_text_fmt(obj, "Power %d", index);

  obj = GetButtonLabelObject();
  if (obj != NULL) lv_label_set_text(obj,"Graph");

  SetContentObject(screenpower,true);
  cont = screenpower;

  if ((cont != NULL) && (lv_obj_get_child_count(cont) == 0))
  {
    create_3d_gauge(cont);
  }
}

void ScreenPowerSetVoltageData(word V)
{
  gauge_set_voltage(V);
}
void ScreenPowerSetCurrentData(word I)
{
  gauge_set_current(I);
}
void ScreenPowerSetPowerData(dword P)
{
  gauge_set_power(P / 1000);
}
void ScreenPowerSetEnergyData(dword E)
{
  gauge_set_energy(E);  
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
