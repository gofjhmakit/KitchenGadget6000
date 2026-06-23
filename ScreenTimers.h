#ifndef SCREEN_TIMERS_H
#define SCREEN_TIMERS_H

#include "UICommon.h"

void screen_timers_init(lv_obj_t* parent);
void screen_timers_update(void);
void screen_timers_handle_rotation(int8_t direction);
void screen_timers_handle_click(void);

#endif // SCREEN_TIMERS_H
