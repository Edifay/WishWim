#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include "../advanced/theme.h"
#include "../advanced/tree-sitter/tree_manager.h"
#include "../data-management/file_structure.h"


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
  // TODO do something with this history to better manage the capacity
  uint32_t maximum_size_history[DESCRIPTOR_SIZE_HISTORY_LENGTH];
  int last_resize;
} WindowHighlightDescriptor;

void tphd_init(TextPartHighlightDescriptor* self, FilePosition begin, FilePosition end);

bool tphd_isCursorIn(TextPartHighlightDescriptor* self, Cursor cursor);

bool tphd_isCursorAfter(TextPartHighlightDescriptor* self, Cursor cursor);

void whd_init(WindowHighlightDescriptor* self);

void whd_insertDescriptor(WindowHighlightDescriptor* self, Cursor begin, Cursor end, NCURSES_PAIRS_T color, attr_t attributes,
                          uint16_t priority, bool override_attributes);

void whd_print(WindowHighlightDescriptor* self);

void whd_reset(WindowHighlightDescriptor* self);

void whd_free(WindowHighlightDescriptor* self);

TextPartHighlightDescriptor* whd_tphd_forCursorWithOffsetIndex(WindowHighlightDescriptor* highlight_descriptor, Cursor cursor,
                                                             int* offset_index);

////// ---------------- COLOR FUNCTIONS ---------------

void initColorsForTheme(HighlightThemeList theme_list, int* color_index, int* color_pair);

void highlightCurrentFile(FileHighlightDatas* highlight_data, WINDOW* ftw, int screen_x, int screen_y, Cursor cursor,
                          WindowHighlightDescriptor* highlight_descriptor);

#endif //HIGHLIGHT_H
