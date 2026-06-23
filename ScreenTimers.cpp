#include "UICommon.h"
#include <Arduino.h>
#include <cstdio>

static lv_obj_t *ui_timer_screen = NULL;
static lv_obj_t *ui_arc_timer = NULL;
static lv_obj_t *ui_label_time = NULL;
static lv_obj_t *ui_label_status = NULL;
static lv_obj_t *ui_label_hint = NULL;
static lv_obj_t *ui_label_slot = NULL;
static lv_obj_t *ui_label_emoji = NULL;

static bool g_selecting_emoji = false;
static int g_emoji_idx = 0;

static const uint32_t TIMER_ARC_CAP_SECONDS = 3600;
static const uint32_t TIMER_STEP_SECONDS = 30;
static const uint32_t TIMER_MAX_SECONDS = 12 * 3600;

struct EmojiOption {
  const char* label;
  const char* emoji;
};

static const EmojiOption kEmojiOptions[] = {
  {"Apple", "🍎"}, {"Green Apple", "🍏"}, {"Pear", "🍐"}, {"Peach", "🍑"},
  {"Cherries", "🍒"}, {"Strawberry", "🍓"}, {"Blueberries", "🫐"}, {"Grapes", "🍇"},
  {"Watermelon", "🍉"}, {"Melon", "🍈"}, {"Pineapple", "🍍"}, {"Mango", "🥭"},
  {"Kiwi", "🥝"}, {"Lemon", "🍋"}, {"Banana", "🍌"}, {"Coconut", "🥥"},
  {"Avocado", "🥑"}, {"Tomato", "🍅"}, {"Eggplant", "🍆"}, {"Bell Pepper", "🫑"},
  {"Hot Pepper", "🌶️"}, {"Cucumber", "🥒"}, {"Leafy Greens", "🥬"}, {"Broccoli", "🥦"},
  {"Garlic", "🧄"}, {"Onion", "🧅"}, {"Corn", "🌽"}, {"Carrot", "🥕"},
  {"Mushroom", "🍄"}, {"Peanuts", "🥜"}, {"Beans", "🫘"}, {"Bread", "🍞"},
  {"Croissant", "🥐"}, {"Bagel", "🥯"}, {"Pretzel", "🥨"}, {"Cheese", "🧀"},
  {"Egg", "🥚"}, {"Butter", "🧈"}, {"Milk", "🥛"}, {"Bacon", "🥓"},
  {"Poultry", "🍗"}, {"Meat", "🍖"}, {"Cow (Beef)", "🐄"}, {"Pig (Pork)", "🐷"},
  {"Chicken", "🐔"}, {"Fish", "🐟"}, {"Shrimp", "🍤"},
  {"Salt", "🧂"}, {"Cooking", "🍳"}, {"Stew", "🍲"}, {"Salad", "🥗"},
  {"Sandwich", "🥪"}, {"Taco", "🌮"}, {"Burrito", "🌯"}, {"Pizza", "🍕"},
  {"Hamburger", "🍔"}, {"Hot Dog", "🌭"}, {"Spaghetti", "🍝"}, {"Rice", "🍚"},
  {"Sushi", "🍣"}, {"Ramen", "🍜"}, {"Soup", "🍲"}, {"Cake", "🍰"},
  {"Cupcake", "🧁"}, {"Cookie", "🍪"}, {"Chocolate", "🍫"}, {"Honey", "🍯"},
  {"Coffee", "☕"}, {"Tea", "🍵"}, {"Beer", "🍺"}, {"Wine", "🍷"},
  {"Champagne", "🍾"}
};

static const int kEmojiOptionCount = (int)(sizeof(kEmojiOptions) / sizeof(kEmojiOptions[0]));

static int timer_arc_value_from_seconds(uint32_t sec) {
  if (sec >= TIMER_ARC_CAP_SECONDS) return 100;
  return (int)((sec * 100U) / TIMER_ARC_CAP_SECONDS);
}

static void format_timer_text(uint32_t total_sec, char *buf, size_t size) {
  uint32_t h = total_sec / 3600U;
  uint32_t m = (total_sec % 3600U) / 60U;
  uint32_t s = total_sec % 60U;
  if (h > 0U) {
    snprintf(buf, size, "%u:%02u:%02u", (unsigned)h, (unsigned)m, (unsigned)s);
  } else {
    snprintf(buf, size, "%02u:%02u", (unsigned)m, (unsigned)s);
  }
}

