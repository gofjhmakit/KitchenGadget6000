#ifndef UI_COMMON_H
#define UI_COMMON_H

#include <lvgl.h>

// ============================================================================
// Enums & Constants
// ============================================================================

enum KitchenMode {
    MODE_LAUNCHER = 0,
    MODE_TIMERS,
    MODE_CONVERTER,
    MODE_SCALER,
    MODE_BREWING,
    MODE_HYDRATION,
    MODE_CHILLING,
    MODE_MEAT,
    MODE_DISHWASHER,
    MODE_COUNT
};

enum ConverterCategory {
    CAT_VOLUME = 0,
    CAT_WEIGHT,
    CAT_TEMP,
    CAT_COUNT
};

static const int TIMER_SLOT_COUNT = 5;

// ============================================================================
// Shared Data Structures
// ============================================================================

struct KitchenTimer {
    uint32_t duration;    // Total duration in seconds
    uint32_t remaining;   // Remaining seconds
    bool active;          // Is the timer ticking?
    bool expired;         // Has the timer completed?
    lv_color_t color;     // Visual color of the progress ring
    const char* name;     // Label (e.g. "Timer 1", "Egg Soft")
    char emoji[8];        // Food/plant emoji (e.g., "🥚", "🥕")
};

// ============================================================================
// Global State Access (defined in UIManager.cpp)
// ============================================================================

// Timer access
extern KitchenTimer g_timers[TIMER_SLOT_COUNT];
extern int g_active_timer_slot;

// WiFi status
extern bool g_wifi_connected;
extern char g_wifi_ip_str[16];

// Current time
extern uint32_t g_current_time_sec;

// LED state
extern int g_led_state;
extern uint8_t g_led_color_r;
extern uint8_t g_led_color_g;
extern uint8_t g_led_color_b;

// ============================================================================
// Helper Functions
// ============================================================================

void format_seconds(uint32_t total_sec, char* buf, size_t size);

#endif // UI_COMMON_H
