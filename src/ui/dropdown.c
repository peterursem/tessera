#include "dropdown.h"
#include "../app.h"

#include <curses.h> // Terminal drawing
#include <dirent.h> // Get serial port
#include <stdlib.h> // printing
#include <string.h> // Strcpy

#define AVAILABLE_RESOLUTIONS 8 // From 2^4 to 2^(4+x)
#define FORM_GAP 2

#define ENTER_KEY 10
#define ESC_KEY 27

int framerates[] = {1, 2, 4, 6, 12, 24, 30, 48, 60, 120, 144};

/*

	Create a new form object with dropdowns.

*/
void form_init(Form *form)
{
	form->focused_field = 0;  // First field is highlighted
	form->menu_open = 0;	  // Menu starts closed
	form->form_active = 1;	  // Form is navigable
	form->menu_selection = 0; // Menu start at the top

	// Field 0: Resolution
	char *resolution_label = (char *)malloc(11 * sizeof(char));
	Dropdown *field;

	field = &form->fields[0];
	// Set dropdown Label
	strcpy(field->label, "Resolution:");
	// Set options
	field->option_count = AVAILABLE_RESOLUTIONS;
	for (int i = 0; i < AVAILABLE_RESOLUTIONS; i++)
	{
		int resolution_option = 2 << (3 + i);
		// Set option label
		sprintf(resolution_label, "%d x %d", resolution_option, resolution_option);
		strcpy(field->options[i], resolution_label);
	}
	// Set default
	field->current_selection = 2;
	free(resolution_label);

	// Field 1: Sensor Port
	int framerate_count = 11;
	// If using faster sample rates than 9,999 the allocation for labels needs to be increased
	char *framerate_label = (char *)malloc(4 * sizeof(char));

	field = &form->fields[1];
	// Set dropdown label
	strcpy(field->label, "Sample Rate:");
	// Set default selection
	field->current_selection = 2;
	field->option_count = framerate_count;
	for (int i = 0; i < framerate_count; i++)
	{
		// Set option label
		sprintf(framerate_label, "%d", framerates[i]);
		strcpy(field->options[i], framerate_label);
	}
	free(framerate_label);

	// Field 2: Sensor Port
	DIR *dir;
	struct dirent *entry;
	int sensors = 0;

	field = &form->fields[2];
	// Set dropdown label
	strcpy(field->label, "Sensor Port:");
	// Default
	field->current_selection = 0;
	// Find available sensor options in /dev/
	dir = opendir("/dev/");
	if (dir)
	{
		while ((entry = readdir(dir)) != NULL)
		{
			if (!memcmp(entry->d_name, "cu.", 3) || !memcmp(entry->d_name, "tty", 3))
			{
				strcpy(field->options[sensors], entry->d_name);
				if (!memcmp(entry->d_name, "cu.usb", 6))
				{
					// If cu.usb is found, make that the default
					field->current_selection = sensors;
				}
				sensors++;
			}
		}
	}
	field->option_count = sensors;
}

/*

	Draw all the fields in the given form.

*/
void form_draw(int x, int y, Form *form)
{
	for (int i = 0; i < FORM_TOTAL_FIELDS; i++)
	{
		int pad = (31 - strlen(form->fields[i].options[form->fields[i].current_selection])) / 2;
		// Remove padding from the end if the string length is even
		int endPad = pad - ((strlen(form->fields[i].options[form->fields[i].current_selection]) % 2));

		// If this field is focused and menu is NOT open, highlight it
		if (i == form->focused_field && !form->menu_open)
		{
			attron(A_REVERSE); // Invert colors
		}

		// Draw the Label and the Current Value
		mvprintw(y + (2 * i), x, "%-15s [ %*s%s%*s ]",
				 form->fields[i].label,
				 pad, " ",
				 form->fields[i].options[form->fields[i].current_selection],
				 endPad, " ");

		// Return to regular text in case a highlight was applied
		attroff(A_REVERSE);
	}

	// If the start button is selected, highlight it
	if (form->focused_field == FORM_TOTAL_FIELDS)
	{
		attron(A_REVERSE);
	}

	// Draw the start button
	attron(COLOR_PAIR(COLOR_GREEN));
	mvprintw(y + (2 * FORM_TOTAL_FIELDS), 21, "[              Start             ]");
	attroff(COLOR_PAIR(COLOR_GREEN));

	// Return to regular text if a highlight was applied
	attroff(A_REVERSE);
}

