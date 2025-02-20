#include "highlight.h"

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <time.h>

#include "term_handler.h"
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

void checkMatchForHighlight(TSNode node, TreePath tree_path[], int tree_path_length, long* args) {
  clock_t t;
  t = clock();
  // Un-abstracting args.
  TreePathSeq* seq = ((TreePathSeq *)args[0]);
  HighlightThemeList* theme_list = ((HighlightThemeList *)args[1]);
  WINDOW* ftw = (WINDOW *)args[3];
  int* screen_x = (int *)args[4];
  int* screen_y = (int *)args[5];
  Cursor cursor = *((Cursor *)args[8]);
  Cursor select = *((Cursor *)args[9]);
  Cursor* tmp = (Cursor *)args[10];
  if (tmp == NULL) {
    tmp = malloc(sizeof(Cursor));
    args[10] = (long)tmp;
    *tmp = cursor;
  }


  // Get litteral string for current node. We don't extract if the litteral is higher than 200 char.
#if REGEX_MAX_DUMP_SIZE != 0
  int char_nb = ts_node_end_byte(node) - ts_node_start_byte(node);
  int array_size = char_nb;
  if (char_nb > REGEX_MAX_DUMP_SIZE) {
    array_size = 0;
  }
  if (char_nb <= REGEX_MAX_DUMP_SIZE) {
    // fprintf(stderr, "READ FROM HIGHLIGHT\n");
    readNBytesAtPosition(tmp, ts_node_start_point(node).row, ts_node_start_point(node).column, litteral_text_node_buffer, char_nb);
  }
  litteral_text_node_buffer[array_size] = '\0';
#else
  char litteral_text_node[1];
  litteral_text_node[0] = '\0';
#endif


  while (seq != NULL) {
    char* result = isTreePathMatchingQuery(litteral_text_node_buffer, tree_path, tree_path_length, seq->value);

    attr_t attr = A_NORMAL;
    NCURSES_PAIRS_T color = DEFAULT_COLOR_PAIR;

    // If a group was found.
    if (result != NULL) {
      bool found = false;
      // Setup style for group found.
      for (int i = 0; i < theme_list->size; i++) {
        if (strcmp(theme_list->groups[i].group, result) == 0) {
          attr |= getAttrForTheme(theme_list->groups[i]);
          color = theme_list->groups[i].color_n;

          found = true;
          break;
        }
      }

      if (found == false) {
        // Quit if a query was found, but not theme for this group was found.
        break;
      }


      // TODO rework error system. It's not working properly.
      for (int i = 0; i < tree_path_length; i++) {
        if (tree_path[i].type == SYMBOL && strcmp("ERROR", tree_path[i].name) == 0) {
          attr |= A_ITALIC;
        }
      }

      TSPoint start_point = ts_node_start_point(node);
      TSPoint end_point = ts_node_end_point(node);


      if (!USE_COLOR) {
        // quit if color are disabled.
        break;
      }

      // Simple line node.
      if (start_point.row == end_point.row) {
        int length_byte = end_point.column - start_point.column;
        highlightLinePartWithBytes(ftw, start_point.row, start_point.column, length_byte, attr, color, cursor, select, tmp, *screen_y, *screen_x);
      }
      else // Mutiple line node.
      {
        // First line
        highlightLinePartWithBytes(ftw, start_point.row, start_point.column, INT_MAX, attr, color, cursor, select, tmp, *screen_y, *screen_x);

        // Between lines.
        int ligne_between = end_point.row - start_point.row - 1;
        for (int i = 1; i < ligne_between + 1; i++) {
          highlightLinePartWithBytes(ftw, start_point.row + i, 0, INT_MAX, attr, color, cursor, select, tmp, *screen_y, *screen_x);
        }

        // End line
        highlightLinePartWithBytes(ftw, end_point.row, 0, end_point.column, attr, color, cursor, select, tmp, *screen_y, *screen_x);
      }

      break;
      // End if group found.
    }else {
      // fprintf(stderr, "No color found for %s node.\n", tree_path[tree_path_length-1].name);

    }

    seq = seq->next;
  }
  t = clock() - t;
  double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds
  sum_check_match_for_highlight += time_taken;
  // fprintf(stderr, "checkMatchForHighlight() took %f seconds to execute %d \n", time_taken, tree_path_length);
  // for (int i = 0 ; i < tree_path_length ; i++) {
  //   fprintf(stderr, "%s.", tree_path[i].name);
  // }
  // fprintf(stderr,"\n");
}


void highlightCurrentFile(FileHighlightDatas* highlight_data, WINDOW* ftw, int* screen_x, int* screen_y, Cursor* cursor, Cursor* select_cursor) {
  ParserContainer* parser = getParserForLanguage(&parsers, highlight_data->lang_name);
  assert(parser != NULL);

  long* args_fct = malloc(11 * sizeof(long *));
  args_fct[0] = (long)&parser->highlight_queries;
  args_fct[1] = (long)&parser->theme_list;
  args_fct[2] = (long)highlight_data->tmp_file_dump;
  args_fct[3] = (long)ftw;
  args_fct[4] = (long)screen_x;
  args_fct[5] = (long)screen_y;
  args_fct[6] = (long)getmaxx(ftw);
  args_fct[7] = (long)getmaxy(ftw);
  args_fct[8] = (long)cursor;
  args_fct[9] = (long)select_cursor;
  args_fct[10] = (long)NULL;

  TSNode root_node = ts_tree_root_node(highlight_data->tree);
  TreePath path[1000]; // TODO refactor this, there is a problem if the depth of the tree is bigger than 1000.

  clock_t t;
  t = clock();

  sum_check_match_for_highlight = 0;
  treeForEachNodeSized(*screen_y, *screen_x, getmaxy(ftw), getmaxx(ftw), root_node, path, 0, checkMatchForHighlight, args_fct);

  t = clock() - t;
  double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds

  // fprintf(stderr, "highlight() took %f seconds to execute part of check for highlight : %f \n", time_taken, sum_check_match_for_highlight);

  free((Cursor *)args_fct[10]);
  free(args_fct);
}
