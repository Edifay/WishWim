#include "highlight.h"

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <time.h>

#include "term_handler.h"
#include "../advanced/tree-sitter/tree_query.h"
#include "../data-management/file_structure.h"
#include "../utils/constants.h"


void tphd_init(TextPartHighlightDescriptor* self, FilePosition begin, FilePosition end) {
  self->begin = begin;
  self->end = end;


  self->color = DEFAULT_COLOR_PAIR;
  self->color_priority = 0;

  self->attributes = A_NORMAL;
  self->a_blink_priority =
      self->a_bold_priority =
      self->a_dim_priority =
      self->a_invis_priority =
      self->a_italic_priority =
      self->a_protect_priority =
      self->a_reverse_priority =
      self->a_standout_priority =
      self->a_underline_priority = 0;
}

bool tphd_isCursorIn(TextPartHighlightDescriptor* self, Cursor cursor) {
  int row = cursor.file_id.absolute_row;
  int column = cursor.line_id.absolute_column;

  int row_start = self->begin.abs_row;
  int column_start = self->begin.abs_column;

  int row_end = self->end.abs_row;
  int column_end = self->end.abs_column;

  return (row_start < row || (row_start == row && column_start < column))
         && (row < row_end || (row == row_end && column <= column_end));
}

bool tphd_isCursorAfter(TextPartHighlightDescriptor* self, Cursor cursor) {
  int row = cursor.file_id.absolute_row;
  int column = cursor.line_id.absolute_column;

  int row_end = self->end.abs_row;
  int column_end = self->end.abs_column;

  return (row > row_end || (row == row_end && column > column_end));
}

void whd_init(WindowHighlightDescriptor* self) {
  self->descriptors = NULL;
  self->size = 0;
  self->capacity = 0;
  self->last_resize = 0;
  for (int i = 0; i < DESCRIPTOR_SIZE_HISTORY_LENGTH; i++) {
    self->maximum_size_history[i] = 0;
  }
}


bool isPositionBeforePosition(FilePosition cursor, FilePosition position) {
  return position.abs_row > cursor.abs_row || (
           position.abs_row == cursor.abs_row && position.abs_column > cursor.abs_column);
}

bool isPositionAfterPosition(FilePosition cursor, FilePosition position) {
  return cursor.abs_row > position.abs_row || (
           position.abs_row == cursor.abs_row && cursor.abs_column > position.abs_column);
}

bool isPositionAfterOrEqualPosition(FilePosition cursor, FilePosition position) {
  return !isPositionBeforePosition(cursor, position);
}

bool isPositionBeforeOrEqualPosition(FilePosition cursor, FilePosition position) {
  return !isPositionAfterPosition(cursor, position);
}

bool arePositionEquals(FilePosition pos1, FilePosition pos2) {
  return pos1.abs_row == pos2.abs_row && pos1.abs_column == pos2.abs_column;
}


FilePosition minPosition(FilePosition pos1, FilePosition pos2) {
  if (isPositionBeforePosition(pos1, pos2)) {
    return pos1;
  }
  return pos2;
}

void tphd_mergeAttributes(TextPartHighlightDescriptor* self, NCURSES_PAIRS_T color, attr_t attributes, uint16_t priority,
                          bool override_attributes) {
  if (self->color_priority <= priority) {
    self->color = color;
    self->color_priority = priority;
  }

  if (self->a_italic_priority <= priority) {
    if (attributes & A_ITALIC) {
      self->attributes |= A_ITALIC;
    }
    else if (override_attributes) {
      self->attributes &= ~A_ITALIC;
    }
    self->a_italic_priority = priority;
  }
  if (self->a_bold_priority <= priority) {
    if (attributes & A_BOLD) {
      self->attributes |= A_BOLD;
    }
    else if (override_attributes) {
      self->attributes &= ~A_BOLD;
    }
    self->a_bold_priority = priority;
  }
  if (self->a_underline_priority <= priority) {
    if (attributes & A_UNDERLINE) {
      self->attributes |= A_UNDERLINE;
    }
    else if (override_attributes) {
      self->attributes &= ~A_UNDERLINE;
    }
    self->a_underline_priority = priority;
  }
  if (self->a_reverse_priority <= priority) {
    if (attributes & A_REVERSE) {
      self->attributes |= A_REVERSE;
    }
    else if (override_attributes) {
      self->attributes &= ~A_REVERSE;
    }
    self->a_reverse_priority = priority;
  }
  if (self->a_dim_priority <= priority) {
    if (attributes & A_DIM) {
      self->attributes |= A_DIM;
    }
    else if (override_attributes) {
      self->attributes &= ~A_DIM;
    }
    self->a_dim_priority = priority;
  }
  if (self->a_standout_priority <= priority) {
    if (attributes & A_STANDOUT) {
      self->attributes |= A_STANDOUT;
    }
    else if (override_attributes) {
      self->attributes &= ~A_STANDOUT;
    }
    self->a_standout_priority = priority;
  }
  if (self->a_blink_priority <= priority) {
    if (attributes & A_BLINK) {
      self->attributes |= A_BLINK;
    }
    else if (override_attributes) {
      self->attributes &= ~A_BLINK;
    }
    self->a_blink_priority = priority;
  }
  if (self->a_protect_priority <= priority) {
    if (attributes & A_PROTECT) {
      self->attributes |= A_PROTECT;
    }
    else if (override_attributes) {
      self->attributes &= ~A_PROTECT;
    }
    self->a_protect_priority = priority;
  }
  if (self->a_invis_priority <= priority) {
    if (attributes & A_INVIS) {
      self->attributes |= A_INVIS;
    }
    else if (override_attributes) {
      self->attributes &= ~A_INVIS;
    }
    self->a_invis_priority = priority;
  }
}