void dropdowns_draw_menu(Form *form)
{
	if (!form->menu_open)
		return;

	Dropdown *field = &form->fields[form->focused_field];

	// Calculate position: right below the field
	int menu_y = field->y_pos + 1;
	int menu_x = 21; // Align with the bracket [
	int pad = 0;
	int endPad = 0;

	// Draw "Shadow" box (The dropdown background)
	attron(COLOR_PAIR(COLOR_CYAN));

	// Top Border
	mvprintw(menu_y, menu_x, "┌────────────────────────────────┐");

	for (int i = 0; i < field->option_count; i++)
	{
		pad = (31 - strlen(field->options[i])) / 2;
		endPad = pad - ((strlen(field->options[i]) % 2));
		mvprintw(menu_y + 1 + i, menu_x, "│"); // Left edge

		// Highlight the item we are currently hovering over inside the menu
		if (i == form->menu_selection)
		{
			attron(A_REVERSE);
		}
		printw(" %*.*s%s%*.*s ", pad, pad, " ", field->options[i], endPad, endPad, " ");
		attroff(A_REVERSE);

		// Turn color pair back on after turning A_REVERSE off
		attron(COLOR_PAIR(COLOR_CYAN));
		printw("│"); // Right edge
	}

	// Bottom Border
	mvprintw(menu_y + 1 + field->option_count, menu_x, "└───────-────────────────────────┘");
	attroff(COLOR_PAIR(COLOR_CYAN));
}

void form_start_handler(Form *form)
{
	char *sensor_selection = (char *)&form->fields[2].options[form->fields[2].current_selection];

	// Set app variables
	app_status.resolution = 2 << (3 + form->fields[0].current_selection);
	app_status.framerate = framerates[form->fields[1].current_selection];
	sprintf((char *)app_status.sensor_port, "/dev/%s", sensor_selection);

	app_status.active = 1;
	form->form_active = 0;
}

void dropdowns_navigate(Form *form)
{
	if (form->form_active)
	{
		// Simple input check to exit
		int ch = getch();
		if (form->menu_open)
		{
			// == NAVIGATING INSIDE A MENU ==
			switch (ch)
			{
			case KEY_UP:
				form->menu_selection--;
				if (form->menu_selection < 0)
					form->menu_selection = form->fields[form->focused_field].option_count - 1;
				break;
			case KEY_DOWN:
				form->menu_selection++;
				if (form->menu_selection >= form->fields[form->focused_field].option_count)
					form->menu_selection = 0;
				break;
			case ENTER_KEY: // Confirm Selection
				form->fields[form->focused_field].current_selection = form->menu_selection;
				form->menu_open = 0;
				break;
			case ESC_KEY: // Cancel
				form->menu_open = 0;
				break;
			}
		}
		else
		{
			// == NAVIGATING THE FORM ==
			switch (ch)
			{
			case KEY_UP:
				form->focused_field--;
				if (form->focused_field < 0)
					form->focused_field = FORM_TOTAL_FIELDS;
				break;
			case KEY_DOWN:
				form->focused_field++;
				if (form->focused_field > FORM_TOTAL_FIELDS)
					form->focused_field = 0;
				break;
			case ENTER_KEY: // Open Menu
				if (form->focused_field == FORM_TOTAL_FIELDS)
				{
					form_start_handler(form);
				}
				else
				{
					form->menu_open = 1;
					// Reset menu cursor to the current value
					form->menu_selection = form->fields[form->focused_field].current_selection;
				}
				break;
			}
		}
	}
}