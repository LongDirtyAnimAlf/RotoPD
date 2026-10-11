#include "Screenbase.h"
#include "powermeter.h"

lv_obj_t * screenpower = NULL;

void Setup_ScreenPower(byte index)
{
  lv_obj_t * obj = NULL;
  lv_obj_t * cont = NULL;

  obj = GetInfoObject();
  //if (obj != NULL) lv_label_set_text(obj,"Power");
  if (obj != NULL) lv_label_set_text_fmt(obj, "Power %d", index);

  obj = GetButtonLabelObject();
  if (obj != NULL) lv_label_set_text(obj,"Graph");

  SetContentObject(screenpower,true);
  cont = screenpower;

  if ((cont != NULL) && (lv_obj_get_child_count(cont) == 0))
  {
    create_3d_gauge(cont);
  }
}

void ScreenPowerAddEPData(dword E, dword P)
{
  gauge_set_energy(E);  
  gauge_set_power(P);
}

byte ScreenPowerGetActive(void)
{
  return 0;
}

void ScreenPowerSetData(PRunDatas MAD)
{
  ScreenPowerAddEPData(MAD->LastBatteryData.E / 1000, MAD->LastBatteryData.P);
}
