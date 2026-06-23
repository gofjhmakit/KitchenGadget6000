#ifndef SCREEN_CHILLING_H
#define SCREEN_CHILLING_H

#include "UICommon.h"

void screen_chilling_init(lv_obj_t* parent);
void screen_chilling_update(void);
void screen_chilling_handle_rotation(int8_t direction);
void screen_chilling_handle_click(void);

#endif // SCREEN_CHILLING_H
