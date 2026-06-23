#ifndef KITCHEN_UI_H
#define KITCHEN_UI_H

// ============================================================================
// Backward Compatibility Layer - Redirects to Modular UI System
// ============================================================================
// This is the single header that KitchenGadget.ino includes.
// It pulls in all modular components and provides compatibility aliases.

// Core UI management
#include "UICommon.h"
#include "UIManager.h"

// Screen implementations - included here to ensure compilation
#include "ScreenLauncher.h"
#include "ScreenTimers.h"
#include "ScreenConverter.h"
#include "ScreenScaler.h"
#include "ScreenBrewing.h"
#include "ScreenHydration.h"
#include "ScreenChilling.h"
#include "ScreenMeat.h"
#include "ScreenDishwasher.h"

// ============================================================================
// Backward Compatibility Aliases
// ============================================================================
// Existing code using the old API names continues to work

#define kitchen_ui_init                    ui_manager_init
#define kitchen_ui_update                  ui_manager_update
#define kitchen_ui_handle_rotation         ui_manager_handle_rotation
#define kitchen_ui_handle_click            ui_manager_handle_click
#define kitchen_ui_handle_double_click     ui_manager_handle_double_click
#define kitchen_ui_handle_long_press       ui_manager_handle_long_press
#define kitchen_ui_set_wifi_status         ui_manager_set_wifi_status
#define kitchen_ui_set_dishwasher_data     ui_manager_set_dishwasher_data
#define kitchen_ui_get_led_state           ui_manager_get_led_state
#define kitchen_ui_is_alarm_active         ui_manager_is_alarm_active

#endif // KITCHEN_UI_H
