// --- Data Structures ---
typedef struct {
    char label[32];          // e.g., "Resolution"
    char options[16][32];     // e.g., "128x128", "256x256"
    int option_count;        // How many options exist
    int current_selection;   // Which option is currently active (0-indexed)
    int y_pos;               // Screen Y position
} Dropdown;

void dropdowns_navigate();
void dropdowns_init();
void dropdowns_draw();
void dropdowns_draw_menu();