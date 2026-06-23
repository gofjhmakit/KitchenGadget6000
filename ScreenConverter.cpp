#include "ScreenConverter.h"
#include "UIManager.h"
#include <Arduino.h>

// ============================================================================
// Converter Screen State
// ============================================================================

static lv_obj_t* conv_lbl_title = NULL;
static lv_obj_t* conv_lbl_cat = NULL;
static lv_obj_t* conv_lbl_from = NULL;
static lv_obj_t* conv_lbl_val = NULL;
static lv_obj_t* conv_lbl_to = NULL;
static lv_obj_t* conv_lbl_result = NULL;

static int conv_focus = 0;         // 0=category, 1=from, 2=value, 3=to
static int conv_category = 0;      // CAT_VOLUME, CAT_WEIGHT, CAT_TEMP
static int conv_from_idx = 0;
static int conv_to_idx = 0;
static float conv_input_val = 1.0;

// Conversion data
const char* cat_names[CAT_COUNT] = {"VOLUME", "WEIGHT", "TEMPERATURE"};

const char* volume_units[] = {"ml", "Cups", "fl.oz", "tbsp", "tsp"};
const float volume_factors[] = {1.0, 236.588, 29.5735, 14.7868, 4.9289};
#define VOL_UNIT_COUNT 5

const char* weight_units[] = {"g", "oz"};
const float weight_factors[] = {1.0, 28.3495};
#define WGT_UNIT_COUNT 2

#define CAT_COUNT 2  // Only VOLUME and WEIGHT (no temperature)

// ============================================================================
// Initialization
// ============================================================================

void screen_converter_init(lv_obj_t* parent) {
    lv_obj_t* header = lv_obj_create(parent);
    lv_obj_set_size(header, 240, 30);
    lv_obj_align(header, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_make(20, 20, 20), 0);
    lv_obj_set_style_border_width(header, 0, 0);

    conv_lbl_title = lv_label_create(header);
    lv_obj_set_style_text_font(conv_lbl_title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(conv_lbl_title, lv_color_make(255, 193, 7), 0);
    lv_label_set_text(conv_lbl_title, "UNIT CONVERTER");
    lv_obj_align(conv_lbl_title, LV_ALIGN_CENTER, 0, 0);

    conv_lbl_cat = lv_label_create(parent);
    lv_obj_set_style_text_font(conv_lbl_cat, &lv_font_montserrat_18, 0);
    lv_obj_align(conv_lbl_cat, LV_ALIGN_TOP_MID, 0, 40);

    conv_lbl_from = lv_label_create(parent);
    lv_obj_set_style_text_font(conv_lbl_from, &lv_font_montserrat_12, 0);
    lv_obj_align(conv_lbl_from, LV_ALIGN_CENTER, -60, -20);

    conv_lbl_to = lv_label_create(parent);
    lv_obj_set_style_text_font(conv_lbl_to, &lv_font_montserrat_12, 0);
    lv_obj_align(conv_lbl_to, LV_ALIGN_CENTER, 60, -20);

    conv_lbl_val = lv_label_create(parent);
    lv_obj_set_style_text_font(conv_lbl_val, &lv_font_montserrat_20, 0);
    lv_obj_align(conv_lbl_val, LV_ALIGN_CENTER, -60, 15);

    conv_lbl_result = lv_label_create(parent);
    lv_obj_set_style_text_font(conv_lbl_result, &lv_font_montserrat_20, 0);
    lv_obj_align(conv_lbl_result, LV_ALIGN_CENTER, 60, 15);
}

// ============================================================================
// Update
// ============================================================================

void screen_converter_update(void) {
    lv_label_set_text(conv_lbl_cat, cat_names[conv_category]);
    
    char buf_from[24];
    char buf_to[24];
    char buf_val[16];
    char buf_res[32];

    // Read categories
    if (conv_category == CAT_VOLUME) {
        snprintf(buf_from, sizeof(buf_from), "From: %s", volume_units[conv_from_idx]);
        snprintf(buf_to, sizeof(buf_to), "To: %s", volume_units[conv_to_idx]);
        
        // Formulate conversion
        float ml = conv_input_val * volume_factors[conv_from_idx];
        float result = ml / volume_factors[conv_to_idx];
        
        snprintf(buf_val, sizeof(buf_val), "%.1f", conv_input_val);
        snprintf(buf_res, sizeof(buf_res), "= %.2f %s", result, volume_units[conv_to_idx]);
    } 
    else if (conv_category == CAT_WEIGHT) {
        snprintf(buf_from, sizeof(buf_from), "From: %s", weight_units[conv_from_idx]);
        snprintf(buf_to, sizeof(buf_to), "To: %s", weight_units[conv_to_idx]);
        
        float g = conv_input_val * weight_factors[conv_from_idx];
        float result = g / weight_factors[conv_to_idx];
        
        snprintf(buf_val, sizeof(buf_val), "%.1f", conv_input_val);
        snprintf(buf_res, sizeof(buf_res), "= %.2f %s", result, weight_units[conv_to_idx]);
    } 
    
    // Temperature conversion removed - using metric system only (Celsius)

    lv_label_set_text(conv_lbl_from, buf_from);
    lv_label_set_text(conv_lbl_to, buf_to);
    lv_label_set_text(conv_lbl_val, buf_val);
    lv_label_set_text(conv_lbl_result, buf_res);

    // Apply color focus highlights
    lv_obj_set_style_text_color(conv_lbl_cat, (conv_focus == 0) ? lv_color_make(255, 87, 34) : lv_color_make(255, 193, 7), 0);
    lv_obj_set_style_text_color(conv_lbl_from, (conv_focus == 1) ? lv_color_make(255, 87, 34) : lv_color_white(), 0);
    lv_obj_set_style_text_color(conv_lbl_val, (conv_focus == 2) ? lv_color_make(255, 87, 34) : lv_color_white(), 0);
    lv_obj_set_style_text_color(conv_lbl_to, (conv_focus == 3) ? lv_color_make(255, 87, 34) : lv_color_white(), 0);
}

// ============================================================================
// Input Handlers
// ============================================================================

void screen_converter_handle_rotation(int8_t direction) {
    if (conv_focus == 0) {  // Category
        conv_category += direction;
        if (conv_category < 0) conv_category = CAT_COUNT - 1;
        if (conv_category >= CAT_COUNT) conv_category = 0;
        conv_from_idx = 0;
        conv_to_idx = 0;
        conv_input_val = 1.0;
    } 
    else if (conv_focus == 1) {  // From Unit
        int count = (conv_category == CAT_VOLUME) ? VOL_UNIT_COUNT : WGT_UNIT_COUNT;
        conv_from_idx += direction;
        if (conv_from_idx < 0) conv_from_idx = count - 1;
        if (conv_from_idx >= count) conv_from_idx = 0;
    } 
    else if (conv_focus == 2) {  // Value
        conv_input_val += direction * 0.5;
        if (conv_input_val < 0.0) conv_input_val = 0.0;
        if (conv_input_val > 999.0) conv_input_val = 999.0;
    } 
    else if (conv_focus == 3) {  // To Unit
        int count = (conv_category == CAT_VOLUME) ? VOL_UNIT_COUNT : WGT_UNIT_COUNT;
        conv_to_idx += direction;
        if (conv_to_idx < 0) conv_to_idx = count - 1;
        if (conv_to_idx >= count) conv_to_idx = 0;
    }
}

void screen_converter_handle_click(void) {
    conv_focus = (conv_focus + 1) % 4;
}
