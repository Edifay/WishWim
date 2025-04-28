#include "highlight.h"

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <time.h>

#include "term_handler.h"
#include "../advanced/tree-sitter/tree_query.h"
#include "../data-management/file_structure.h"
#include "../utils/constants.h"


////// ---------------- COLOR FUNCTIONS ---------------


void initColorsForTheme(HighlightThemeList theme_list, int* color_index, int* color_pair) {
  // Setup color sheme.
  for (int i = 0; i < theme_list.size; i++) {
    init_color((*color_index)++, theme_list.groups[i].color.r, theme_list.groups[i].color.g, theme_list.groups[i].color.b);
    init_pair(*color_pair, *color_index - 1, COLOR_BLACK);
    theme_list.groups[i].color_n = *color_pair;

    init_color((*color_index)++, theme_list.groups[i].color_hover.r, theme_list.groups[i].color_hover.g, theme_list.groups[i].color_hover.b);
    init_pair(*color_pair + 1000, *color_index - 1, COLOR_HOVER);
    theme_list.groups[i].color_hover_n = *color_pair + 1000;
    (*color_pair)++;
  }
}

void highlightLinePartWithBytes(WINDOW* ftw, int start_row_byte, int start_column_byte, int length_byte, attr_t attr, NCURSES_PAIRS_T color, Cursor cursor, Cursor select,
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

    int mov_res = wmove(ftw, start_row - screen_y, getScreenXForCursor(*tmp, screen_x) - size/*TODO optimize, do not use getScreenXForCursor*/);
    if (mov_res != ERR) {
      wchgat(ftw, size, current_char_attr, current_char_color, NULL);
    }

    length_byte += 1 - sizeChar_U8(current_ch);
  }
}


void highlightLinePart(WINDOW* ftw, int start_row, int start_column, int length, attr_t attr, NCURSES_PAIRS_T color, Cursor cursor, Cursor select, Cursor* tmp, int screen_y,
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

    int mov_res = wmove(ftw, start_row - screen_y, getScreenXForCursor(*tmp, screen_x) - size/*TODO optimize, do not use getScreenXForCursor*/);
    if (mov_res != ERR) {
      wchgat(ftw, size, current_char_attr, current_char_color, NULL);
    }
  }
}


char litteral_text_node_buffer[REGEX_MAX_DUMP_SIZE * 4 + 1];
double sum_check_match_for_highlight;

void printHighlightedChar(HighlightThemeList theme_list, WINDOW* ftw, int screen_x, int screen_y, Cursor cursor, Cursor select, Cursor tmp, TSNode node, const char* result) {
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


  // TODO rework error system. It's not working properly.
  // for (int i = 0; i < tree_path_length; i++) {
  //   if (tree_path[i].type == SYMBOL && strcmp("ERROR", tree_path[i].name) == 0) {
  //     attr |= A_ITALIC;
  //   }
  // }

#ifndef USE_COLOR
        // quit if color are disabled.
        continue;
#endif

  TSPoint start_point = ts_node_start_point(node);
  TSPoint end_point = ts_node_end_point(node);
  // Simple line node.
  if (start_point.row == end_point.row) {
    int length_byte = end_point.column - start_point.column;
    highlightLinePartWithBytes(ftw, start_point.row, start_point.column, length_byte, attr, color, cursor, select, &tmp, screen_y, screen_x);
  }
  else // Mutiple line node.
  {
    // First line
    highlightLinePartWithBytes(ftw, start_point.row, start_point.column, INT_MAX, attr, color, cursor, select, &tmp, screen_y, screen_x);

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
  RegexMap regex_map = parser->regex_map;


  TSQueryMatch query_match;
  while (TSQueryCursorNextMatchWithPredicates(&tmp, query, qcursor, &query_match, regex_map)) {
    for (int index = 0; index < query_match.capture_count; index++) {
      TSNode node = query_match.captures[index].node;
      // fprintf(stderr, "Node (%d, %d) -> (%d, %d) : (index: %d) \n", ts_node_start_point(node).row, ts_node_start_point(node).column, ts_node_end_point(node).row,
      // ts_node_end_point(node).column, index);


      uint32_t size = 0;
      const char* result = ts_query_capture_name_for_id(query, query_match.captures[index].index, &size);
      // fprintf(stderr, " -> capture name : %s\n", result);

      // If a capture.
      printHighlightedChar(theme_list, ftw, screen_x, screen_y, cursor, select, tmp, node, result);
    }
  }
}


void highlightCurrentFile(FileHighlightDatas* highlight_data, WINDOW* ftw, int screen_x, int screen_y, Cursor cursor, Cursor select_cursor) {
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

  fprintf(stderr, "highlight() took %f seconds to execute part of check for highlight : %f \n", time_taken, sum_check_match_for_highlight);
}
