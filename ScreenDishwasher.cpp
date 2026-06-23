#include "ScreenDishwasher.h"
#include "UIManager.h"
#include <Arduino.h>

// ============================================================================
// Dishwasher Screen State
// ============================================================================

static lv_obj_t* dw_arc_prog = NULL;
static lv_obj_t* dw_lbl_status = NULL;
static lv_obj_t* dw_lbl_time = NULL;
static lv_obj_t* dw_lbl_details = NULL;

static bool dw_sim_mode = true;
static int dw_total_mins = 135;
static int dw_remaining_mins = 82;
static float dw_elapsed_sec = 0.0;
static char dw_status_str[32] = "Washing...";
static char dw_prog_str[32] = "Eco 50";

// ============================================================================
// Initialization
// ============================================================================

void screen_dishwasher_init(lv_obj_t* parent) {
    lv_obj_t* header = lv_obj_create(parent);
    lv_obj_set_size(header, 240, 30);
    lv_obj_align(header, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_make(20, 20, 20), 0);
    lv_obj_set_style_border_width(header, 0, 0);

    lv_obj_t* title = lv_label_create(header);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_make(255, 193, 7), 0);
    lv_label_set_text(title, "IoT DISHWASHER");
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    // Progress arc
    dw_arc_prog = lv_arc_create(parent);
    lv_obj_set_size(dw_arc_prog, 180, 180);
    lv_obj_align(dw_arc_prog, LV_ALIGN_CENTER, 0, 20);
    lv_arc_set_bg_angles(dw_arc_prog, 0, 360);
    lv_obj_remove_style(dw_arc_prog, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(dw_arc_prog, 4, LV_PART_MAIN);
    lv_obj_set_style_arc_color(dw_arc_prog, lv_color_make(40, 40, 40), LV_PART_MAIN);
    lv_obj_set_style_arc_width(dw_arc_prog, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(dw_arc_prog, lv_color_make(0, 150, 136), LV_PART_INDICATOR);
    lv_arc_set_range(dw_arc_prog, 0, 360);

    dw_lbl_status = lv_label_create(parent);
    lv_obj_set_style_text_font(dw_lbl_status, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(dw_lbl_status, lv_color_white(), 0);
    lv_obj_align(dw_lbl_status, LV_ALIGN_CENTER, 0, -50);

    dw_lbl_time = lv_label_create(parent);
    lv_obj_set_style_text_font(dw_lbl_time, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(dw_lbl_time, lv_color_make(33, 150, 243), 0);
    lv_obj_align(dw_lbl_time, LV_ALIGN_CENTER, 0, 10);

    dw_lbl_details = lv_label_create(parent);
    lv_obj_set_style_text_font(dw_lbl_details, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(dw_lbl_details, lv_color_make(150, 150, 150), 0);
    lv_obj_align(dw_lbl_details, LV_ALIGN_BOTTOM_MID, 0, -20);
}

// ============================================================================
// Update
// ============================================================================

void screen_dishwasher_update(void) {
    if (dw_sim_mode) {
        // Simulation mode - advance time
        dw_elapsed_sec += 0.02;  // Increment by 20ms per frame (50Hz)
        if (dw_elapsed_sec >= 1.0) {
            dw_elapsed_sec -= 1.0;
            if (dw_remaining_mins > 0) {
                dw_remaining_mins--;
            }
        }
    }

    // Update progress arc
    uint32_t progress = 0;
    if (dw_total_mins > 0) {
        progress = ((dw_total_mins - dw_remaining_mins) * 360) / dw_total_mins;
    }
    lv_arc_set_value(dw_arc_prog, progress);

    // Update time display
    char buf_time[16];
    if (dw_remaining_mins > 0) {
        int hours = dw_remaining_mins / 60;
        int mins = dw_remaining_mins % 60;
        if (hours > 0) {
            snprintf(buf_time, sizeof(buf_time), "%d:%02d", hours, mins);
        } else {
            snprintf(buf_time, sizeof(buf_time), "%d min", mins);
        }
    } else {
        snprintf(buf_time, sizeof(buf_time), "Done");
    }
    lv_label_set_text(dw_lbl_time, buf_time);

    // Status
    lv_label_set_text(dw_lbl_status, dw_status_str);

    // Program details
    char buf_details[48];
    snprintf(buf_details, sizeof(buf_details), "%s | %d/%d min", dw_prog_str, dw_total_mins - dw_remaining_mins, dw_total_mins);
    lv_label_set_text(dw_lbl_details, buf_details);
}

// ============================================================================
// Input Handlers
// ============================================================================

void screen_dishwasher_handle_rotation(int8_t direction) {
    // Optional: Adjust remaining time in simulation mode
    if (dw_sim_mode) {
        dw_remaining_mins += direction * 5;
        if (dw_remaining_mins < 0) dw_remaining_mins = 0;
        if (dw_remaining_mins > dw_total_mins) dw_remaining_mins = dw_total_mins;
    }
}

void screen_dishwasher_handle_click(void) {
    // Toggles dishwasher simulation vs. live mode
    dw_sim_mode = !dw_sim_mode;
    if (!dw_sim_mode) {
        // If live but no wifi, default to some static text
        snprintf(dw_status_str, sizeof(dw_status_str), "Syncing...");
    } else {
        snprintf(dw_status_str, sizeof(dw_status_str), "Washing...");
    }
}

// ============================================================================
// External Update Function (called from main sketch via Home Connect API)
// ============================================================================

void screen_dishwasher_set_data(const char* status, const char* program, int remaining, int total) {
    if (status) strncpy(dw_status_str, status, sizeof(dw_status_str) - 1);
    if (program) strncpy(dw_prog_str, program, sizeof(dw_prog_str) - 1);
    dw_remaining_mins = remaining;
    dw_total_mins = total > 0 ? total : 1;  // Avoid division by zero
    dw_sim_mode = false;  // Switch to live mode when data is received
}
