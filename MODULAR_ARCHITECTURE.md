# KitchenGadget - Modular Architecture

This project has been refactored into a **fully modular architecture** where each screen/functionality is its own independent `.cpp`/`.h` file pair. This makes the codebase:

- ✅ **Easy to maintain** - Each screen has its own file
- ✅ **Easy to extend** - Add new screens by creating new `Screen*.cpp` files
- ✅ **Arduino IDE compatible** - Works directly in Arduino IDE without platformio or external tools
- ✅ **arduino-cli compatible** - Can be built with `arduino-cli`
- ✅ **Modular** - All files coexist peacefully in the main folder

## Project Structure

### Core System Files
```
KitchenGadget.ino          → Main entry point (unchanged)
UIManager.h/cpp            → Screen manager and routing
UICommon.h                 → Shared types, enums, and global state
KitchenUI.h/cpp            → Backward compatibility layer
```

### Screen Files (9 Screens Total)
```
ScreenLauncher.h/cpp       → Home menu with screen selection
ScreenTimers.h/cpp         → Multi-timer with 3 concentric rings
ScreenConverter.h/cpp      → Unit converter (volume, weight, temperature)
ScreenScaler.h/cpp         → Portion/ingredient scaler
ScreenBrewing.h/cpp        → Tea & egg brewing presets
ScreenHydration.h/cpp      → Baker's bread hydration calculator
ScreenChilling.h/cpp       → Drink chiller calculator
ScreenMeat.h/cpp           → Meat doneness temperature guide
ScreenDishwasher.h/cpp     → IoT Dishwasher status monitor
```

### Hardware Drivers
```
CST816D.h/cpp              → Capacitive touch driver (unchanged)
```

## How It Works

### UIManager Architecture

The `UIManager` acts as a **central orchestrator**:

1. **Initialization** (`ui_manager_init`)
   - Creates LVGL screen objects for all 9 modes
   - Calls each screen's `init()` function to set up UI elements
   - Manages global state (timers, WiFi, LED colors)

2. **Event Routing**
   - Rotation, clicks, double-clicks, long-presses are routed to the active screen
   - Each screen handles its own input logic independently

3. **Update Loop** (`ui_manager_update`)
   - Updates all timer countdowns
   - Manages LED state based on timer activity
   - Calls the active screen's `update()` function

4. **Screen Transitions**
   - Double-click returns to launcher from any screen
   - Click on launcher menu item opens that screen
   - Some screens (e.g., Brewing) auto-transition to Timers when launching a timer

### Global State Management

All screens access shared state through `UICommon.h`:

```c
// Timer management
extern KitchenTimer g_timers[3];
extern int g_active_timer_slot;

// WiFi status
extern bool g_wifi_connected;
extern char g_wifi_ip_str[16];

// Current time
extern uint32_t g_current_time_sec;

// LED state
extern int g_led_state;           // 0=idle, 1=running, 2=alarm
extern uint8_t g_led_color_r;
extern uint8_t g_led_color_g;
extern uint8_t g_led_color_b;
```

## Building & Uploading

### Arduino IDE

1. Open `KitchenGadget.ino` in Arduino IDE
2. All screen files will be automatically included
3. Select your board and port
4. Click Upload

### Arduino CLI

```bash
# Build
arduino-cli compile --fqbn esp32:esp32:esp32-s3-box KitchenGadget

# Upload
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32-s3-box KitchenGadget

# Monitor
arduino-cli monitor -p /dev/ttyUSB0
```

## Adding a New Screen

To add a new screen:

### 1. Create Header File (`ScreenNewFeature.h`)
```c
#ifndef SCREEN_NEW_FEATURE_H
#define SCREEN_NEW_FEATURE_H

#include "UICommon.h"

void screen_newfeature_init(lv_obj_t* parent);
void screen_newfeature_update(void);
void screen_newfeature_handle_rotation(int8_t direction);
void screen_newfeature_handle_click(void);

#endif
```

