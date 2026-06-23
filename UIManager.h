#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "UICommon.h"

// ============================================================================
// Screen Initialization Functions (implemented in each Screen_*.cpp)
// ============================================================================

void screen_launcher_init(lv_obj_t* parent);
void screen_launcher_update(void);
void screen_launcher_handle_rotation(int8_t direction);
void screen_launcher_handle_click(void);

void screen_timers_init(lv_obj_t* parent);
void screen_timers_update(void);
void screen_timers_handle_rotation(int8_t direction);
void screen_timers_handle_click(void);

void screen_converter_init(lv_obj_t* parent);
void screen_converter_update(void);
void screen_converter_handle_rotation(int8_t direction);
void screen_converter_handle_click(void);

void screen_scaler_init(lv_obj_t* parent);
void screen_scaler_update(void);
void screen_scaler_handle_rotation(int8_t direction);
void screen_scaler_handle_click(void);

void screen_brewing_init(lv_obj_t* parent);
void screen_brewing_update(void);
void screen_brewing_handle_rotation(int8_t direction);
void screen_brewing_handle_click(void);

void screen_hydration_init(lv_obj_t* parent);
void screen_hydration_update(void);
void screen_hydration_handle_rotation(int8_t direction);
void screen_hydration_handle_click(void);

void screen_chilling_init(lv_obj_t* parent);
void screen_chilling_update(void);
void screen_chilling_handle_rotation(int8_t direction);
void screen_chilling_handle_click(void);

void screen_meat_init(lv_obj_t* parent);
void screen_meat_update(void);
void screen_meat_handle_rotation(int8_t direction);
void screen_meat_handle_click(void);

void screen_dishwasher_init(lv_obj_t* parent);
void screen_dishwasher_update(void);
void screen_dishwasher_handle_rotation(int8_t direction);
void screen_dishwasher_handle_click(void);

// ============================================================================
// UI Manager API (called from main sketch)
// ============================================================================

void ui_manager_init(void);
void ui_manager_update(float delta_sec);
void ui_manager_handle_rotation(int8_t direction);
void ui_manager_handle_click(void);
void ui_manager_handle_double_click(void);
void ui_manager_handle_long_press(void);

// External settings & IoT integration
void ui_manager_set_wifi_status(bool connected, const char* ip);
void ui_manager_set_dishwasher_data(const char* status, const char* program, int remaining, int total);

// LED state query
int ui_manager_get_led_state(uint8_t *r, uint8_t *g, uint8_t *b);
bool ui_manager_is_alarm_active(void);

// Internal routing
void ui_manager_transition_to_mode(KitchenMode new_mode);

#endif // UI_MANAGER_H
