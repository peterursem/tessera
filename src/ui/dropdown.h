#ifndef UI_DROPDOWN
#define UI_DROPDOWN

#define FORM_TOTAL_FIELDS 3

// ===== Data Structures =====
typedef struct {
    char label[32];
    char options[16][32];
    int option_count;
    int current_selection;
    int y_pos; // For drawing the option on screen
} Dropdown;

typedef struct {
    Dropdown fields[FORM_TOTAL_FIELDS];
    int focused_field; // Which field is highlighted
    int menu_open;	   // 0 = closed, 1 = open
    int form_active;
    int menu_selection; // Which item inside the open menu is highlighted
} Form;

void form_init(Form *form);
void form_draw(int x, int y, Form *form);
void dropdowns_draw_menu(Form *form);
void dropdowns_navigate(Form *form);

#endif