### 2. Create Implementation (`ScreenNewFeature.cpp`)
```c
#include "ScreenNewFeature.h"
#include "UIManager.h"

// Screen state variables
static lv_obj_t* label1 = NULL;

void screen_newfeature_init(lv_obj_t* parent) {
    // Create LVGL objects
    label1 = lv_label_create(parent);
    // ... setup
}

void screen_newfeature_update(void) {
    // Called ~50Hz to update display
}

void screen_newfeature_handle_rotation(int8_t direction) {
    // Handle encoder rotation
}

void screen_newfeature_handle_click(void) {
    // Handle button click
}
```

### 3. Add to `UICommon.h`
Add enum value:
```c
enum KitchenMode {
    // ... existing modes
    MODE_NEW_FEATURE,
    MODE_COUNT
};
```

### 4. Add to `UIManager.h`
Declare the screen functions:
```c
void screen_newfeature_init(lv_obj_t* parent);
void screen_newfeature_update(void);
void screen_newfeature_handle_rotation(int8_t direction);
void screen_newfeature_handle_click(void);
```

### 5. Update `UIManager.cpp`
Initialize and wire up the screen:
```c
// In ui_manager_init()
screen_newfeature_init(g_screens[MODE_NEW_FEATURE]);

// In ui_manager_update()
case MODE_NEW_FEATURE: screen_newfeature_update(); break;

// In ui_manager_handle_rotation()
case MODE_NEW_FEATURE: screen_newfeature_handle_rotation(direction); break;

// In ui_manager_handle_click()
case MODE_NEW_FEATURE: screen_newfeature_handle_click(); break;
```

### 6. Update Menu (Optional)
If you want it in the launcher, update menu names/descriptions in `ScreenLauncher.cpp`.

## File Sizes

| File | Size | Purpose |
|------|------|---------|
| UIManager.cpp | 7.8K | Screen routing & state management |
| ScreenTimers.cpp | 6.9K | Multi-timer with progress rings |
| ScreenChilling.cpp | 6.0K | Drink chiller calculator |
| ScreenDishwasher.cpp | 5.7K | IoT dishwasher monitor |
| ScreenConverter.cpp | 7.1K | Unit conversion (volume/weight/temp) |
| ScreenBrewing.cpp | 3.5K | Tea/egg cooking presets |
| ScreenHydration.cpp | 4.1K | Bread hydration ratio |
| ScreenScaler.cpp | 3.6K | Portion scaling |
| ScreenMeat.cpp | 3.7K | Meat doneness temps |
| ScreenLauncher.cpp | 4.9K | Home menu |

**Total: ~55KB of screen code** (before LVGL and hardware drivers)

## Backward Compatibility

The `KitchenUI.h` header file provides backward compatibility:

```c
#define kitchen_ui_init                ui_manager_init
#define kitchen_ui_update              ui_manager_update
#define kitchen_ui_handle_rotation     ui_manager_handle_rotation
// ... etc
```

This means the main sketch (`KitchenGadget.ino`) doesn't need any changes—it continues using the old function names.

## Architecture Diagram

```
KitchenGadget.ino
    ↓
KitchenUI.h (compatibility layer)
    ↓
UIManager.h/cpp (orchestrator)
    ├→ ScreenLauncher.h/cpp
    ├→ ScreenTimers.h/cpp
    ├→ ScreenConverter.h/cpp
    ├→ ScreenScaler.h/cpp
    ├→ ScreenBrewing.h/cpp
    ├→ ScreenHydration.h/cpp
    ├→ ScreenChilling.h/cpp
    ├→ ScreenMeat.h/cpp
    └→ ScreenDishwasher.h/cpp
    ↓
UICommon.h (shared state)
    ↓
LVGL + Hardware Drivers (CST816D, LovyanGFX, etc)
```

## Benefits of This Architecture

1. **Easy to Debug** - Each screen's logic is isolated and testable
2. **Easy to Modify** - Change one screen without affecting others
3. **Easy to Add** - New screens follow a simple pattern
4. **Arduino IDE Native** - No build tools needed
5. **CLI Compatible** - `arduino-cli build/upload/monitor` works perfectly
6. **Scalable** - Can easily handle 20+ screens without bloat

---

**Created:** 2024
**Architecture:** Modular UI Framework
**Compatible with:** Arduino IDE, arduino-cli
