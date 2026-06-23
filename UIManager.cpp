#include "UIManager.h"
#include <Arduino.h>
#include <time.h>

// ============================================================================
// Global State (accessible to all screens)
// ============================================================================

KitchenMode g_current_mode = MODE_LAUNCHER;
lv_obj_t* g_screens[MODE_COUNT] = {NULL};

// Timer management
KitchenTimer g_timers[TIMER_SLOT_COUNT] = {
    {0, 0, false, false, {0}, "Timer 1", "?"},
    {0, 0, false, false, {0}, "Timer 2", "?"},
    {0, 0, false, false, {0}, "Timer 3", "?"},
    {0, 0, false, false, {0}, "Timer 4", "?"},
    {0, 0, false, false, {0}, "Timer 5", "?"}
};
int g_active_timer_slot = 0;
static float g_timer_second_accum[TIMER_SLOT_COUNT] = {0.0f};

// WiFi status
bool g_wifi_connected = false;
char g_wifi_ip_str[16] = "No WiFi";

// Time
uint32_t g_current_time_sec = 0;

// LED state
int g_led_state = 0;  // 0=idle, 1=timer pulsing, 2=alarm flashing
uint8_t g_led_color_r = 255;
uint8_t g_led_color_g = 140;
uint8_t g_led_color_b = 60;

// ============================================================================
// Helper Functions
// ============================================================================

void format_seconds(uint32_t total_sec, char* buf, size_t size) {
    uint32_t h = total_sec / 3600;
    uint32_t m = (total_sec % 3600) / 60;
    uint32_t s = total_sec % 60;
    if (h > 0) {
        snprintf(buf, size, "%02u:%02u:%02u", h, m, s);
    } else {
        snprintf(buf, size, "%02u:%02u", m, s);
    }
}

static uint8_t lerp_u8(uint8_t a, uint8_t b, float t) {
    if (t <= 0.0f) return a;
    if (t >= 1.0f) return b;
    return (uint8_t)(a + (b - a) * t);
}

static void color_for_progress(float progress, uint8_t* r, uint8_t* g, uint8_t* b) {
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    // Green -> Yellow -> Orange -> Red
    if (progress < 0.33f) {
        float t = progress / 0.33f;
        *r = lerp_u8(0, 255, t);
        *g = 255;
        *b = 0;
    } else if (progress < 0.66f) {
        float t = (progress - 0.33f) / 0.33f;
        *r = 255;
        *g = lerp_u8(255, 140, t);
        *b = 0;
    } else {
        float t = (progress - 0.66f) / 0.34f;
        *r = 255;
        *g = lerp_u8(140, 0, t);
        *b = 0;
    }
}

// ============================================================================
// Screen Transition
// ============================================================================

void ui_manager_transition_to_mode(KitchenMode new_mode) {
    if (new_mode >= MODE_COUNT) return;
    if (new_mode == g_current_mode) return;
    
    // Hide current screen
    if (g_screens[g_current_mode] != NULL) {
        lv_scr_load(g_screens[new_mode]);
    }
    
    // Load new screen
    g_current_mode = new_mode;
    if (g_screens[new_mode] != NULL) {
        lv_scr_load(g_screens[new_mode]);
    }
}

// ============================================================================
// UI Manager API
// ============================================================================

void ui_manager_init(void) {
    // Initialize timer colors
    const lv_color_t timer_colors[TIMER_SLOT_COUNT] = {
        lv_color_make(255, 87, 34),   // Coral
        lv_color_make(0, 150, 136),   // Teal
        lv_color_make(33, 150, 243),  // Blue
        lv_color_make(255, 193, 7),   // Amber
        lv_color_make(156, 39, 176)   // Purple
    };
    for (int i = 0; i < TIMER_SLOT_COUNT; i++) {
        g_timers[i].color = timer_colors[i];
    }

    // Create all screen objects
    for (int i = 0; i < MODE_COUNT; i++) {
        g_screens[i] = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(g_screens[i], lv_color_black(), 0);
        lv_obj_set_style_bg_opa(g_screens[i], LV_OPA_COVER, 0);
    }

    // Initialize each screen via their respective modules
    screen_launcher_init(g_screens[MODE_LAUNCHER]);
    screen_timers_init(g_screens[MODE_TIMERS]);
    screen_converter_init(g_screens[MODE_CONVERTER]);
    screen_scaler_init(g_screens[MODE_SCALER]);
    screen_brewing_init(g_screens[MODE_BREWING]);
    screen_hydration_init(g_screens[MODE_HYDRATION]);
    screen_chilling_init(g_screens[MODE_CHILLING]);
    screen_meat_init(g_screens[MODE_MEAT]);
    screen_dishwasher_init(g_screens[MODE_DISHWASHER]);

    // Load initial screen
    ui_manager_transition_to_mode(MODE_LAUNCHER);
}