void whd_insertDescriptor(WindowHighlightDescriptor* self, Cursor begin, Cursor end, NCURSES_PAIRS_T color, attr_t attributes,
                          uint16_t priority, bool override_attributes) {
  FilePosition current_pos = {begin.file_id.absolute_row, begin.line_id.absolute_column};
  FilePosition end_pos = {end.file_id.absolute_row, end.line_id.absolute_column};

  // reach the WindowHighlightDescriptor position
  int i = 0;
  while (i < self->size && isPositionBeforePosition(self->descriptors[i].begin, current_pos)) {
    if (isPositionAfterPosition(current_pos, self->descriptors[i].end)) {
      // after the whole text-part
      i++;
    }
    else {
      // the start is cutting the text-part i
      break;
    }
  }

  while (isPositionBeforePosition(current_pos, end_pos)) {
    FilePosition new_field_end;
    bool is_merge = false;
    if (i >= self->size) {
      // nothing next
      new_field_end = end_pos;
    }
    else if (isPositionBeforePosition(current_pos, self->descriptors[i].begin)) {
      // start in the middle of an empty space
      Cursor prev = moveLeft(tryToReachAbsPosition(begin, current_pos.abs_row, current_pos.abs_column));
      new_field_end = minPosition((FilePosition){prev.file_id.absolute_row, prev.line_id.absolute_column}, end_pos);
    }
    else {
      // so the current_pos is in a field.
      new_field_end = minPosition(self->descriptors[i].end, end_pos);
      is_merge = true;
    }


    if (!is_merge) {
      self->size += 1;

      // need to allocate more memory.
      if (self->capacity < self->size) {
        // TODO implement a cache system
        self->capacity = self->size;
        self->descriptors = realloc(self->descriptors, self->capacity * sizeof(TextPartHighlightDescriptor));
        assert(self->descriptors != NULL);
      }

      if (self->size - i - 1 > 0) {
        memmove(self->descriptors + i + 1, self->descriptors + i, sizeof(TextPartHighlightDescriptor) * (self->size - i - 1));
      }

      tphd_init(self->descriptors + i, current_pos, new_field_end);
      tphd_mergeAttributes(self->descriptors + i, color, attributes, priority, override_attributes);

      i++;
    }
    else {
      // merging
      bool before_offset = !arePositionEquals(self->descriptors[i].begin, current_pos);
      bool after_offset = !arePositionEquals(self->descriptors[i].end, new_field_end);

      int increase_size = (before_offset ? 1 : 0) + (after_offset ? 1 : 0);
      if (increase_size != 0) {
        self->size += increase_size;

        // need to allocate more memory.
        if (self->capacity < self->size) {
          // TODO implement a cache system
          self->capacity = self->size;
          self->descriptors = realloc(self->descriptors, self->capacity * sizeof(TextPartHighlightDescriptor));
          assert(self->descriptors != NULL);
        }


        // creating increase_size new case.
        if (self->size - i - increase_size > 0) {
          memmove(self->descriptors + i + increase_size, self->descriptors + i,
                  sizeof(TextPartHighlightDescriptor) * (self->size - i - increase_size));
        }

        // copy to new cases old data.
        for (int j = 0; j < increase_size; j++) {
          self->descriptors[i + j] = self->descriptors[i + increase_size];
        }
      }

      if (before_offset) {
        // should happen max once !
        Cursor end_before = moveLeft(tryToReachAbsPosition(begin, current_pos.abs_row, current_pos.abs_column));
        self->descriptors[i].end = (FilePosition){end_before.file_id.absolute_row, end_before.line_id.absolute_column};
      }

      int middle_offset = before_offset ? 1 : 0;
      self->descriptors[i + middle_offset].begin = current_pos;
      self->descriptors[i + middle_offset].end = new_field_end;

      tphd_mergeAttributes(self->descriptors + i + middle_offset, color, attributes, priority, override_attributes);

      if (after_offset) {
        // should happen max once !
        Cursor begin_after = moveRight(tryToReachAbsPosition(begin, new_field_end.abs_row, new_field_end.abs_column));
        self->descriptors[i + middle_offset + 1].begin = (FilePosition){
          begin_after.file_id.absolute_row, begin_after.line_id.absolute_column
        };
      }
      i += 1 + increase_size;
    }

    // Set the new current_pos
    Cursor new_current_pos = moveRight(tryToReachAbsPosition(begin, new_field_end.abs_row, new_field_end.abs_column));
    current_pos = (FilePosition){new_current_pos.file_id.absolute_row, new_current_pos.line_id.absolute_column};
  }
}


