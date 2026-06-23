#ifndef SCREEN_CONVERTER_H
#define SCREEN_CONVERTER_H

#include "UICommon.h"

void screen_converter_init(lv_obj_t* parent);
void screen_converter_update(void);
void screen_converter_handle_rotation(int8_t direction);
void screen_converter_handle_click(void);

#endif // SCREEN_CONVERTER_H
