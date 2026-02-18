#include "app.h"

#include <ncurses.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "dropdown.h"
#include "manager.h"

// Dimensions for the UI layout
#define BAR_START_Y 18
#define BG_START_X 61 

void draw_bricks(int start_x, int start_y, int width, int height) {
    // The base pattern (6 characters long)
    // Note: Backslashes must be escaped: "\\" is one backslash.
    const char *pattern = "\\  \\__";
    const int pat_len = 6;

    attron(COLOR_PAIR(3)); // Use your background color pair

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Calculate absolute screen positions
            int abs_y = start_y + y;
            int abs_x = start_x + x;

            // Math to determine which character creates the diagonal shift
            // We shift the pattern by 4 indices every time we go down 1 row.
            int index = (abs_x + (abs_y * 2)) % pat_len;

            mvaddch(abs_y, abs_x, pattern[index]);
        }
    }

    attroff(COLOR_PAIR(3));
}

void draw_logo() {
    int start_y = 0;
    const char *logo[] = {
        "Welcome to",
        "",
        "MMP\"\"MM\"\"YMM",
        "P'   MM   `7",
        "     MM  .gP\"Ya  ,pP\"Ybd ,pP\"Ybd  .gP\"Ya `7Mb,od8 ,6\"Yb.",
        "     MM ,M'   Yb 8I   `\" 8I   `\" ,M'   Yb  MM' \"'8)   MM",
        "     MM 8M\"\"\"\"\"\" `YMMMa. `YMMMa. 8M\"\"\"\"\"\"  MM     ,pm9MM",
        "     MM YM.    , L.   I8 L.   I8 YM.    ,  MM    8M   MM",
        "   .JMML.`Mbmmd' M9mmmP' M9mmmP'  `Mbmmd'.JMML.  `Moo9^Yo",
        "",
        "                                                     v0.1"
    };

    int line;

    attron(COLOR_PAIR(1) | A_BOLD); // Cyan and Bold
    for (line = 0; line < 11; line++) {
        mvprintw(start_y + line, 2, "%s", logo[line]);
    }
    attroff(COLOR_PAIR(1) | A_BOLD);
}

void draw_progress_bar(int x, int width, float percentage) {
    // Clamp progress
    if (percentage < 0) percentage = 0.0f;
    if (percentage > 1.0f) percentage = 1.0f;
    // Draw the container box
    mvprintw(BAR_START_Y, x, "╔");
    for(int i=0; i<width-2; i++) printw("═");
    printw("╗");

    mvprintw(BAR_START_Y+1, x, "║");

    // Calculate total sub-blocks (8 per character);
    int total_sub_blocks = (width - 2) * 8; 
    int filled_sub_blocks = (int)(total_sub_blocks * percentage);
    
    // 4. Calculate full blocks and the remainder
    int full_blocks = filled_sub_blocks / 8;
    int remainder = filled_sub_blocks % 8;

    attron(COLOR_PAIR(4)); // Green

    // A. Print Full Blocks
    for(int i=0; i<full_blocks; i++) printw("█");

    // B. Print the Partial Block (if needed)
    if (remainder > 0) {
        // Array of partial blocks: index 0 is 1/8th, index 7 is full
        // Note: We use remainder-1 because 1/8th is index 0
        const char* partials[] = {"▏", "▎", "▍", "▌", "▋", "▊", "▉"};
        printw("%s", partials[remainder - 1]);
    }

    attroff(COLOR_PAIR(4));

    // C. Fill the rest with empty space
    // If we printed a partial block, that took up 1 cell.
    int printed_cells = full_blocks + (remainder > 0 ? 1 : 0);
    for(int i=printed_cells; i<width-2; i++) printw(" ");

    mvprintw(BAR_START_Y+1, x + width - 1, "║"); // Close the right side

    // Bottom border
    mvprintw(BAR_START_Y+2, x, "╚");
    for(int i=0; i<width-2; i++) printw("═");
    printw("╝");

    // Optional: Print exact percentage text
    int secs_left = ((resolution * resolution) - progress) / framerate;
    int mins_left = secs_left / 60;
    secs_left = secs_left % 60;
    char progress_str[32];
    char time_str[32];
    int gap;

    snprintf(progress_str, 32,"%.1f%% - Sample %d / %d", percentage  * 100.0f, progress, resolution * resolution);
    snprintf(time_str, 32, "%dm %ds Left", mins_left, secs_left);

    gap = 48 - strlen(progress_str) - strlen(time_str);

    mvprintw(BAR_START_Y+3, x+1, "%s%*s%s", progress_str, gap, "", time_str);
}

int main() {
    int height, width;

    // 1. Setup Locale for UTF-8 (Crucial for box characters)
    setlocale(LC_ALL, "");

    // 2. Initialize Ncurses
    initscr();            // Start curses mode
    cbreak();             // Disable line buffering (pass input immediately)
    noecho();             // Don't print what user types
    curs_set(0);          // Hide the cursor
    keypad(stdscr, TRUE); // Enable Arrow Keys

    // 3. Color Setup
    if (has_colors()) {
        start_color();

        int bg_color = COLOR_BLACK;
        if (use_default_colors() == OK) {
            bg_color = -1;
        }

        // Initialize pairs with the safe background color
        init_pair(1, COLOR_CYAN, bg_color);
        init_pair(2, COLOR_WHITE, bg_color);
        init_pair(3, COLOR_BLUE, bg_color);
        init_pair(4, COLOR_GREEN, bg_color);
    }

    dropdowns_init();

    while(1) {
        getmaxyx(stdscr, height, width); // Get terminal size
        erase(); // Clear screen buffer

        // Draw layers
        draw_bricks(BG_START_X, 0, (width - BG_START_X), height);
        draw_logo();
        dropdowns_draw();
        if(progress > 0) {
            draw_progress_bar(5, 50, (float)progress / (resolution * resolution));
        }
        dropdowns_draw_menu();

        refresh(); // Push buffer to screen

        dropdowns_navigate();
    }

    // 5. Cleanup
    endwin();
    return 0;
}