#ifndef SCREEN_BREWING_H
#define SCREEN_BREWING_H

#include "UICommon.h"

void screen_brewing_init(lv_obj_t* parent);
void screen_brewing_update(void);
void screen_brewing_handle_rotation(int8_t direction);
void screen_brewing_handle_click(void);

#endif // SCREEN_BREWING_H
