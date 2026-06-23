#include "ScreenHydration.h"
#include "UIManager.h"
#include <Arduino.h>

// ============================================================================
// Hydration Screen State
// ============================================================================

static lv_obj_t* hyd_lbl_flour = NULL;
static lv_obj_t* hyd_lbl_percent = NULL;
static lv_obj_t* hyd_lbl_water = NULL;
static lv_obj_t* hyd_lbl_starter = NULL;
static lv_obj_t* hyd_lbl_salt = NULL;

static int hyd_focus = 0;           // 0=flour, 1=hydration %
static float hyd_flour_weight = 500.0;
static int hyd_percent = 70;

// ============================================================================
// Initialization
// ============================================================================

void screen_hydration_init(lv_obj_t* parent) {
    lv_obj_t* header = lv_obj_create(parent);
    lv_obj_set_size(header, 240, 30);
    lv_obj_align(header, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_make(20, 20, 20), 0);
    lv_obj_set_style_border_width(header, 0, 0);

    lv_obj_t* title = lv_label_create(header);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_make(255, 193, 7), 0);
    lv_label_set_text(title, "BAKER'S HYDRATION");
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    hyd_lbl_flour = lv_label_create(parent);
    lv_obj_set_style_text_font(hyd_lbl_flour, &lv_font_montserrat_12, 0);
    lv_obj_align(hyd_lbl_flour, LV_ALIGN_TOP_MID, 0, 40);

    hyd_lbl_percent = lv_label_create(parent);
    lv_obj_set_style_text_font(hyd_lbl_percent, &lv_font_montserrat_12, 0);
    lv_obj_align(hyd_lbl_percent, LV_ALIGN_TOP_MID, 0, 65);

    hyd_lbl_water = lv_label_create(parent);
    lv_obj_set_style_text_font(hyd_lbl_water, &lv_font_montserrat_20, 0);
    lv_obj_align(hyd_lbl_water, LV_ALIGN_CENTER, 0, 20);

    hyd_lbl_starter = lv_label_create(parent);
    lv_obj_set_style_text_font(hyd_lbl_starter, &lv_font_montserrat_14, 0);
    lv_obj_align(hyd_lbl_starter, LV_ALIGN_CENTER, 0, 70);

    hyd_lbl_salt = lv_label_create(parent);
    lv_obj_set_style_text_font(hyd_lbl_salt, &lv_font_montserrat_14, 0);
    lv_obj_align(hyd_lbl_salt, LV_ALIGN_CENTER, 0, 100);
}

// ============================================================================
// Update
// ============================================================================

void screen_hydration_update(void) {
    char buf[32];

    snprintf(buf, sizeof(buf), "Flour: %.0fg", hyd_flour_weight);
    lv_label_set_text(hyd_lbl_flour, buf);
    lv_obj_set_style_text_color(hyd_lbl_flour, (hyd_focus == 0) ? lv_color_make(255, 87, 34) : lv_color_white(), 0);

    snprintf(buf, sizeof(buf), "Hydration: %d%%", hyd_percent);
    lv_label_set_text(hyd_lbl_percent, buf);
    lv_obj_set_style_text_color(hyd_lbl_percent, (hyd_focus == 1) ? lv_color_make(255, 87, 34) : lv_color_white(), 0);

    float water_weight = (hyd_flour_weight * hyd_percent) / 100.0;
    snprintf(buf, sizeof(buf), "Water: %.0fg", water_weight);
    lv_label_set_text(hyd_lbl_water, buf);

    float starter_weight = hyd_flour_weight * 0.20;  // 20% starter
    snprintf(buf, sizeof(buf), "Starter: %.0fg", starter_weight);
    lv_label_set_text(hyd_lbl_starter, buf);

    float salt_weight = (hyd_flour_weight * 2.0) / 100.0;  // 2% salt
    snprintf(buf, sizeof(buf), "Salt: %.0fg", salt_weight);
    lv_label_set_text(hyd_lbl_salt, buf);
}

// ============================================================================
// Input Handlers
// ============================================================================

void screen_hydration_handle_rotation(int8_t direction) {
    if (hyd_focus == 0) {  // Flour
        hyd_flour_weight += direction * 10.0;  // steps of 10g
        if (hyd_flour_weight < 0.0) hyd_flour_weight = 0.0;
        if (hyd_flour_weight > 5000.0) hyd_flour_weight = 5000.0;
    } else {  // Hydration %
        hyd_percent += direction;
        if (hyd_percent < 10) hyd_percent = 10;
        if (hyd_percent > 100) hyd_percent = 100;
    }
}

void screen_hydration_handle_click(void) {
    hyd_focus = (hyd_focus + 1) % 2;
}
