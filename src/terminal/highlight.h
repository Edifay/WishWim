#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include "../advanced/theme.h"
#include "../advanced/tree-sitter/tree_manager.h"
#include "../data-management/file_structure.h"
#include "../../lib/tree-sitter/lib/include/tree_sitter/api.h"


#define USE_COLOR true


////// ---------------- COLOR FUNCTIONS ---------------

void initColorsForTheme(HighlightThemeList theme_list, int* color_index, int* color_pair);


void highlightFilePart(WINDOW* ftw, int start_row, int start_column, int length, attr_t attr, NCURSES_PAIRS_T color, Cursor cursor, Cursor select, Cursor* tmp, int screen_y,
                       int screen_x);

void checkMatchForHighlight(TSNode node, TreePath tree_path[], int tree_path_length, long* args);

#endif //HIGHLIGHT_H
