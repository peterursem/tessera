#include "dropdown.h"
#include <ncurses.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "manager.h"
#include "app.h"

#define TOTAL_FIELDS 3

#define AVAILABLE_RESOLUTIONS 8 // From 2^4 to 2^(4+x)
#define FORM_START_Y 12

#define ENTER_KEY 10
#define ESC_KEY 27

// Global State
Dropdown fields[TOTAL_FIELDS];
int focused_field = 0; // Which field is highlighted (0-2)
int menu_open = 0;	   // 0 = closed, 1 = open
int form_active = 1;
int menu_selection = 0; // Which item inside the open menu is highlighted

int framerates[] = {1, 2, 4, 6, 12, 24, 30, 48, 60, 120, 144};

pthread_t app_thread;

void dropdowns_init()
{
	// Field 0: Resolution
	char *resolution_label = (char *)malloc(11 * sizeof(char));
	int resolution_option = 0;
	Dropdown *field;

	field = &fields[0];
	strcpy(field->label, "Resolution:");
	field->y_pos = FORM_START_Y;
	field->option_count = AVAILABLE_RESOLUTIONS;
	for (int i = 0; i < AVAILABLE_RESOLUTIONS; i++)
	{
		resolution_option = 2 << (3 + i);
		sprintf(resolution_label, "%d x %d", resolution_option, resolution_option);
		strcpy(field->options[i], resolution_label);
	}
	field->current_selection = 2;
	free(resolution_label);

	// Field 1: Sensor Port
	int framerate_count = 11;
	char *framerate_label = (char *)malloc(3 * sizeof(char));

	field = &fields[1];
	strcpy(field->label, "Sample Rate:");
	field->y_pos = FORM_START_Y + 2;
	field->current_selection = 2;
	field->option_count = framerate_count;
	for (int i = 0; i < framerate_count; i++)
	{
		sprintf(framerate_label, "%d", framerates[i]);
		strcpy(field->options[i], framerate_label);
	}
	free(framerate_label);

	// Field 2: Sensor Port
	DIR *dir;
	struct dirent *entry;
	int sensors = 0;

	field = &fields[2];
	strcpy(field->label, "Sensor Port:");
	field->y_pos = FORM_START_Y + 4;
	field->current_selection = 0;
	// Find available sensors in /dev/
	dir = opendir("/dev/");
	if (dir)
	{
		while ((entry = readdir(dir)) != NULL)
		{
			if (!memcmp(entry->d_name, "cu.", 3))
			{
				strcpy(field->options[sensors], entry->d_name);
				if (!memcmp(entry->d_name, "cu.usb", 6))
				{
					field->current_selection = sensors;
				}
				sensors++;
			}
		}
	}
	field->option_count = sensors;
}

void dropdowns_draw()
{
	int pad = 0;
	int endPad = 0;
	for (int i = 0; i < TOTAL_FIELDS; i++)
	{
		pad = (31 - strlen(fields[i].options[fields[i].current_selection])) / 2;
		endPad = pad - ((strlen(fields[i].options[fields[i].current_selection]) % 2));
		// If this field is focused and menu is NOT open, highlight it
		if (i == focused_field && !menu_open)
		{
			attron(A_REVERSE); // Invert colors
		}

		// Draw the Label and the Current Value
		mvprintw(fields[i].y_pos, 5, "%-15s [ %*s%s%*s ]",
				 fields[i].label,
				 pad, " ", fields[i].options[fields[i].current_selection], endPad, " ");

		attroff(A_REVERSE);
	}

	if (focused_field == TOTAL_FIELDS)
	{
		attron(A_REVERSE);
	}
	attron(COLOR_PAIR(4));
	mvprintw(fields[TOTAL_FIELDS - 1].y_pos + 2, 21, "[              Start             ]");
	attroff(COLOR_PAIR(4));
	attroff(A_REVERSE);
}

void dropdowns_draw_menu()
{
	if (!menu_open)
		return;

	Dropdown *field = &fields[focused_field];

	// Calculate position: right below the field
	int menu_y = field->y_pos + 1;
	int menu_x = 21; // Align with the bracket [
	int pad = 0;
	int endPad = 0;

	// Draw "Shadow" box (The dropdown background)
	attron(COLOR_PAIR(1));

	// Top Border
	mvprintw(menu_y, menu_x, "┌────────────────────────────────┐");

	for (int i = 0; i < field->option_count; i++)
	{
		pad = (31 - strlen(field->options[i])) / 2;
		endPad = pad - ((strlen(field->options[i]) % 2));
		mvprintw(menu_y + 1 + i, menu_x, "│"); // Left edge

		// Highlight the item we are currently hovering over inside the menu
		if (i == menu_selection)
		{
			attron(A_REVERSE);
		}
		printw(" %*.*s%s%*.*s ", pad, pad, " ", field->options[i], endPad, endPad, " ");
		attroff(A_REVERSE);

		// Turn color pair back on after turning A_REVERSE off
		attron(COLOR_PAIR(1));
		printw("│"); // Right edge
	}

	// Bottom Border
	mvprintw(menu_y + 1 + field->option_count, menu_x, "└───────-────────────────────────┘");
	attroff(COLOR_PAIR(3));
}

void dropdowns_start()
{
	// Set app variables
	resolution = 2 << (3 + fields[0].current_selection);
	framerate = framerates[fields[1].current_selection];
	sprintf(sensor_port, "/dev/%s", fields[2].options[fields[2].current_selection]);

	// Start a new thread to run the patterns and reconstruction
	if (!pthread_create(&app_thread, NULL, app_main, NULL))
	{
		form_active = 0;
		return;
	}
}

void dropdowns_navigate()
{
	if (form_active) {
	// Simple input check to exit
    int ch = getch();
	if (menu_open)
	{
		// == NAVIGATING INSIDE A MENU ==
		switch (ch)
		{
		case KEY_UP:
			menu_selection--;
			if (menu_selection < 0)
				menu_selection = fields[focused_field].option_count - 1;
			break;
		case KEY_DOWN:
			menu_selection++;
			if (menu_selection >= fields[focused_field].option_count)
				menu_selection = 0;
			break;
		case ENTER_KEY: // Confirm Selection
			fields[focused_field].current_selection = menu_selection;
			menu_open = 0;
			break;
		case ESC_KEY: // Cancel
			menu_open = 0;
			break;
		}
	}
	else
	{
		// == NAVIGATING THE FORM ==
		switch (ch)
		{
		case KEY_UP:
			focused_field--;
			if (focused_field < 0)
				focused_field = TOTAL_FIELDS;
			break;
		case KEY_DOWN:
			focused_field++;
			if (focused_field > TOTAL_FIELDS)
				focused_field = 0;
			break;
		case ENTER_KEY: // Open Menu
			if (focused_field == TOTAL_FIELDS)
				dropdowns_start();
			else
			{
				menu_open = 1;
				// Reset menu cursor to the current value
				menu_selection = fields[focused_field].current_selection;
			}
			break;
		}
	} 
}else {
		usleep(500000);
	}
}