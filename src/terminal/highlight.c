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
        int new_size = self->capacity * sizeof(TextPartHighlightDescriptor);
        TextPartHighlightDescriptor* ptr = self->descriptors;
        self->descriptors = realloc(ptr, new_size);
        assert(self->descriptors != NULL);
      }

      if (self->size - i - 1 > 0) {
        self->descriptors = memmove(self->descriptors + i + 1, self->descriptors + i,
                                    sizeof(TextPartHighlightDescriptor) * self->size - i - 1);
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
        self->descriptors = memmove(self->descriptors + i + increase_size, self->descriptors + i,
                                    sizeof(TextPartHighlightDescriptor) * self->size - i - increase_size);

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

void highlightLinePartWithBytes(WINDOW* ftw, int start_row_byte, int start_column_byte, int length_byte, attr_t attr,
                                NCURSES_PAIRS_T color, Cursor cursor, Cursor select,
                                Cursor* tmp, int screen_y,
                                int screen_x) {
  // Convert the byte cursor to Cursor.
  Cursor converted_cur = byteCursorToCursor(*tmp, start_row_byte, start_column_byte);
  int start_row = converted_cur.file_id.absolute_row;
  int start_column = converted_cur.line_id.absolute_column;


  // Support wide char in the word.
  *tmp = tryToReachAbsPosition(*tmp, start_row, start_column);
  for (int i = 0; i < length_byte && hasElementAfterLine(tmp->line_id) == true; i++) {
    // Move the cursor to the current char printed.
    *tmp = tryToReachAbsPosition(*tmp, start_row, start_column + i + 1);
    if (tmp->line_id.absolute_column == 0) continue;

    Char_U8 current_ch = getCharAtCursor(*tmp);
    int size = charPrintSize(current_ch);

    attr_t current_char_attr = attr;
    NCURSES_PAIRS_T current_char_color = color;
    if (areCursorEqual(moveLeft(*tmp), cursor)) {
      current_char_attr |= A_STANDOUT;
    }
    else if (isCursorDisabled(select) == false && isCursorBetweenOthers(*tmp, cursor, select)) {
      current_char_color += 1000;
    }

    int mov_res = wmove(ftw, start_row - screen_y,
                        getScreenXForCursor(*tmp, screen_x) - size/*TODO optimize, do not use getScreenXForCursor*/);
    if (mov_res != ERR) {
      wchgat(ftw, size, current_char_attr, current_char_color, NULL);
    }

    length_byte += 1 - sizeChar_U8(current_ch);
  }
}


void highlightLinePart(WINDOW* ftw, int start_row, int start_column, int length, attr_t attr, NCURSES_PAIRS_T color,
                       Cursor cursor, Cursor select, Cursor* tmp, int screen_y,
                       int screen_x) {
  // Support wide char in the word.
  *tmp = tryToReachAbsPosition(*tmp, start_row, start_column);
  for (int i = 0; i < length && hasElementAfterLine(tmp->line_id) == true; i++) {
    // Move the cursor to the current char printed.
    *tmp = tryToReachAbsPosition(*tmp, start_row, start_column + i + 1);
    if (tmp->line_id.absolute_column == 0) continue;

    Char_U8 current_ch = getCharAtCursor(*tmp);
    int size = charPrintSize(current_ch);

    attr_t current_char_attr = attr;
    NCURSES_PAIRS_T current_char_color = color;
    if (areCursorEqual(moveLeft(*tmp), cursor)) {
      current_char_attr |= A_STANDOUT;
    }
    else if (isCursorDisabled(select) == false && isCursorBetweenOthers(*tmp, cursor, select)) {
      current_char_color += 1000;
    }

    int mov_res = wmove(ftw, start_row - screen_y,
                        getScreenXForCursor(*tmp, screen_x) - size/*TODO optimize, do not use getScreenXForCursor*/);
    if (mov_res != ERR) {
      wchgat(ftw, size, current_char_attr, current_char_color, NULL);
    }
  }
}


double sum_check_match_for_highlight;

void printHighlightedChar(HighlightThemeList theme_list, WINDOW* ftw, int screen_x, int screen_y, Cursor cursor, Cursor select,
                          Cursor tmp, TSNode node, const char* result, WindowHighlightDescriptor* highlight_descriptor,
                          uint16_t priority) {
  if (result == NULL) {
    // fprintf(stderr, "No color found for %s node.\n", tree_path[tree_path_length-1].name);
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
    // fprintf(stderr, "      || No theme found for capture %s  || \n", result);
    return;
  }

#ifndef USE_COLOR
  // quit if color are disabled.
  continue;
#endif

  TSPoint start_point = ts_node_start_point(node);
  TSPoint end_point = ts_node_end_point(node);

  Cursor begin_cursor = byteCursorToCursor(tmp, start_point.row, start_point.column);
  Cursor end_cursor = byteCursorToCursor(tmp, end_point.row, end_point.column);
  whd_insertDescriptor(highlight_descriptor, begin_cursor, end_cursor, color, attr, priority, true);


  // Simple line node.
  if (start_point.row == end_point.row) {
    int length_byte = end_point.column - start_point.column;
    highlightLinePartWithBytes(ftw, start_point.row, start_point.column, length_byte, attr, color, cursor, select, &tmp, screen_y,
                               screen_x);
  }
  else // Mutiple line node.
  {
    // First line
    highlightLinePartWithBytes(ftw, start_point.row, start_point.column, INT_MAX, attr, color, cursor, select, &tmp, screen_y,
                               screen_x);

    // Between lines.
    int ligne_between = end_point.row - start_point.row - 1;
    for (int i = 1; i < ligne_between + 1; i++) {
      highlightLinePartWithBytes(ftw, start_point.row + i, 0, INT_MAX, attr, color, cursor, select, &tmp, screen_y, screen_x);
    }

    // End line
    highlightLinePartWithBytes(ftw, end_point.row, 0, end_point.column, attr, color, cursor, select, &tmp, screen_y, screen_x);
  }

  // End if group found.
}

void captureHighlight(TSQuery* query, TSQueryCursor* qcursor, FileHighlightDatas* highlight_data,
                      HighlightThemeList theme_list, WINDOW* ftw, int screen_x,
                      int screen_y, Cursor cursor, Cursor select) {
  Cursor tmp = cursor;
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
    qcursor,
    begin,
    end
  );

  ts_query_cursor_exec(qcursor, query, ts_tree_root_node(highlight_data->tree));

  ParserContainer* parser = getParserForLanguage(&parsers, highlight_data->lang_id);
  assert(parser != NULL);
  RegexMap* regex_map = &parser->regex_map;

  WindowHighlightDescriptor highlight_descriptor;
  whd_init(&highlight_descriptor);


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
      printHighlightedChar(theme_list, ftw, screen_x, screen_y, cursor, select, tmp, node, result, &highlight_descriptor,
                           priority);
    }
  }

  fprintf(stderr, "================ HIGHLIGHT DESCRIPTOR END ================\n\n");
  fprintf(stderr, "======= BEGIN =======\n");
  whd_print(&highlight_descriptor);
  fprintf(stderr, "======= END =======\n");
}


void highlightCurrentFile(FileHighlightDatas* highlight_data, WINDOW* ftw, int screen_x, int screen_y, Cursor cursor,
                          Cursor select_cursor) {
  assert(highlight_data->is_active == true);
  ParserContainer* parser = getParserForLanguage(&parsers, highlight_data->lang_id);
  assert(parser != NULL);

  clock_t t;
  t = clock();

  // sum_check_match_for_highlight = 0;

  captureHighlight(parser->queries, parser->cursor, highlight_data, parser->theme_list,
                   ftw, screen_x, screen_y, cursor, select_cursor);

  t = clock() - t;
  double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds

  fprintf(stderr, "highlight() took %f seconds to execute part of check for highlight : %f \n", time_taken,
          sum_check_match_for_highlight);
}
