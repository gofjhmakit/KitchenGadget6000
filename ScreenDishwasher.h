#ifndef SCREEN_DISHWASHER_H
#define SCREEN_DISHWASHER_H

#include "UICommon.h"

void screen_dishwasher_init(lv_obj_t* parent);
void screen_dishwasher_update(void);
void screen_dishwasher_handle_rotation(int8_t direction);
void screen_dishwasher_handle_click(void);

#endif // SCREEN_DISHWASHER_H
