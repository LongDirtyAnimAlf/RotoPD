    cell = lv_obj_create(grid);
    lv_obj_remove_style_all(cell);
    lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t * scale_left = lv_scale_create(cell);
    lv_obj_set_size(scale_left, lv_pct(8), lv_pct(96));

    lv_obj_t * scale_right = lv_scale_create(cell);
    lv_obj_set_size(scale_right, lv_pct(8), lv_pct(96));


    chart = lv_chart_create(cell);
    //lv_obj_null_on_delete(&chart);  
    lv_obj_set_size(chart, lv_pct(92), lv_pct(96));

    // Set left scale
    lv_scale_set_mode(scale_left, LV_SCALE_MODE_VERTICAL_LEFT);
    lv_obj_align(scale_left, LV_ALIGN_LEFT_MID, 0, 0);    
    lv_scale_set_total_tick_count(scale_left, 29);
    lv_scale_set_major_tick_every(scale_left, 7);

    static const char * voltages[] = {"0", "5", "10", "15", "20", "25", "30", NULL};
    lv_scale_set_text_src(scale_left, voltages);

    // Set right scale
    lv_scale_set_mode(scale_right, LV_SCALE_MODE_VERTICAL_RIGHT);
    lv_obj_align(scale_right, LV_ALIGN_RIGHT_MID, 0, 0);    
    lv_scale_set_total_tick_count(scale_right, 29);
    lv_scale_set_major_tick_every(scale_right, 7);

    static const char * currents[] = {"0", "1", "2", "3", "4", "5", "6", NULL};
    lv_scale_set_text_src(scale_right, currents);

    // Set chart
    lv_obj_align(chart, LV_ALIGN_RIGHT_MID, 0, 0);
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

    //lv_obj_set_style_pad_ver(scale_left, lv_chart_get_first_point_center_offset(chart), 0);
    //lv_obj_set_style_pad_ver(scale_right, lv_chart_get_first_point_center_offset(chart), 0);


    obj = lv_label_create(cell);
    lv_obj_align_to(obj, scale_left, LV_ALIGN_OUT_RIGHT_TOP, 5, 0);
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN);  
    lv_label_set_text(obj, "Volt");
