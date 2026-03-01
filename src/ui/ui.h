#ifndef UI
#define UI

#include "dropdown.h"

void *open_ui(void *args);
Form *ui_init();
void ui_frame(Form *form);
void ui_loop(Form *form);

#endif