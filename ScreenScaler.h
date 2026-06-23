#ifndef SCREEN_SCALER_H
#define SCREEN_SCALER_H

#include "UICommon.h"

void screen_scaler_init(lv_obj_t* parent);
void screen_scaler_update(void);
void screen_scaler_handle_rotation(int8_t direction);
void screen_scaler_handle_click(void);

#endif // SCREEN_SCALER_H