void stringForAttribute(attr_t attributes, char* text) {
  int i = 0;
  if (attributes & A_ITALIC) {
    text[i++] = 'i';
  }
  if (attributes & A_BOLD) {
    text[i++] = 'b';
  }
  if (attributes & A_UNDERLINE) {
    text[i++] = 'u';
  }
  if (attributes & A_REVERSE) {
    text[i++] = 'r';
  }
  if (attributes & A_DIM) {
    text[i++] = 'd';
  }
  if (attributes & A_STANDOUT) {
    text[i++] = 's';
  }
  if (attributes & A_BLINK) {
    text[i++] = 'k';
  }
  if (attributes & A_PROTECT) {
    text[i++] = 'p';
  }
  if (attributes & A_INVIS) {
    text[i++] = 'v';
  }
  text[i] = '\0';
}

void whd_print(WindowHighlightDescriptor* self) {
  fprintf(stderr, "Descriptor : size=%d, capacity=%d\n", self->size, self->capacity);
  for (int i = 0; i < self->size; i++) {
    TextPartHighlightDescriptor descriptor = self->descriptors[i];
    char attribute_str[100];
    stringForAttribute(descriptor.attributes, attribute_str);
    fprintf(stderr, "(%d, %d) -> (%d, %d) : color(%d), attribute('%s')\n", descriptor.begin.abs_row, descriptor.begin.abs_column,
            descriptor.end.abs_row,
            descriptor.end.abs_column, descriptor.color, attribute_str);
  }
}

void whd_reset(WindowHighlightDescriptor* self) {
  memmove(self->maximum_size_history, self->maximum_size_history + 1, (DESCRIPTOR_SIZE_HISTORY_LENGTH - 1) * sizeof(uint32_t));
  self->maximum_size_history[DESCRIPTOR_SIZE_HISTORY_LENGTH - 1] = self->size;
  self->size = 0;
}

void whd_free(WindowHighlightDescriptor* self) {
  free(self->descriptors);
}

TextPartHighlightDescriptor* whd_tphd_forCursorWithOffsetIndex(WindowHighlightDescriptor* highlight_descriptor, Cursor cursor,
                                                               int* offset_index) {
  // check to increment to the next
  TextPartHighlightDescriptor* current_highlight = NULL;
  if (*offset_index < highlight_descriptor->size) {
    current_highlight = highlight_descriptor->descriptors + *offset_index;

    // skip text_part until reach current position
    while (tphd_isCursorAfter(current_highlight, cursor) && *offset_index < highlight_descriptor->size - 1) {
      (*offset_index)++;
      current_highlight = highlight_descriptor->descriptors + *offset_index;
    }

    if (!tphd_isCursorIn(current_highlight, cursor)) {
      current_highlight = NULL;
    }
  }
  return current_highlight;
}


////// ---------------- COLOR FUNCTIONS ---------------