void screen_timers_init(lv_obj_t* parent) {
  ui_timer_screen = parent;
  g_active_timer_slot = 0;
  g_selecting_emoji = false;
  g_emoji_idx = 0;

  lv_obj_clear_flag(ui_timer_screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(ui_timer_screen, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(ui_timer_screen, LV_OPA_COVER, 0);

  ui_arc_timer = lv_arc_create(ui_timer_screen);
  lv_obj_set_size(ui_arc_timer, 204, 204);
  lv_obj_align(ui_arc_timer, LV_ALIGN_CENTER, 0, 0);
  lv_arc_set_range(ui_arc_timer, 0, 100);
  lv_arc_set_bg_angles(ui_arc_timer, 0, 359);
  lv_arc_set_rotation(ui_arc_timer, 270);
  lv_arc_set_value(ui_arc_timer, 0);
  lv_obj_set_style_arc_width(ui_arc_timer, 14, LV_PART_MAIN);
  lv_obj_set_style_arc_color(ui_arc_timer, lv_color_hex(0x303030), LV_PART_MAIN);
  lv_obj_set_style_arc_width(ui_arc_timer, 14, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(ui_arc_timer, lv_color_hex(0x3CD96A), LV_PART_INDICATOR);
  lv_obj_set_style_arc_rounded(ui_arc_timer, true, LV_PART_INDICATOR);
  lv_obj_remove_style(ui_arc_timer, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(ui_arc_timer, LV_OBJ_FLAG_CLICKABLE);

  ui_label_time = lv_label_create(ui_timer_screen);
  lv_obj_align(ui_label_time, LV_ALIGN_CENTER, 0, -18);
  lv_obj_set_style_text_font(ui_label_time, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(ui_label_time, lv_color_hex(0xA8FFB9), 0);
  lv_label_set_text(ui_label_time, "00:00");

  ui_label_emoji = lv_label_create(ui_timer_screen);
  lv_obj_align(ui_label_emoji, LV_ALIGN_CENTER, 0, 24);
  lv_obj_set_style_text_font(ui_label_emoji, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(ui_label_emoji, lv_color_hex(0xFFD166), 0);
  lv_label_set_text(ui_label_emoji, "🍳");

  ui_label_status = lv_label_create(ui_timer_screen);
  lv_obj_align(ui_label_status, LV_ALIGN_CENTER, 0, 66);
  lv_obj_set_style_text_font(ui_label_status, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(ui_label_status, lv_color_hex(0xF8C64E), 0);
  lv_label_set_text(ui_label_status, "SET");

  ui_label_hint = lv_label_create(ui_timer_screen);
  lv_obj_align(ui_label_hint, LV_ALIGN_BOTTOM_MID, 0, -8);
  lv_obj_set_style_text_font(ui_label_hint, &lv_font_montserrat_10, 0);
  lv_obj_set_style_text_color(ui_label_hint, lv_color_hex(0x9E9E9E), 0);
  lv_label_set_text(ui_label_hint, "Rotate:set | Click:emoji | Dbl:menu");

  ui_label_slot = lv_label_create(ui_timer_screen);
  lv_obj_align(ui_label_slot, LV_ALIGN_TOP_MID, 0, 8);
  lv_obj_set_style_text_font(ui_label_slot, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(ui_label_slot, lv_color_hex(0x66CCFF), 0);
  lv_label_set_text(ui_label_slot, "TIMER 1/5");
}

void screen_timers_update(void) {
  KitchenTimer &timer = g_timers[g_active_timer_slot];
  char buf[24];
  char slot_buf[32];

  snprintf(slot_buf, sizeof(slot_buf), "TIMER %d/%d", g_active_timer_slot + 1, TIMER_SLOT_COUNT);
  lv_label_set_text(ui_label_slot, slot_buf);

  if (g_selecting_emoji) {
    lv_arc_set_value(ui_arc_timer, timer_arc_value_from_seconds(timer.duration));
    lv_label_set_text(ui_label_time, "Set timer emoji");
    lv_label_set_text(ui_label_emoji, kEmojiOptions[g_emoji_idx].emoji);
    lv_label_set_text(ui_label_status, kEmojiOptions[g_emoji_idx].label);
    lv_label_set_text(ui_label_hint, "Rotate:emoji | Click:start | Dbl:menu");
    return;
  }

  if (timer.expired) {
    bool blink = ((millis() / 180U) % 2U) == 0U;
    lv_arc_set_value(ui_arc_timer, 0);
    lv_obj_set_style_arc_color(
      ui_arc_timer,
      blink ? lv_color_hex(0xFF3355) : lv_color_hex(0xFF9F1A),
      LV_PART_INDICATOR
    );
    lv_obj_set_style_arc_width(ui_arc_timer, blink ? 18 : 14, LV_PART_INDICATOR);
    lv_label_set_text(ui_label_time, blink ? "00:00" : "ALARM");
    lv_label_set_text(ui_label_status, blink ? "TIME'S UP" : "HOLD TO CLEAR");
    lv_label_set_text(ui_label_hint, "Click:reset | Rotate:switch | Dbl:menu");
    lv_obj_set_style_text_color(ui_label_status, lv_color_hex(0xFF5A5A), 0);
    lv_label_set_text(ui_label_emoji, timer.emoji[0] ? timer.emoji : "⏰");
    return;
  }

  uint32_t shown_seconds = timer.active ? timer.remaining : timer.duration;
  lv_arc_set_value(ui_arc_timer, timer_arc_value_from_seconds(shown_seconds));
  lv_obj_set_style_arc_width(ui_arc_timer, 14, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(ui_arc_timer, lv_color_hex(0x3CD96A), LV_PART_INDICATOR);

  format_timer_text(shown_seconds, buf, sizeof(buf));
  lv_label_set_text(ui_label_time, buf);
  lv_label_set_text(ui_label_emoji, timer.emoji[0] ? timer.emoji : "🍳");

  if (timer.active) {
    lv_label_set_text(ui_label_status, "RUNNING");
    lv_label_set_text(ui_label_hint, "Click:pause | Rotate:switch | Dbl:menu");
    lv_obj_set_style_text_color(ui_label_status, lv_color_hex(0x7EE787), 0);
  } else if (timer.remaining > 0 && timer.remaining < timer.duration) {
    lv_label_set_text(ui_label_status, "PAUSED");
    lv_label_set_text(ui_label_hint, "Click:resume | Dbl:menu");
    lv_obj_set_style_text_color(ui_label_status, lv_color_hex(0xF8C64E), 0);
  } else if (timer.duration > 0) {
    lv_label_set_text(ui_label_status, "READY");
    lv_label_set_text(ui_label_hint, "Click:emoji | Dbl:menu");
    lv_obj_set_style_text_color(ui_label_status, lv_color_hex(0xF8C64E), 0);
  } else {
    lv_label_set_text(ui_label_status, "SET");
    lv_label_set_text(ui_label_hint, "Rotate:set | Click:emoji | Dbl:menu");
    lv_obj_set_style_text_color(ui_label_status, lv_color_hex(0xF8C64E), 0);
  }
}

void screen_timers_handle_rotation(int8_t direction) {
  KitchenTimer &timer = g_timers[g_active_timer_slot];

  if (g_selecting_emoji) {
    g_emoji_idx += direction;
    while (g_emoji_idx < 0) g_emoji_idx += kEmojiOptionCount;
    while (g_emoji_idx >= kEmojiOptionCount) g_emoji_idx -= kEmojiOptionCount;
    return;
  }

  if (timer.active || timer.expired) {
    g_active_timer_slot += direction;
    if (g_active_timer_slot < 0) g_active_timer_slot = TIMER_SLOT_COUNT - 1;
    if (g_active_timer_slot >= TIMER_SLOT_COUNT) g_active_timer_slot = 0;
    return;
  }

  int32_t next = (int32_t)timer.duration + ((int32_t)direction * (int32_t)TIMER_STEP_SECONDS);
  if (next < 0) next = 0;
  if (next > (int32_t)TIMER_MAX_SECONDS) next = (int32_t)TIMER_MAX_SECONDS;
  timer.duration = (uint32_t)next;
  timer.remaining = timer.duration;
}

void screen_timers_handle_click(void) {
  KitchenTimer &timer = g_timers[g_active_timer_slot];

  if (g_selecting_emoji) {
    snprintf(timer.emoji, sizeof(timer.emoji), "%s", kEmojiOptions[g_emoji_idx].emoji);
    timer.name = kEmojiOptions[g_emoji_idx].label;
    if (timer.duration == 0) {
      timer.duration = 60;
      timer.remaining = 60;
    } else if (timer.remaining == 0) {
      timer.remaining = timer.duration;
    }
    timer.active = true;
    timer.expired = false;
    g_selecting_emoji = false;
    return;
  }

  if (timer.expired) {
    timer.expired = false;
    timer.active = false;
    timer.remaining = timer.duration;
    return;
  }

  if (timer.active) {
    timer.active = false;
    return;
  }

  if (timer.remaining > 0 && timer.remaining < timer.duration) {
    timer.active = true;
    return;
  }

  g_selecting_emoji = true;
}
