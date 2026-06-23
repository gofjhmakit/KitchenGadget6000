#ifndef SCREEN_HYDRATION_H
#define SCREEN_HYDRATION_H

#include "UICommon.h"

void screen_hydration_init(lv_obj_t* parent);
void screen_hydration_update(void);
void screen_hydration_handle_rotation(int8_t direction);
void screen_hydration_handle_click(void);

#endif // SCREEN_HYDRATION_H