void ui_manager_update(float delta_sec) {
    // Update timer countdown logic
    for (int i = 0; i < TIMER_SLOT_COUNT; i++) {
        if (g_timers[i].active && g_timers[i].remaining > 0) {
            g_timer_second_accum[i] += delta_sec;
            if (g_timer_second_accum[i] < 1.0f) {
                continue;
            }

            uint32_t tick_seconds = (uint32_t)g_timer_second_accum[i];
            g_timer_second_accum[i] -= (float)tick_seconds;

            if (tick_seconds >= g_timers[i].remaining) {
                g_timers[i].remaining = 0;
                g_timers[i].active = false;
                g_timers[i].expired = true;
                g_led_state = 2;  // Set alarm state
            } else {
                g_timers[i].remaining -= tick_seconds;
            }
        }
    }

    // Update current time (increment by delta if no NTP sync)
    g_current_time_sec = (uint32_t)(millis() / 1000);

    // Determine LED state based on timers
    if (g_led_state != 2) {  // Not in alarm state
        int running_idx = -1;
        for (int i = 0; i < TIMER_SLOT_COUNT; i++) {
            if (g_timers[i].active) {
                running_idx = i;
                break;
            }
        }

        if (running_idx >= 0) {
            g_led_state = 1;
            const KitchenTimer& t = g_timers[running_idx];
            float progress = 0.0f;
            if (t.duration > 0) {
                uint32_t elapsed = (t.duration > t.remaining) ? (t.duration - t.remaining) : 0;
                progress = (float)elapsed / (float)t.duration;
            }
            color_for_progress(progress, &g_led_color_r, &g_led_color_g, &g_led_color_b);
        } else {
            g_led_state = 0;
            g_led_color_r = 255;
            g_led_color_g = 140;
            g_led_color_b = 60;
        }
    }

    // Update current screen
    switch (g_current_mode) {
        case MODE_LAUNCHER:     screen_launcher_update(); break;
        case MODE_TIMERS:       screen_timers_update(); break;
        case MODE_CONVERTER:    screen_converter_update(); break;
        case MODE_SCALER:       screen_scaler_update(); break;
        case MODE_BREWING:      screen_brewing_update(); break;
        case MODE_HYDRATION:    screen_hydration_update(); break;
        case MODE_CHILLING:     screen_chilling_update(); break;
        case MODE_MEAT:         screen_meat_update(); break;
        case MODE_DISHWASHER:   screen_dishwasher_update(); break;
        default: break;
    }
}

void ui_manager_handle_rotation(int8_t direction) {
    switch (g_current_mode) {
        case MODE_LAUNCHER:     screen_launcher_handle_rotation(direction); break;
        case MODE_TIMERS:       screen_timers_handle_rotation(direction); break;
        case MODE_CONVERTER:    screen_converter_handle_rotation(direction); break;
        case MODE_SCALER:       screen_scaler_handle_rotation(direction); break;
        case MODE_BREWING:      screen_brewing_handle_rotation(direction); break;
        case MODE_HYDRATION:    screen_hydration_handle_rotation(direction); break;
        case MODE_CHILLING:     screen_chilling_handle_rotation(direction); break;
        case MODE_MEAT:         screen_meat_handle_rotation(direction); break;
        case MODE_DISHWASHER:   screen_dishwasher_handle_rotation(direction); break;
        default: break;
    }
}

void ui_manager_handle_click(void) {
    switch (g_current_mode) {
        case MODE_LAUNCHER:     screen_launcher_handle_click(); break;
        case MODE_TIMERS:       screen_timers_handle_click(); break;
        case MODE_CONVERTER:    screen_converter_handle_click(); break;
        case MODE_SCALER:       screen_scaler_handle_click(); break;
        case MODE_BREWING:      screen_brewing_handle_click(); break;
        case MODE_HYDRATION:    break;  // No click handling needed
        case MODE_CHILLING:     screen_chilling_handle_click(); break;
        case MODE_MEAT:         break;  // No click handling needed
        case MODE_DISHWASHER:   screen_dishwasher_handle_click(); break;
        default: break;
    }
}

void ui_manager_handle_double_click(void) {
    // Double-click returns to launcher from any screen
    ui_manager_transition_to_mode(MODE_LAUNCHER);
}

void ui_manager_handle_long_press(void) {
    // Long-press clears the active timer alarm
    if (g_led_state == 2) {  // In alarm state
        g_led_state = 0;
        for (int i = 0; i < TIMER_SLOT_COUNT; i++) {
            if (g_timers[i].expired) {
                g_timers[i].expired = false;
                g_timers[i].remaining = 0;
                g_timers[i].active = false;
                g_timer_second_accum[i] = 0.0f;
            }
        }
    }
}

void ui_manager_set_wifi_status(bool connected, const char* ip) {
    g_wifi_connected = connected;
    if (ip) {
        strncpy(g_wifi_ip_str, ip, sizeof(g_wifi_ip_str) - 1);
        g_wifi_ip_str[sizeof(g_wifi_ip_str) - 1] = '\0';
    } else {
        strncpy(g_wifi_ip_str, "No WiFi", sizeof(g_wifi_ip_str) - 1);
    }
}

void ui_manager_set_dishwasher_data(const char* status, const char* program, int remaining, int total) {
    // This will be called by ScreenDishwasher to update its display
    // Screens will have their own state management
}

int ui_manager_get_led_state(uint8_t *r, uint8_t *g, uint8_t *b) {
    if (r) *r = g_led_color_r;
    if (g) *g = g_led_color_g;
    if (b) *b = g_led_color_b;
    return g_led_state;
}

bool ui_manager_is_alarm_active(void) {
    return g_led_state == 2;
}
