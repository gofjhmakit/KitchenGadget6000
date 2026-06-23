/**
 * @file lv_conf.h
 * Configuration for LVGL 8.3.11 on ESP32-S3 KitchenGadget
 * 
 * Device Specifications:
 * - Display: 240x240 GC9A01 circular TFT
 * - Touch: CST816D capacitive touchscreen
 * - MCU: ESP32-S3 (8MB PSRAM, dual-core)
 * - LED: Neopixel strip (5 pixels)
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Display Settings (240x240 circular display)
 * ========================================================================== */

/** Horizontal resolution of the display. */
#define LV_HOR_RES_MAX          240

/** Vertical resolution of the display. */
#define LV_VER_RES_MAX          240

/** Bit depth of the display.
 * - 1:  1 bit per pixel
 * - 8:  8 bits per pixel
 * - 16: 16 bits per pixel
 * - 32: 32 bits per pixel
 */
#define LV_COLOR_DEPTH          16

/** Swap the 2 bytes of RGB565 color.
 * Helpful if the display has a different byte order than the LittleEndian.
 * 0: disable, 1: enable
 */
#define LV_COLOR_16_SWAP        0

/** Enable anti-aliasing of lines, arcs and curves */
#define LV_ANTIALIAS            1

/* ============================================================================
 * Memory Settings for ESP32-S3 (8MB PSRAM available)
 * ========================================================================== */

/** Memory allocator. 0: malloc, 1: custom allocator */
#define LV_MEM_CUSTOM           0

/** Size of the memory used by `lv_mem_alloc` in bytes. (>= 2048)
 * On ESP32-S3, use PSRAM for better memory management
 * 
 * IMPORTANT: This is allocated in internal DRAM, not PSRAM.
 * Reduce significantly to fit in ESP32-S3's ~120KB available DRAM.
 */
#define LV_MEM_SIZE             (32 * 1024)  // 32KB only (reduced from 320KB)

/** Maximal data size to allocate for an operation in one call in bytes.
 * LVGL allocates this internally, so it should be reasonably large to 'fair' operations.
 */
#define LV_MEM_ALLOC_MAX_SIZE   (16 * 1024)  // 16KB max single alloc (reduced from 128KB)

/** Number of the memory extents. */
#define LV_MEM_EXT_CNT          0

/** Give information about the used memory. For debugging purpose.
 * Adds two more words for each allocated data.
 */
#define LV_MEM_MONITOR          0

/* ============================================================================
 * Drawing Settings
 * ========================================================================== */

/** Enable the Swap Buffer mode.
 * In Swap Buffer mode, the display will be flushed with 2 buffers working in a ping-pong manner.
 * Increases memory usage but allows rendering to a buffer while flushing the other.
 * 
 * For ESP32-S3 with limited internal DRAM, use partial buffers.
 * Tip: Set to ~1/10 of full screen to fit in DRAM (partial rendering with DMA)
 */
#define LV_VDB_SIZE             (240 * 30)  // Partial buffer (30px height) - fits in DRAM

/** Use the same buffer for double buffering */
#define LV_DISP_DOUBLE_BUFFERED 0  // Disable double buffering to save DRAM

/** Enable shadow drawing */
#define LV_SHADOW_CUSTOM        0

/* ============================================================================
 * Operating System & Threading
 * ========================================================================== */

/** 1: Enable the operating system and make the built-in `lv_timer_handler` work */
#define LV_USE_OS               0

/** Porting on embedded systems:
 *  - `LV_OS_NONE` the `lv_timer_handler` should be called periodically from an ISR
 *  - `LV_OS_CUSTOM` call `lv_timer_handler()` from a custom OS, it has some failure tolerence
 *  - `LV_OS_FREERTOS` use it if you use FreeRTOS
 *  - `LV_OS_PTHREADS` use it if you use pthreads
 *  - `LV_OS_RTTHREAD` use it if you use RT-Thread
 *  - `LV_OS_ZEPHYR` use it if you use Zephyr
 */
#define LV_OS_NONE              1

/** Enable a custom tick source.
 * It removes the need to manually call `lv_tick_inc()`.
 */
#define LV_TICK_CUSTOM          0

