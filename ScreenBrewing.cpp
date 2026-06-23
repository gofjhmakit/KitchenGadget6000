#include "ScreenBrewing.h"
#include "TeaSteeingTimes.h"
#include "UIManager.h"
#include <Arduino.h>
#include <cstdio>

// ============================================================================
// Brewing Screen State
// ============================================================================

static lv_obj_t* brew_lbl_preset = NULL;
static lv_obj_t* brew_lbl_time = NULL;
static lv_obj_t* brew_lbl_temp = NULL;
static int brew_selected_idx = 0;

// ============================================================================
// Initialization
// ============================================================================

void screen_brewing_init(lv_obj_t* parent) {
    lv_obj_t* header = lv_obj_create(parent);
    lv_obj_set_size(header, 240, 30);
    lv_obj_align(header, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_make(20, 20, 20), 0);
    lv_obj_set_style_border_width(header, 0, 0);

    lv_obj_t* title = lv_label_create(header);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_make(255, 193, 7), 0);
    lv_label_set_text(title, "BREW PRESETS");
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    brew_lbl_preset = lv_label_create(parent);
    lv_obj_set_style_text_font(brew_lbl_preset, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(brew_lbl_preset, lv_color_make(255, 87, 34), 0);
    lv_obj_align(brew_lbl_preset, LV_ALIGN_CENTER, 0, 50);

    brew_lbl_time = lv_label_create(parent);
    lv_obj_set_style_text_font(brew_lbl_time, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(brew_lbl_time, lv_color_white(), 0);
    lv_obj_align(brew_lbl_time, LV_ALIGN_CENTER, 0, 100);
    
    brew_lbl_temp = lv_label_create(parent);
    lv_obj_set_style_text_font(brew_lbl_temp, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(brew_lbl_temp, lv_color_make(100, 200, 255), 0);
    lv_obj_align(brew_lbl_temp, LV_ALIGN_CENTER, 0, 150);
}

// ============================================================================
// Update
// ============================================================================

void screen_brewing_update(void) {
    lv_label_set_text(brew_lbl_preset, get_tea_name(brew_selected_idx));
    
    char buf_time[16];
    uint16_t steep_sec = get_tea_steep_time(brew_selected_idx);
    format_seconds(steep_sec, buf_time, sizeof(buf_time));
    lv_label_set_text(brew_lbl_time, buf_time);
    
    // Show water temperature hint
    char buf_temp[32];
    uint16_t temp_c = tea_presets[brew_selected_idx].water_temp_c;
    snprintf(buf_temp, sizeof(buf_temp), "Water: %d°C", temp_c);
    lv_label_set_text(brew_lbl_temp, buf_temp);
}

// ============================================================================
// Input Handlers
// ============================================================================

void screen_brewing_handle_rotation(int8_t direction) {
    brew_selected_idx += direction;
    if (brew_selected_idx < 0) brew_selected_idx = TEA_PRESET_COUNT - 1;
    if (brew_selected_idx >= TEA_PRESET_COUNT) brew_selected_idx = 0;
}

void screen_brewing_handle_click(void) {
    // Launch timer with this preset
    int free_slot = -1;
    for (int i = 0; i < TIMER_SLOT_COUNT; i++) {
        if (!g_timers[i].active && g_timers[i].remaining == 0 && !g_timers[i].expired) {
            free_slot = i;
            break;
        }
    }
    if (free_slot == -1) free_slot = 0;

    uint16_t steep_sec = get_tea_steep_time(brew_selected_idx);
    g_timers[free_slot].duration = steep_sec;
    g_timers[free_slot].remaining = steep_sec;
    g_timers[free_slot].active = true;
    g_timers[free_slot].expired = false;
    g_timers[free_slot].name = get_tea_name(brew_selected_idx);
    snprintf(g_timers[free_slot].emoji, sizeof(g_timers[free_slot].emoji), "☕");
    
    g_active_timer_slot = free_slot;
    ui_manager_transition_to_mode(MODE_TIMERS);
}
