#include "highlight.h"

#include <limits.h>
#include <string.h>

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

// TODO patch highlight wrong with wide char BEFORE the word.
void highlightFilePart(WINDOW* ftw, int start_row, int start_column, int length, attr_t attr, NCURSES_PAIRS_T color, Cursor cursor, Cursor select, Cursor* tmp, int screen_y,
                       int screen_x) {
  // Support wide char in the word.
  int offset = 0;
  *tmp = tryToReachAbsPosition(*tmp, start_row + 1, start_column);
  for (int i = 0; i < length && hasElementAfterLine(tmp->line_id) == true; i++) {
    // Move the cursor to the current char printed.
    *tmp = tryToReachAbsPosition(*tmp, start_row + 1, start_column + i + 1);
    if (tmp->line_id.absolute_column == 0) continue;
    int size = charPrintSize(getCharAtCursor(*tmp));

    attr_t current_char_attr = attr;
    NCURSES_PAIRS_T current_char_color = color;
    if (areCursorEqual(moveLeft(*tmp), cursor)) {
      current_char_attr |= A_STANDOUT;
    }
    else if (isCursorDisabled(select) == false && isCursorBetweenOthers(*tmp, cursor, select)) {
      current_char_color += 1000;
    }
    int mov_res = wmove(ftw, start_row - screen_y + 1, start_column - screen_x + 1 + offset);
    if (mov_res != ERR) {
      wchgat(ftw, size, current_char_attr, current_char_color, NULL);
    }
    offset += size;
  }
}


void checkMatchForHighlight(TSNode node, TreePath tree_path[], int tree_path_length, long* args) {
  // Un-abstracting args.
  TreePathSeq* seq = ((TreePathSeq *)args[0]);
  HighlightThemeList* theme_list = ((HighlightThemeList *)args[1]);
  char* source = (char *)args[2];
  WINDOW* ftw = (WINDOW *)args[3];
  int* screen_x = (int *)args[4];
  int* screen_y = (int *)args[5];
  int width = (int)args[6];
  int height = (int)args[7];
  Cursor cursor = *((Cursor *)args[8]);
  Cursor select = *((Cursor *)args[9]);
  Cursor* tmp = (Cursor *)args[10];
  if (tmp == NULL) {
    tmp = malloc(sizeof(Cursor));
    args[10] = (long)tmp;
    *tmp = cursor;
  }


  // Get litteral string for current node. We don't extract if the litteral is higher than 200 char.
  int char_nb = ts_node_end_byte(node) - ts_node_start_byte(node);
  int array_size = char_nb;
  if (char_nb > 200) {
    array_size = 0;
  }
  char litteral_text_node[array_size + 1];
  if (char_nb <= 200) {
    strncpy(litteral_text_node, source + ts_node_start_byte(node), char_nb);
  }
  litteral_text_node[array_size] = '\0';

  while (seq != NULL) {
    char* result = isTreePathMatchingQuery(litteral_text_node, tree_path, tree_path_length, seq->value);

    // If a group was found.
    if (result != NULL) {
      attr_t attr = A_NORMAL;
      NCURSES_PAIRS_T color;

      bool found = false;
      // Setup style for group found.
      for (int i = 0; i < theme_list->size; i++) {
        if (strcmp(theme_list->groups[i].group, result) == 0) {
          attr = getAttrForTheme(theme_list->groups[i]);
          color = theme_list->groups[i].color_n;

          found = true;
          break;
        }
      }

      if (found == false) {
        // Quit if no group was found.
        break;
      }

      TSPoint start_point = ts_node_start_point(node);
      TSPoint end_point = ts_node_end_point(node);


      if (!USE_COLOR) {
        // quit if color are disabled.
        break;
      }

      // Simple line node.
      if (start_point.row == end_point.row) {
        int length = end_point.column - start_point.column;
        highlightFilePart(ftw, start_point.row, start_point.column, length, attr, color, cursor, select, tmp, *screen_y, *screen_x);
      }
      else // Mutiple line node.
      {
        // First line
        highlightFilePart(ftw, start_point.row, start_point.column, INT_MAX, attr, color, cursor, select, tmp, *screen_y, *screen_x);

        // Between lines.
        int ligne_between = end_point.row - start_point.row - 1;
        for (int i = 1; i < ligne_between + 1; i++) {
          highlightFilePart(ftw, start_point.row + i, 0, INT_MAX, attr, color, cursor, select, tmp, *screen_y, *screen_x);
        }

        // End line
        highlightFilePart(ftw, end_point.row, 0, end_point.column, attr, color, cursor, select, tmp, *screen_y, *screen_x);
      }

      break;
      // End if group found.
    }

    seq = seq->next;
  }
}