/** Porting on embedded systems:
 *  - `LV_TICK_CUSTOM` should be 1
 *  - call `lv_tick_inc()` in a timer or ISR periodically
 *  - from `lv_timer_handler()` use a hardware timer
 */
#define LV_TICK_CUSTOM_INCLUDE  ""

/* ============================================================================
 * Feature Toggles
 * ========================================================================== */

/** Enable animations */
#define LV_USE_ANIMATION        1

/** Enable shadow drawing */
#define LV_USE_SHADOW           1

/** Enable outline drawing */
#define LV_USE_OUTLINE          1

/** Enable blur (computationally expensive) */
#define LV_USE_BLUR             0

/** Enable gradient support */
#define LV_USE_GRADIENT         1

/** Enable pattern support */
#define LV_USE_PATTERN          0

/** Enable image support */
#define LV_USE_IMG              1

/** Enable canvas support */
#define LV_USE_CANVAS           0

/** Enable layer support */
#define LV_USE_LAYER            0

/* ============================================================================
 * Input Devices (Touch & Encoder)
 * ========================================================================== */

/** Enable touchpad support */
#define LV_USE_INDEV_DEFAULT_CONFIG 1

/** Enable keyboard input */
#define LV_INDEV_DEF_READ_PERIOD    20

/** Enable encoder (rotary) input */
#define LV_INDEV_DEF_READ_PERIOD    20

/* ============================================================================
 * Widget Settings
 * ========================================================================== */

/** Enable label widget */
#define LV_USE_LABEL            1

/** Enable button widget */
#define LV_USE_BTN              1

/** Enable image widget */
#define LV_USE_IMG              1

/** Enable label image widget (clickable) */
#define LV_USE_IMGBTN           1

/** Enable indicator (progress bar, arc, etc) */
#define LV_USE_ARC              1

/** Enable progress bar */
#define LV_USE_BAR              1

/** Enable slider */
#define LV_USE_SLIDER           1

/** Enable spinner */
#define LV_USE_SPINNER          0

/** Enable roller */
#define LV_USE_ROLLER           0

/** Enable dropdown */
#define LV_USE_DROPDOWN         0

/** Enable checkbox */
#define LV_USE_CHECKBOX         0

/** Enable toggle switch */
#define LV_USE_SWITCH           0

/** Enable textarea */
#define LV_USE_TEXTAREA         0

/** Enable keyboard */
#define LV_USE_KEYBOARD         0

/** Enable spinbox */
#define LV_USE_SPINBOX          0

/** Enable spinbox/number picker in popup */
#define LV_USE_SPINBOX_POPUP    0

/** Enable scale */
#define LV_USE_SCALE            0

/** Enable calendar */
#define LV_USE_CALENDAR         0

/** Enable table */
#define LV_USE_TABLE            0

/** Enable tabview */
#define LV_USE_TABVIEW          0

/** Enable page */
#define LV_USE_PAGE             0

/** Enable msgbox */
#define LV_USE_MSGBOX           0

/** Enable win */
#define LV_USE_WIN              0

/* ============================================================================
 * Font Settings
 * ========================================================================== */

/** Enable font support */
#define LV_USE_FONT_COMPRESSED 0

/** Enable font subsetting to reduce code size */
#define LV_USE_FONT_SUBPX       0

/** Subpixel rendering (anti-aliasing):
 * 0: no subpixel, 1: horizontal, 2: vertical, 3: both
 */
#define LV_SUBPX_BGR            0

/** Cache the glyphs of fonts. Requires extra memory.
 * - 0: No font cache
 * - 1: Cache all glyphs
 */
#define LV_FNT_CACHE_SEARCH_SIZE 16

/** Default fonts to use */
#define LV_FONT_DEFAULT         &lv_font_montserrat_14

/** Montserrat font variants for the UI */
#define LV_FONT_MONTSERRAT_10   1
#define LV_FONT_MONTSERRAT_12   1
#define LV_FONT_MONTSERRAT_14   1
#define LV_FONT_MONTSERRAT_16   1
#define LV_FONT_MONTSERRAT_18   1
#define LV_FONT_MONTSERRAT_20   1
#define LV_FONT_MONTSERRAT_24   1
#define LV_FONT_MONTSERRAT_28   1
#define LV_FONT_MONTSERRAT_32   1
#define LV_FONT_MONTSERRAT_36   1

