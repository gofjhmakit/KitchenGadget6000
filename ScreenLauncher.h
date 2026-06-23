#ifndef SCREEN_LAUNCHER_H
#define SCREEN_LAUNCHER_H

#include "UICommon.h"

void screen_launcher_init(lv_obj_t* parent);
void screen_launcher_update(void);
void screen_launcher_handle_rotation(int8_t direction);
void screen_launcher_handle_click(void);

#endif // SCREEN_LAUNCHER_H
