#include "Screenbase.h"
#include "ScreenChart.h"

#define CHARTSIZE 255

lv_obj_t * screenchart = NULL;

static lv_obj_t * ScreenChartVoltageDisplay = NULL;
static lv_obj_t * ScreenChartCurrentDisplay = NULL;
static lv_obj_t * chart = NULL;
static lv_chart_series_t * voltage_series = NULL;
static lv_chart_series_t * current_series = NULL;

static int32_t my_voltage_values[CHARTSIZE]; 
static int32_t my_current_values[CHARTSIZE]; 

void Setup_ScreenChart(byte index)
{
  lv_obj_t * obj = NULL;
  lv_obj_t * cont = NULL;

  obj = GetInfoObject();
  if (obj != NULL) lv_label_set_text(obj,"Graph");

  obj = GetButtonLabelObject();
  if (obj != NULL) lv_label_set_text(obj,"Control");

  SetContentObject(screenchart,true);
  cont = screenchart;

  if ((cont != NULL) && (lv_obj_get_child_count(cont) == 0))
  {
  
    static int32_t col_dsc2[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc2[] = {LV_GRID_FR(2), LV_GRID_FR(6), LV_GRID_TEMPLATE_LAST};

    lv_obj_t * grid = lv_obj_create(cont);
    lv_obj_set_size(grid, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(grid, 0, LV_PART_MAIN);

    lv_obj_set_style_pad_row(grid, 0, 0);
    lv_obj_set_style_pad_column(grid, 0, 0);
    lv_obj_set_grid_dsc_array(grid, col_dsc2, row_dsc2);
    lv_obj_center(grid);

    // Style for 7 segment panel
    static lv_style_t style2;
    lv_style_init(&style2);
    // Set a background color and a radius
    lv_style_set_bg_opa(&style2, LV_OPA_COVER);
    lv_style_set_bg_color(&style2, lv_palette_lighten(LV_PALETTE_GREY, 1));
    // Add a shadow
    lv_style_set_shadow_width(&style2, 6);
    lv_style_set_shadow_color(&style2, lv_palette_main(LV_PALETTE_BLUE));

    lv_obj_t * cell = NULL;

    cell = lv_obj_create(grid);
    lv_obj_remove_style_all(cell);
    lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_scrollable(cell, false);

    ScreenChartVoltageDisplay = create_display(cell, lv_palette_main(LV_PALETTE_RED), true, 5);
    //lv_obj_null_on_delete(&ScreenChartVoltageDisplay);
    lv_obj_add_style(ScreenChartVoltageDisplay, &style2, 0);  
    lv_obj_align(ScreenChartVoltageDisplay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_scrollable(ScreenChartVoltageDisplay, false);

    cell = lv_obj_create(grid);
    lv_obj_remove_style_all(cell);
    lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_scrollable(cell, false);

    ScreenChartCurrentDisplay = create_display(cell, lv_palette_main(LV_PALETTE_GREEN), true, 5);
    //lv_obj_null_on_delete(&ScreenChartCurrentDisplay);
    lv_obj_add_style(ScreenChartCurrentDisplay, &style2, 0);  
    lv_obj_align(ScreenChartCurrentDisplay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_scrollable(ScreenChartCurrentDisplay, false);

    cell = lv_obj_create(grid);
    lv_obj_remove_style_all(cell);
    lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_scrollable(cell, false);
    
    lv_obj_t * scale_left = lv_scale_create(cell);
    lv_obj_set_size(scale_left, lv_pct(8), lv_pct(96));
    lv_obj_set_style_pad_all(scale_left, 0, 0);
    lv_scale_set_mode(scale_left, LV_SCALE_MODE_VERTICAL_LEFT);
    //lv_scale_set_range(scale_left, 0, 30);
    lv_obj_align(scale_left, LV_ALIGN_BOTTOM_LEFT, -8, -4);    
    lv_scale_set_total_tick_count(scale_left, 31);
    lv_scale_set_major_tick_every(scale_left, 5);
    //lv_scale_set_label_show(scale_left, true);
    lv_obj_set_style_line_color(scale_left, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    lv_obj_set_style_line_color(scale_left, lv_palette_main(LV_PALETTE_RED), LV_PART_ITEMS);
    lv_obj_set_style_text_color(scale_left, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR); // For labels

    static const char * voltages[] = {"0", "5", "10", "15", "20", "25", "30", NULL};
    lv_scale_set_text_src(scale_left, voltages);


    lv_obj_t * scale_right = lv_scale_create(cell);
    lv_obj_set_size(scale_right, lv_pct(8), lv_pct(96));
    lv_obj_set_style_pad_all(scale_right, 0, 0);
    // Set right scale
    lv_scale_set_mode(scale_right, LV_SCALE_MODE_VERTICAL_RIGHT);
    //lv_scale_set_range(scale_right, 0, 6);
    lv_obj_align(scale_right, LV_ALIGN_BOTTOM_RIGHT, 12, -4);    
    lv_scale_set_total_tick_count(scale_right, 31);
    lv_scale_set_major_tick_every(scale_right, 5);
    //lv_scale_set_label_show(scale_right, true);
    lv_obj_set_style_line_color(scale_right, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
    lv_obj_set_style_line_color(scale_right, lv_palette_main(LV_PALETTE_GREEN), LV_PART_ITEMS);
    lv_obj_set_style_text_color(scale_right, lv_palette_main(LV_PALETTE_GREEN), LV_PART_INDICATOR); // For labels

    static const char * currents[] = {"0", "1", "2", "3", "4", "5", "6", NULL};
    lv_scale_set_text_src(scale_right, currents);

    chart = lv_chart_create(cell);
    //lv_obj_null_on_delete(&chart);  
    //lv_obj_set_size(chart, lv_pct(100-8-8+2), lv_pct(96));
    lv_obj_set_size(chart, (lv_pct(100-8-8) + 2), lv_pct(96));

    // Set chart
    //lv_obj_align(chart, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_align(chart, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);   //Show lines and points too

    lv_obj_set_style_pad_all(chart, 0, 0);
    lv_obj_set_style_radius(chart, 0, 0);
    lv_chart_set_div_line_count(chart, 7, 0);

    //lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_CIRCULAR);
    lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 30000);    
    lv_chart_set_range(chart, LV_CHART_AXIS_SECONDARY_Y, 0, 6000);    
    lv_chart_set_point_count(chart, CHARTSIZE);  

    voltage_series = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    //lv_obj_null_on_delete(&voltage_series);  
    current_series = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_SECONDARY_Y);  
    //lv_obj_null_on_delete(&current_series);  
    
    lv_chart_set_ext_y_array(chart, voltage_series, my_voltage_values);    
    lv_chart_set_ext_y_array(chart, current_series, my_current_values);    

    obj = lv_label_create(cell);
    lv_obj_align_to(obj, scale_left, LV_ALIGN_OUT_RIGHT_TOP, 5, -10);
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN);  
    lv_label_set_text(obj, "Volt");
    lv_obj_set_style_text_color(obj, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);    

    obj = lv_label_create(cell);
    lv_obj_align_to(obj, scale_right, LV_ALIGN_OUT_LEFT_TOP, -20, -10);
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN);  
    lv_label_set_text(obj, "Amp");
    lv_obj_set_style_text_color(obj, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);    

  }
}

void ScreenChartAddData(word V, word I)
{
  dword tempcalc;

  if (!lv_obj_is_hidden(screenchart))
  {
    tempcalc = V;
    if (ScreenChartVoltageDisplay != NULL) SetDisplaymV(ScreenChartVoltageDisplay, (word)tempcalc);
    if (voltage_series != NULL) lv_chart_set_next_value(chart, voltage_series, (word)tempcalc);

    tempcalc = I;
    if (ScreenChartCurrentDisplay != NULL)  SetDisplaymV(ScreenChartCurrentDisplay, (word)tempcalc);
    if (current_series != NULL) lv_chart_set_next_value(chart, current_series, (word)tempcalc);
  }

}

byte ScreenChartGetActive(void)
{
  return 0;
}

void ScreenChartSetDataQueue(const CircularDataQueue<TMeasurementData, DATASIZE>& q)
{
  dword tempvcalc,tempicalc = 0;
  // Create array
  static TMeasurementData lastItems[CHARTSIZE];

  if (ScreenChartVoltageDisplay != NULL) SetDisplaymV(ScreenChartVoltageDisplay, 0);          
  if (ScreenChartCurrentDisplay != NULL) SetDisplaymV(ScreenChartCurrentDisplay, 0);

  for(int i = 0; i < CHARTSIZE; i++) {
    my_voltage_values[i] = LV_CHART_POINT_NONE;
    my_current_values[i] = LV_CHART_POINT_NONE;
  }

  TMeasurementData newestItem;
  if (q.peekFromNewest(0, newestItem)) {
      // newestItem contains the most recent reading
  }

  TMeasurementData item200;
  if (q.peekFromNewest(199, item200)) {
      // item200 contains the 200th newest reading
  }

  // Request up to 200 of the newest entries
  uint16_t fetchedCount = q.peekNewest(lastItems, CHARTSIZE);

  while (fetchedCount > 0)
  {
      --fetchedCount;
      tempvcalc = lastItems[fetchedCount].V;
      if (voltage_series != NULL) lv_chart_set_next_value(chart, voltage_series, (word)tempvcalc);
      tempicalc = lastItems[fetchedCount].I;
      if (current_series != NULL) lv_chart_set_next_value(chart, current_series, (word)tempicalc);
  }

  if (ScreenChartVoltageDisplay != NULL) SetDisplaymV(ScreenChartVoltageDisplay, (word)tempvcalc);
  if (ScreenChartCurrentDisplay != NULL) SetDisplaymV(ScreenChartCurrentDisplay, (word)tempicalc);
}
