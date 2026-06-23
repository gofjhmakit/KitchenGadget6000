#ifndef SCREEN_MEAT_H
#define SCREEN_MEAT_H

#include "UICommon.h"

void screen_meat_init(lv_obj_t* parent);
void screen_meat_update(void);
void screen_meat_handle_rotation(int8_t direction);
void screen_meat_handle_click(void);

#endif // SCREEN_MEAT_H
