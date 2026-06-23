#include "ScreenMeat.h"
#include "MeatTemperatures.h"
#include "UIManager.h"
#include <Arduino.h>
#include <cstdio>

// ============================================================================
// Meat Screen State
// ============================================================================

static lv_obj_t* meat_lbl_type = NULL;
static lv_obj_t* meat_lbl_done = NULL;
static lv_obj_t* meat_lbl_temp = NULL;
static lv_obj_t* meat_lbl_info = NULL;

static int meat_focus = 0;         // 0=type, 1=doneness
static int meat_type_idx = 0;      // From MeatTemperatures.h
static int meat_done_idx = 1;      // Rare, Med-Rare, Medium, Well

// ============================================================================
// Initialization
// ============================================================================

void screen_meat_init(lv_obj_t* parent) {
    lv_obj_t* header = lv_obj_create(parent);
    lv_obj_set_size(header, 240, 30);
    lv_obj_align(header, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_make(20, 20, 20), 0);
    lv_obj_set_style_border_width(header, 0, 0);

    lv_obj_t* title = lv_label_create(header);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_make(255, 193, 7), 0);
    lv_label_set_text(title, "MEAT TEMPS");
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    meat_lbl_type = lv_label_create(parent);
    lv_obj_set_style_text_font(meat_lbl_type, &lv_font_montserrat_20, 0);
    lv_obj_align(meat_lbl_type, LV_ALIGN_CENTER, 0, 50);

    meat_lbl_done = lv_label_create(parent);
    lv_obj_set_style_text_font(meat_lbl_done, &lv_font_montserrat_16, 0);
    lv_obj_align(meat_lbl_done, LV_ALIGN_CENTER, 0, 90);

    meat_lbl_temp = lv_label_create(parent);
    lv_obj_set_style_text_font(meat_lbl_temp, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(meat_lbl_temp, lv_color_make(255, 87, 34), 0);
    lv_obj_align(meat_lbl_temp, LV_ALIGN_CENTER, 0, 130);
}

// ============================================================================
// Update
// ============================================================================

void screen_meat_update(void) {
    lv_obj_set_style_text_color(meat_lbl_type, (meat_focus == 0) ? lv_color_make(255, 87, 34) : lv_color_white(), 0);
    lv_label_set_text(meat_lbl_type, meat_types[meat_type_idx]);

    lv_obj_set_style_text_color(meat_lbl_done, (meat_focus == 1) ? lv_color_make(255, 87, 34) : lv_color_white(), 0);
    lv_label_set_text(meat_lbl_done, meat_doneness[meat_done_idx]);

    char buf[32];
    int temp_c = get_meat_temp(meat_type_idx, meat_done_idx);
    snprintf(buf, sizeof(buf), "%d°C", temp_c);
    lv_label_set_text(meat_lbl_temp, buf);
}

// ============================================================================
// Input Handlers
// ============================================================================

void screen_meat_handle_rotation(int8_t direction) {
    if (meat_focus == 0) {  // Meat type
        meat_type_idx += direction;
        if (meat_type_idx < 0) meat_type_idx = MEAT_TYPE_COUNT - 1;
        if (meat_type_idx >= MEAT_TYPE_COUNT) meat_type_idx = 0;
    } else {  // Doneness
        meat_done_idx += direction;
        if (meat_done_idx < 0) meat_done_idx = DONENESS_COUNT - 1;
        if (meat_done_idx >= DONENESS_COUNT) meat_done_idx = 0;
    }
}

void screen_meat_handle_click(void) {
    meat_focus = (meat_focus + 1) % 2;
}
