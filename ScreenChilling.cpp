#include "ScreenChilling.h"
#include "UIManager.h"
#include <Arduino.h>

// ============================================================================
// Chilling Screen State
// ============================================================================

static lv_obj_t* chill_lbl_container = NULL;
static lv_obj_t* chill_lbl_method = NULL;
static lv_obj_t* chill_lbl_temp = NULL;
static lv_obj_t* chill_lbl_btn = NULL;
static lv_obj_t* chill_lbl_res = NULL;

static int chill_focus = 0;         // 0=container, 1=method, 2=temp, 3=start
static int chill_container_idx = 0; // 0=Can, 1=Glass, 2=Plastic
static int chill_method_idx = 0;    // 0=Freezer, 1=Fridge
static int chill_temp_idx = 0;      // 0=Room, 1=Warm/Pantry

const char* chill_containers[] = {"Can", "Glass Bottle", "Plastic Bottle"};
const char* chill_methods[] = {"Freezer", "Refrigerator"};
const char* chill_temps[] = {"Room Temp", "Warm (Pantry)"};

// ============================================================================
// Initialization
// ============================================================================

void screen_chilling_init(lv_obj_t* parent) {
    lv_obj_t* header = lv_obj_create(parent);
    lv_obj_set_size(header, 240, 30);
    lv_obj_align(header, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_make(20, 20, 20), 0);
    lv_obj_set_style_border_width(header, 0, 0);

    lv_obj_t* title = lv_label_create(header);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_make(255, 193, 7), 0);
    lv_label_set_text(title, "DRINK CHILLER");
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    chill_lbl_container = lv_label_create(parent);
    lv_obj_set_style_text_font(chill_lbl_container, &lv_font_montserrat_12, 0);
    lv_obj_align(chill_lbl_container, LV_ALIGN_TOP_MID, 0, 40);

    chill_lbl_method = lv_label_create(parent);
    lv_obj_set_style_text_font(chill_lbl_method, &lv_font_montserrat_12, 0);
    lv_obj_align(chill_lbl_method, LV_ALIGN_TOP_MID, 0, 65);

    chill_lbl_temp = lv_label_create(parent);
    lv_obj_set_style_text_font(chill_lbl_temp, &lv_font_montserrat_12, 0);
    lv_obj_align(chill_lbl_temp, LV_ALIGN_TOP_MID, 0, 90);

    chill_lbl_res = lv_label_create(parent);
    lv_obj_set_style_text_font(chill_lbl_res, &lv_font_montserrat_24, 0);
    lv_obj_align(chill_lbl_res, LV_ALIGN_CENTER, 0, 100);

    chill_lbl_btn = lv_label_create(parent);
    lv_obj_set_style_text_font(chill_lbl_btn, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(chill_lbl_btn, lv_color_make(255, 193, 7), 0);
    lv_label_set_text(chill_lbl_btn, "Click to Start Timer");
    lv_obj_align(chill_lbl_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
}

// ============================================================================
// Update
// ============================================================================

void screen_chilling_update(void) {
    char buf[24];

    snprintf(buf, sizeof(buf), "Container: %s", chill_containers[chill_container_idx]);
    lv_label_set_text(chill_lbl_container, buf);
    lv_obj_set_style_text_color(chill_lbl_container, (chill_focus == 0) ? lv_color_make(255, 87, 34) : lv_color_white(), 0);

    snprintf(buf, sizeof(buf), "Method: %s", chill_methods[chill_method_idx]);
    lv_label_set_text(chill_lbl_method, buf);
    lv_obj_set_style_text_color(chill_lbl_method, (chill_focus == 1) ? lv_color_make(255, 87, 34) : lv_color_white(), 0);

    snprintf(buf, sizeof(buf), "Start: %s", chill_temps[chill_temp_idx]);
    lv_label_set_text(chill_lbl_temp, buf);
    lv_obj_set_style_text_color(chill_lbl_temp, (chill_focus == 2) ? lv_color_make(255, 87, 34) : lv_color_white(), 0);

    // Calculate time
    int base_time = (chill_container_idx == 0) ? 18 : ((chill_container_idx == 1) ? 35 : 50);
    float method_mult = (chill_method_idx == 0) ? 1.0 : 3.5;
    float temp_mult = (chill_temp_idx == 0) ? 1.0 : 1.3;
    int total_mins = (int)(base_time * method_mult * temp_mult);

    snprintf(buf, sizeof(buf), "~%d min", total_mins);
    lv_label_set_text(chill_lbl_res, buf);
}

// ============================================================================
// Input Handlers
// ============================================================================

void screen_chilling_handle_rotation(int8_t direction) {
    if (chill_focus == 0) {  // Container
        chill_container_idx += direction;
        if (chill_container_idx < 0) chill_container_idx = 2;
        if (chill_container_idx > 2) chill_container_idx = 0;
    } else if (chill_focus == 1) {  // Method
        chill_method_idx += direction;
        if (chill_method_idx < 0) chill_method_idx = 1;
        if (chill_method_idx > 1) chill_method_idx = 0;
    } else if (chill_focus == 2) {  // Temperature
        chill_temp_idx += direction;
        if (chill_temp_idx < 0) chill_temp_idx = 1;
        if (chill_temp_idx > 1) chill_temp_idx = 0;
    }
}

void screen_chilling_handle_click(void) {
    if (chill_focus == 3) {
        // Calculate and launch timer
        int base_time = (chill_container_idx == 0) ? 18 : ((chill_container_idx == 1) ? 35 : 50);
        float method_mult = (chill_method_idx == 0) ? 1.0 : 3.5;
        float temp_mult = (chill_temp_idx == 0) ? 1.0 : 1.3;
        uint32_t total_sec = (uint32_t)(base_time * method_mult * temp_mult) * 60;

        int free_slot = -1;
        for (int i = 0; i < TIMER_SLOT_COUNT; i++) {
            if (!g_timers[i].active && g_timers[i].remaining == 0 && !g_timers[i].expired) {
                free_slot = i;
                break;
            }
        }
        if (free_slot == -1) free_slot = 0;

        g_timers[free_slot].duration = total_sec;
        g_timers[free_slot].remaining = total_sec;
        g_timers[free_slot].active = true;
        g_timers[free_slot].expired = false;
        g_timers[free_slot].name = "Chilling";
        snprintf(g_timers[free_slot].emoji, sizeof(g_timers[free_slot].emoji), "🧊");
        
        g_active_timer_slot = free_slot;
        ui_manager_transition_to_mode(MODE_TIMERS);
    } else {
        chill_focus = (chill_focus + 1) % 4;
    }
}
