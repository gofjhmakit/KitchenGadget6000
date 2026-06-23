#include "ScreenLauncher.h"
#include "UIManager.h"
#include <Arduino.h>

// ============================================================================
// Launcher Screen State
// ============================================================================

static lv_obj_t* launcher_arc = NULL;
static lv_obj_t* launcher_title_lbl = NULL;
static lv_obj_t* launcher_desc_lbl = NULL;
static lv_obj_t* launcher_time_lbl = NULL;
static lv_obj_t* launcher_wifi_lbl = NULL;
static int launcher_selected_idx = 0;

const char* menu_names[MODE_COUNT - 1] = {
    "1. MULTI-TIMER",
    "2. UNIT CONVERTER",
    "3. PORTION SCALER",
    "4. BREW ASSISTANT",
    "5. BAKER'S BREAD",
    "6. DRINK CHILLER",
    "7. MEAT TEMPS",
    "8. IoT DISHWASHER"
};

const char* menu_descs[MODE_COUNT - 1] = {
    "Run up to 3 concentric timers",
    "Quick kitchen unit conversion",
    "Scale ingredient multipliers",
    "Tea and egg cooking presets",
    "Sourdough hydration ratio tool",
    "Calculates fast drink cooling",
    "Meat doneness internal temps",
    "HomeConnect dishwasher monitor"
};

// ============================================================================
// Initialization
// ============================================================================

void screen_launcher_init(lv_obj_t* parent) {
    // Arc display
    launcher_arc = lv_arc_create(parent);
    lv_obj_set_size(launcher_arc, 220, 220);
    lv_obj_align(launcher_arc, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_bg_angles(launcher_arc, 0, 360);
    lv_obj_remove_style(launcher_arc, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(launcher_arc, 4, LV_PART_MAIN);
    lv_obj_set_style_arc_color(launcher_arc, lv_color_make(40, 40, 40), LV_PART_MAIN);
    lv_obj_set_style_arc_width(launcher_arc, 6, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(launcher_arc, lv_color_make(255, 87, 34), LV_PART_INDICATOR); // Coral
    lv_arc_set_range(launcher_arc, 0, MODE_COUNT - 2);

    // Title label
    launcher_title_lbl = lv_label_create(parent);
    lv_obj_set_style_text_font(launcher_title_lbl, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(launcher_title_lbl, lv_color_make(255, 193, 7), 0); // Amber/Yellow
    lv_obj_align(launcher_title_lbl, LV_ALIGN_CENTER, 0, -10);

    // Description label
    launcher_desc_lbl = lv_label_create(parent);
    lv_obj_set_style_text_font(launcher_desc_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(launcher_desc_lbl, lv_color_make(200, 200, 200), 0);
    lv_obj_set_width(launcher_desc_lbl, 160);
    lv_obj_set_style_text_align(launcher_desc_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(launcher_desc_lbl, LV_ALIGN_CENTER, 0, 35);

    // Time label
    launcher_time_lbl = lv_label_create(parent);
    lv_obj_set_style_text_font(launcher_time_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(launcher_time_lbl, lv_color_white(), 0);
    lv_obj_align(launcher_time_lbl, LV_ALIGN_TOP_MID, 0, 22);

    // WiFi status label
    launcher_wifi_lbl = lv_label_create(parent);
    lv_obj_set_style_text_font(launcher_wifi_lbl, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(launcher_wifi_lbl, lv_color_make(128, 128, 128), 0);
    lv_obj_align(launcher_wifi_lbl, LV_ALIGN_BOTTOM_MID, 0, -22);
}

// ============================================================================
// Update
// ============================================================================

void screen_launcher_update(void) {
    lv_arc_set_value(launcher_arc, launcher_selected_idx);
    lv_label_set_text(launcher_title_lbl, menu_names[launcher_selected_idx]);
    lv_label_set_text(launcher_desc_lbl, menu_descs[launcher_selected_idx]);

    // Format time
    char time_str[10];
    uint32_t h = (g_current_time_sec / 3600) % 24;
    uint32_t m = (g_current_time_sec % 3600) / 60;
    snprintf(time_str, sizeof(time_str), "%02u:%02u", h, m);
    lv_label_set_text(launcher_time_lbl, time_str);

    // Format WiFi status
    char wifi_str[36];
    if (g_wifi_connected) {
        snprintf(wifi_str, sizeof(wifi_str), LV_SYMBOL_WIFI " %s", g_wifi_ip_str);
    } else {
        snprintf(wifi_str, sizeof(wifi_str), LV_SYMBOL_WARNING " Offline");
    }
    lv_label_set_text(launcher_wifi_lbl, wifi_str);
}

// ============================================================================
// Input Handlers
// ============================================================================

void screen_launcher_handle_rotation(int8_t direction) {
    launcher_selected_idx += direction;
    if (launcher_selected_idx < 0) {
        launcher_selected_idx = MODE_COUNT - 2;
    } else if (launcher_selected_idx > MODE_COUNT - 2) {
        launcher_selected_idx = 0;
    }
}

void screen_launcher_handle_click(void) {
    // Click launches the selected screen (maps 0-7 menu to MODE_TIMERS to MODE_DISHWASHER)
    KitchenMode target = (KitchenMode)(launcher_selected_idx + 1);
    if (target < MODE_COUNT) {
        ui_manager_transition_to_mode(target);
    }
}
