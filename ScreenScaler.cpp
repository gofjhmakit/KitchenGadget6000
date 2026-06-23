#include "ScreenScaler.h"
#include "UIManager.h"
#include <Arduino.h>

// ============================================================================
// Scaler Screen State
// ============================================================================

static lv_obj_t* scale_lbl_mult = NULL;
static lv_obj_t* scale_lbl_val = NULL;
static lv_obj_t* scale_lbl_res = NULL;

static int scale_focus = 0;         // 0=multiplier, 1=value
static int scale_mult_idx = 3;      // maps to 1.0x
static float scale_input_val = 100.0;

const float scale_multipliers[] = {0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 2.5, 3.0, 4.0};
#define MULT_COUNT 9

// ============================================================================
// Initialization
// ============================================================================

void screen_scaler_init(lv_obj_t* parent) {
    lv_obj_t* header = lv_obj_create(parent);
    lv_obj_set_size(header, 240, 30);
    lv_obj_align(header, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_make(20, 20, 20), 0);
    lv_obj_set_style_border_width(header, 0, 0);

    lv_obj_t* title = lv_label_create(header);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_make(255, 193, 7), 0);
    lv_label_set_text(title, "PORTION SCALER");
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    scale_lbl_mult = lv_label_create(parent);
    lv_obj_set_style_text_font(scale_lbl_mult, &lv_font_montserrat_16, 0);
    lv_obj_align(scale_lbl_mult, LV_ALIGN_CENTER, -60, 30);

    scale_lbl_val = lv_label_create(parent);
    lv_obj_set_style_text_font(scale_lbl_val, &lv_font_montserrat_16, 0);
    lv_obj_align(scale_lbl_val, LV_ALIGN_CENTER, 60, 30);

    scale_lbl_res = lv_label_create(parent);
    lv_obj_set_style_text_font(scale_lbl_res, &lv_font_montserrat_24, 0);
    lv_obj_align(scale_lbl_res, LV_ALIGN_CENTER, 0, 100);
}

// ============================================================================
// Update
// ============================================================================

void screen_scaler_update(void) {
    char buf_mult[16];
    char buf_val[24];
    char buf_res[32];

    float factor = scale_multipliers[scale_mult_idx];
    snprintf(buf_mult, sizeof(buf_mult), "Mult:\n%.2fx", factor);
    snprintf(buf_val, sizeof(buf_val), "Qty:\n%.0fg", scale_input_val);
    
    float scaled = scale_input_val * factor;
    snprintf(buf_res, sizeof(buf_res), "Scaled: %.0fg", scaled);

    lv_label_set_text(scale_lbl_mult, buf_mult);
    lv_label_set_text(scale_lbl_val, buf_val);
    lv_label_set_text(scale_lbl_res, buf_res);

    // Focus style highlights
    lv_obj_set_style_text_color(scale_lbl_mult, (scale_focus == 0) ? lv_color_make(255, 87, 34) : lv_color_white(), 0);
    lv_obj_set_style_text_color(scale_lbl_val, (scale_focus == 1) ? lv_color_make(255, 87, 34) : lv_color_white(), 0);
}

// ============================================================================
// Input Handlers
// ============================================================================

void screen_scaler_handle_rotation(int8_t direction) {
    if (scale_focus == 0) {  // Multiplier
        scale_mult_idx += direction;
        if (scale_mult_idx < 0) scale_mult_idx = MULT_COUNT - 1;
        if (scale_mult_idx >= MULT_COUNT) scale_mult_idx = 0;
    } else {  // Value
        scale_input_val += direction * 5.0;  // increments of 5g
        if (scale_input_val < 0.0) scale_input_val = 0.0;
        if (scale_input_val > 9999.0) scale_input_val = 9999.0;
    }
}

void screen_scaler_handle_click(void) {
    scale_focus = (scale_focus + 1) % 2;
}
