#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include "../advanced/theme.h"
#include "../advanced/tree-sitter/tree_manager.h"
#include "../data-management/file_structure.h"


#define USE_COLOR

////// ---------------- HIGHLIGHTS DESCRIPTOR ---------------

typedef struct {
	int abs_row;
	int abs_column;
} FilePosition;

typedef struct {
	// Color descriptor
	NCURSES_PAIRS_T color;
	uint16_t color_priority;

	// Attributes descriptor
	attr_t attributes;
	uint16_t a_italic_priority;
	uint16_t a_bold_priority;
	uint16_t a_underline_priority;
	uint16_t a_reverse_priority;
	uint16_t a_dim_priority;
	uint16_t a_standout_priority;
	uint16_t a_blink_priority;
	uint16_t a_protect_priority;
	uint16_t a_invis_priority;

	// Part
	FilePosition begin;
	FilePosition end;
} TextPartHighlightDescriptor;

#define DESCRIPTOR_SIZE_HISTORY_LENGTH 10

typedef struct {
	int size;
	TextPartHighlightDescriptor* descriptors;
	int capacity;
	uint32_t maximum_size_history[DESCRIPTOR_SIZE_HISTORY_LENGTH];
	int last_resize;
} WindowHighlightDescriptor;

void tphd_init(TextPartHighlightDescriptor *self, FilePosition begin, FilePosition end);

void whd_init(WindowHighlightDescriptor* self);

void whd_insertDescriptor(WindowHighlightDescriptor* self, Cursor begin, Cursor end, NCURSES_PAIRS_T color, attr_t attributes,
													uint16_t priority, bool override_attributes);

void whd_print(WindowHighlightDescriptor* self);

void whd_reset(WindowHighlightDescriptor* self);

void whd_free(WindowHighlightDescriptor* self);

////// ---------------- COLOR FUNCTIONS ---------------

void initColorsForTheme(HighlightThemeList theme_list, int* color_index, int* color_pair);


void highlightLinePartWithBytes(WINDOW* ftw, int start_row_byte, int start_column_byte, int length_byte, attr_t attr,
																NCURSES_PAIRS_T color, Cursor cursor, Cursor select,
																Cursor* tmp, int screen_y,
																int screen_x);

void highlightLinePart(WINDOW* ftw, int start_row, int start_column, int length, attr_t attr, NCURSES_PAIRS_T color,
											Cursor cursor, Cursor select, Cursor* tmp, int screen_y,
											int screen_x);


void highlightCurrentFile(FileHighlightDatas* highlight_data, WINDOW* ftw, int screen_x, int screen_y, Cursor cursor,
													Cursor select_cursor);

#endif //HIGHLIGHT_H