/* ============================================================================
 * Text & Layout Settings
 * ========================================================================== */

/** Enable bidirectional text support */
#define LV_USE_BIDI             0

/** Enable Arabic text support */
#define LV_USE_ARABIC_HEBREW_PERSIAN 0

/** Enable Thai text support */
#define LV_USE_THAI             0

/** Enable rich text support (markup) */
#define LV_LABEL_LONG_TXT_HINT  1

/** Size of the text shadow buffer */
#define LV_TEXT_SHADOW_CACHE_SIZE 0

/* ============================================================================
 * Color & Theme Settings
 * ========================================================================== */

/** Use a single theme for the entire application */
#define LV_USE_THEME_DEFAULT    0

/** Theme to use */
#define LV_THEME_DEFAULT_DARK   0  // 1 = dark, 0 = light

/** Use color manipulation functions */
#define LV_USE_PALETTE          0

/* ============================================================================
 * Log Settings (for debugging)
 * ========================================================================== */

/** Enable logging */
#define LV_USE_LOG              0

/** Set the log level:
 * - LV_LOG_LEVEL_TRACE
 * - LV_LOG_LEVEL_DEBUG
 * - LV_LOG_LEVEL_INFO
 * - LV_LOG_LEVEL_WARN
 * - LV_LOG_LEVEL_ERROR
 * - LV_LOG_LEVEL_USER
 * - LV_LOG_LEVEL_NONE
 */
#define LV_LOG_LEVEL            LV_LOG_LEVEL_WARN

/** Print file and line information on log */
#define LV_LOG_PRINT_FILE_LINE  0

/* ============================================================================
 * Assert Settings
 * ========================================================================== */

/** Enable custom assert handler */
#define LV_USE_ASSERT_NULL      0
#define LV_USE_ASSERT_MALLOC    0
#define LV_USE_ASSERT_STYLE     0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ       0

/* ============================================================================
 * Performance & Optimization
 * ========================================================================== */

/** Enable performance monitoring */
#define LV_USE_PERF_MONITOR     0

/** Enable memory profiling */
#define LV_USE_MEM_MONITOR      0

/** Split long touchpad events into multiple smaller events */
#define LV_SPLIT_LONG_PRESS     0

/** Inactivity period to reset long press counter in milliseconds */
#define LV_INACTIVITY_PERIOD    0

/* ============================================================================
 * Gesture Support
 * ========================================================================== */

/** Enable gesture recognition */
#define LV_USE_GESTURES         0

/** Enable rotate gesture */
#define LV_GESTURE_DRAG         0

/** Enable rotate gesture */
#define LV_GESTURE_ROTATE       0

/** Enable zoom gesture */
#define LV_GESTURE_ZOOM         0

/* ============================================================================
 * Advanced Settings (usually not needed)
 * ========================================================================== */

/** Enable custom fill operations */
#define LV_FILL_CUSTOM          0

/** Use malloc and free from C library */
#define LV_USE_STDLIB_MALLOC    0
#define LV_USE_STDLIB_MEMCPY    1
#define LV_USE_STDLIB_MEMSET    1

/* ============================================================================
 * Widget Customization
 * ========================================================================== */

/** Maximum number of objects (screens, objects, etc) */
#define LV_OBJ_ID_FREE_NUM      10000

/** Maximum number of free entries in groups */
#define LV_GROUP_DEF_SIZE       16

/** Enable draw debug mode */
#define LV_DRAW_DEBUG           0

/* ============================================================================
 * Compile-time Checks
 * ========================================================================== */

#if LV_COLOR_DEPTH != 1 && LV_COLOR_DEPTH != 8 && LV_COLOR_DEPTH != 16 && LV_COLOR_DEPTH != 32
#error "LV_COLOR_DEPTH must be 1, 8, 16, or 32"
#endif

#if LV_HOR_RES_MAX < 1 || LV_VER_RES_MAX < 1
#error "LV_HOR_RES_MAX and LV_VER_RES_MAX must be greater than 0"
#endif

#endif /* LV_CONF_H */