void initColorsForTheme(HighlightThemeList theme_list, int* color_index, int* color_pair) {
  // Setup color sheme.
  for (int i = 0; i < theme_list.size; i++) {
    init_color((*color_index)++, theme_list.groups[i].color.r, theme_list.groups[i].color.g, theme_list.groups[i].color.b);
    init_pair(*color_pair, *color_index - 1, COLOR_BLACK);
    theme_list.groups[i].color_n = *color_pair;

    init_color((*color_index)++, theme_list.groups[i].color_hover.r, theme_list.groups[i].color_hover.g,
               theme_list.groups[i].color_hover.b);
    init_pair(*color_pair + 1000, *color_index - 1, COLOR_HOVER);
    theme_list.groups[i].color_hover_n = *color_pair + 1000;
    (*color_pair)++;
  }
}

void saveCaptureToHighlightDescriptor(HighlightThemeList theme_list, Cursor tmp, TSNode node, const char* result,
                                      WindowHighlightDescriptor* highlight_descriptor, uint16_t priority) {
  if (result == NULL) {
    return;
  }

  attr_t attr = A_NORMAL;
  NCURSES_PAIRS_T color = DEFAULT_COLOR_PAIR;

  bool found = false;
  // Setup style for group found.
  for (int i = 0; i < theme_list.size; i++) {
    if (strcmp(theme_list.groups[i].group, result) == 0) {
      attr |= getAttrForTheme(theme_list.groups[i]);
      color = theme_list.groups[i].color_n;

      found = true;
      break;
    }
  }

  if (found == false) {
    // Quit if a query was found, but not theme for this group was found.
    fprintf(stderr, "      || No theme found for capture %s  || \n", result);
    return;
  }

  TSPoint start_point = ts_node_start_point(node);
  TSPoint end_point = ts_node_end_point(node);

  Cursor begin_cursor = byteCursorToCursor(tmp, start_point.row, start_point.column);
  Cursor end_cursor = byteCursorToCursor(tmp, end_point.row, end_point.column);
  whd_insertDescriptor(highlight_descriptor, begin_cursor, end_cursor, color, attr, priority, true);
}

void executeHighlightQuery(TSQuery* query, TSQueryCursor* qcursor, RegexMap* regex_map, HighlightThemeList theme_list,
                           Cursor cursor, WindowHighlightDescriptor* highlight_descriptor) {
  Cursor tmp = cursor;

  TSQueryMatch query_match;
  while (TSQueryCursorNextMatchWithPredicates(&tmp, query, qcursor, &query_match, regex_map)) {
    for (int index = 0; index < query_match.capture_count; index++) {
      TSNode node = query_match.captures[index].node;
      // fprintf(stderr, "Node (%d, %d) -> (%d, %d) : (index: %d) \n", ts_node_start_point(node).row, ts_node_start_point(node).column, ts_node_end_point(node).row,

      uint32_t size = 0;
      const char* result = ts_query_capture_name_for_id(query, query_match.captures[index].index, &size);
      // fprintf(stderr, " -> capture name : %s\n", result);

      uint16_t priority = query_match.captures[index].index;
      // If a capture.
      saveCaptureToHighlightDescriptor(theme_list, tmp, node, result, highlight_descriptor, priority);
    }
  }
}


void highlightCurrentFile(FileHighlightDatas* highlight_data, WINDOW* ftw, int screen_x, int screen_y, Cursor cursor,
                          WindowHighlightDescriptor* highlight_descriptor) {
  if (highlight_data->is_active == false) {
    return;
  }

  clock_t t;
  t = clock();

  // setup parser to iterate on captures
  ParserContainer* parser = getParserForLanguage(&parsers, highlight_data->lang_id);
  assert(parser != NULL);

  int width = getmaxx(ftw);
  int height = getmaxy(ftw);

  // Setup cursor range
  TSPoint begin;
  begin.row = screen_y - 1;
  begin.column = screen_x;
  TSPoint end;
  end.row = screen_y + height - 2;
  end.column = screen_x + width;
  ts_query_cursor_set_point_range(
    parser->cursor,
    begin,
    end
  );

  ts_query_cursor_exec(parser->cursor, parser->queries, ts_tree_root_node(highlight_data->tree));

  // execute the query search.
  executeHighlightQuery(parser->queries, parser->cursor, &parser->regex_map, parser->theme_list, cursor, highlight_descriptor);

  t = clock() - t;
  double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds

  fprintf(stderr, "highlight() took %f seconds to execute part of check for highlight. \n", time_taken);
}
