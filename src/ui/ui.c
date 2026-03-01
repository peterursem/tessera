#include "ui.h"
#include "../app.h"

#include <curses.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

/*

	UI Constants

*/

#define LOGO_X 2
#define LOGO_Y 0

#define FORM_START_Y 12
#define FORM_START_X 5

#define BAR_X 5
#define BAR_Y 14
#define BAR_WIDTH 50

#define BG_X 61

/*

	Paint a brick ASCII pattern in a square.

*/
void draw_bricks(int start_x, int start_y, int width, int height, int colour)
{
	const char *pattern = "\\  \\__"; // One unit of the brick pattern
	const int pat_len = 6;
	const int pat_offset = 2; // Offset for each column

	int end_x = start_x + width;
	int end_y = start_y + height;

	attron(COLOR_PAIR(colour));

	for (int y = start_y; y < end_y; y++)
	{
		for (int x = start_x; x < end_x; x++)
		{
			// Get the character index
			int index = (x + (y * pat_offset)) % pat_len;
			mvaddch(y, x, pattern[index]);
		}
	}

	attroff(COLOR_PAIR(colour));
}

/*

	Draw the ASCII logo
	x, y: top left position of the logo
	colour: the COLOR_PAIR number to use

*/
void draw_logo(int x, int y, int colour)
{
	const char logo_lines = 11;
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
		"                                                     v0.1"};

	attron(COLOR_PAIR(colour) | A_BOLD);

	for (int line = 0; line < logo_lines; line++)
	{
		mvprintw(y + line, x, "%s", logo[line]);
	}

	attroff(COLOR_PAIR(colour) | A_BOLD);
}

/*

	Draw a progress bar using UTF-8 blocks

*/
void draw_progress_bar(int x, int y, int width, float percentage)
{
	// ---- Bar visual vars ----

	// Characters for the loading bar
	// Each partial block is 1/8 of a character
	const char *partials[] = {" ", "▏", "▎", "▍", "▌", "▋", "▊", "▉"};

	// Length of the bar in partials (1/8 of a character)
	int bar_length = (width - 2) * 8;
	int bar_filled = (int)(bar_length * percentage);

	// A full block is 8 partials
	int full_blocks = bar_filled / 8;
	// Remainder to show as a partial block
	int remainder = bar_filled % 8;

	// ---- Bar text vars ----

	int total_pixels = app_status.resolution * app_status.resolution;
	int secs_left = (total_pixels - app_status.progress) / (app_status.framerate * app_status.batch_size);
	int mins_left = secs_left / 60;
	char progress_str[32]; // The formatted progress string "x% - Sample y/z"
	char time_str[32];	   // The formatted time string "xm ys remaining"
	int gap;			   // The gap to put between the progress and time strings
	secs_left = secs_left % 60;

	// Clamp progress
	if (percentage < 0)
		percentage = 0.0f;
	if (percentage > 1.0f)
		percentage = 1.0f;

	// Draw the top of the container
	mvprintw(y, x, "╔");
	for (int i = 0; i < width - 2; i++)
		printw("═");
	printw("╗");

	// Bottom of the container
	mvprintw(y + 2, x, "╚");
	for (int i = 0; i < width - 2; i++)
		printw("═");
	printw("╝");

	// Container left side
	mvprintw(y + 1, x, "║");

	// Draw the progress bar
	attron(COLOR_PAIR(COLOR_GREEN));

	for (int i = 0; i < full_blocks; i++)
		printw("█");
	printw("%s", partials[remainder]);

	attroff(COLOR_PAIR(COLOR_GREEN));

	// Container right side
	mvprintw(y + 1, x + width - 1, "║");

	// Format the bottom text
	snprintf(progress_str, 32, "%.1f%% - Sample %d / %d", percentage * 100.0f, app_status.progress, total_pixels);
	snprintf(time_str, 32, "%dm %ds Left", mins_left, secs_left);
	gap = width - 2 - strlen(progress_str) - strlen(time_str);
	// Draw the bottom text
	mvprintw(y + 3, x + 1, "%s%*s%s", progress_str, gap, "", time_str);
}

Form *ui_init()
{
	// UI Form
	Form *form = (Form *)malloc(sizeof(Form));

	// Initialize Ncurses
	setlocale(LC_ALL, "");
	initscr();			  // Start curses mode
	cbreak();			  // Disable line buffering (pass input immediately)
	noecho();			  // Don't print what user types
	curs_set(0);		  // Hide the cursor
	keypad(stdscr, TRUE); // Enable Arrow Keys

	// Color Pair Setup
	if (has_colors())
	{
		start_color();

		// Use a transparent bg if it is supported, otherwise use black
		int bg_color = COLOR_BLACK;
		if (use_default_colors() == OK)
		{
			bg_color = -1;
		}

		// Initialize pairs with the safe background color
		init_pair(COLOR_WHITE, COLOR_WHITE, bg_color);
		init_pair(COLOR_CYAN, COLOR_CYAN, bg_color);
		init_pair(COLOR_BLUE, COLOR_BLUE, bg_color);
		init_pair(COLOR_GREEN, COLOR_GREEN, bg_color);
	}

	// Fill out menu options
	form_init(form);

	return form;
}

void ui_frame(Form *form)
{
	int height, width;				 // Terminal dimensions
	getmaxyx(stdscr, height, width); // Get terminal size
	erase();						 // Clear screen buffer

	// Draw layers
	draw_bricks(BG_X, 0, width - BG_X, height, COLOR_BLUE); // Background: Bricks from top of the screen to the bottom
	draw_logo(LOGO_X, LOGO_Y, COLOR_CYAN);					// Top logo
	if (form != NULL) {
		form_draw(FORM_START_X, FORM_START_Y, form);	// Draw menu options
		dropdowns_draw_menu(form);	// Draw the dropdown menus if one is open
		dropdowns_navigate(form);	// Accept key input for menus
	} else {
		// When the form is inactive show the progress bar
		draw_progress_bar(BAR_X, BAR_Y, BAR_WIDTH, (float)app_status.progress / (app_status.resolution * app_status.resolution));
	}

	refresh(); // Push buffer to screen
}

void ui_loop(Form *form)
{
	while (form == NULL || form->form_active)
	{
		ui_frame(form);
	}
}

void *open_ui(void *args) {
	Form *form = ui_init();

	// Loop menu
	ui_loop(form);
	free(form);

	// Loop progress bar
	ui_loop(NULL);

	endwin();
	return NULL;
}