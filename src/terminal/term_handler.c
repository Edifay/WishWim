#include <assert.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>

#include "../data-management/file_management.h"
#include "term_handler.h"

#include "highlight.h"
#include "../utils/constants.h"


////// -------------- WINDOWS MANAGEMENTS --------------


void initGUIContext(GUIContext* gui_context) {
  // Init GUI vars
  gui_context->ftw = NULL; // File Text Window
  gui_context->lnw = NULL; // Line Number Window
  gui_context->ofw = NULL; // Opened Files Window
  gui_context->few = NULL; // File Explorer Window
  gui_context->refresh_edw = true; // Need to reprint editor window
  gui_context->refresh_ofw = true; // Need to reprint opened file window
  gui_context->refresh_few = true; // Need to reprint file explorer window
  gui_context->focus_w = NULL; // Used to set the window where start mouse drag

  // EDW Datas

  // OFW Datas
  gui_context->current_file_offset = 0;
  // Height of Opened Files Window. 0 => Disabled on start.   OPENED_FILE_WINDOW_HEIGHT => Enabled on start.
  gui_context->ofw_height = 0;

  // Few Datas
  gui_context->few_width = 0; // File explorer width
  gui_context->saved_few_width = FILE_EXPLORER_WIDTH;
  gui_context->few_x_offset = 0; /* TODO unused */
  gui_context->few_y_offset = 0; // Y Scroll state of File Explorer Window
  gui_context->few_selected_line = -1;
}

void initNCurses(GUIContext* gui_context) {
  // Init ncurses
  initscr();
  gui_context->ftw = newwin(0, 0, gui_context->ofw_height, gui_context->few_width);
  gui_context->lnw = newwin(0, 0, 0, gui_context->few_width);
  gui_context->ofw = newwin(gui_context->ofw_height, 0, 0, gui_context->few_width);
  // Keyboard setup
  raw();
  keypad(stdscr, TRUE);
  noecho();
  curs_set(0);
  // Mouse setup
  mouseinterval(0);
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  timeout(100);
  printf("\033[?1003h"); // enable mouse tracking
  fflush(stdout);
  // Color setup
  start_color();
  // Default color.
  init_color(COLOR_HOVER, 390, 390, 390);
  init_pair(DEFAULT_COLOR_PAIR, COLOR_WHITE, COLOR_BLACK);
  init_pair(DEFAULT_COLOR_HOVER_PAIR, COLOR_WHITE, COLOR_HOVER);
  init_pair(ERROR_COLOR_PAIR, COLOR_RED, COLOR_BLACK);
  init_pair(ERROR_COLOR_HOVER_PAIR, COLOR_RED, COLOR_HOVER);
}


////// -------------- PRINT FUNCTIONS --------------


void printChar_U8ToNcurses(WINDOW* w, Char_U8 ch) {
  int size = sizeChar_U8(ch);
  for (int i = 0; i < size; i++) {
    wprintw(w, "%c", ch.t[i]);
  }
}

void printEditor_printLineNumber(GUIContext* gui_context, Cursor cursor, int screen_y, FileIdentifier file_cur, int row) {
  char line_number[40];
  sprintf(line_number, "%d", file_cur.absolute_row);
  int lineNumberSize = strlen(line_number);

  if (file_cur.absolute_row != cursor.file_id.absolute_row) {
    wattron(gui_context->lnw, A_DIM);
  }
  else {
    wattron(gui_context->lnw, A_BOLD);
  }

  wmove(gui_context->lnw, row - screen_y, 0);
  for (int i = 0; i < getmaxx(gui_context->lnw) - lineNumberSize - 1; i++) {
    wprintw(gui_context->lnw, " ");
  }
  wprintw(gui_context->lnw, "%s", line_number);
  if (file_cur.absolute_row == cursor.file_id.absolute_row) {
    wprintw(gui_context->lnw, "🭳");
  }
  else {
    wprintw(gui_context->lnw, "🭵");
  }

  if (file_cur.absolute_row != cursor.file_id.absolute_row) {
    wattroff(gui_context->lnw, A_DIM);
  }
  else {
    wattroff(gui_context->lnw, A_BOLD);
  }
}

void printEditor_printFileContent(GUIContext* gui_context, Cursor cursor, Cursor select_cursor, int screen_x,
                                  WindowHighlightDescriptor* highlight_descriptor, FileIdentifier file_cur,
                                  const int column_count, int* whd_offset) {
  LineIdentifier begin_screen_line_cur =
      tryToReachAbsColumn(moduloLineIdentifierR(getLineForFileIdentifier(file_cur), 0), screen_x);
  LineIdentifier end_screen_line_cur =
      tryToReachAbsColumn(begin_screen_line_cur, screen_x + column_count - 3);

  int column = screen_x;
  while (end_screen_line_cur.absolute_column >= screen_x
         && begin_screen_line_cur.absolute_column <= end_screen_line_cur.absolute_column
         && screen_x + column_count - 3 >= column) {
    Char_U8 ch = getCharForLineIdentifier(begin_screen_line_cur);
    Cursor ch_cursor = cursorOf(file_cur, begin_screen_line_cur);

    int size = charPrintSize(ch);
    // If the char is detected as not printable char.
    if (size <= 0) {
      ch = readChar_U8FromCharArray("�");
      size = 1;
    }

    // If a char of size 2 is at the end of the line replace it by '_' to avoid line overflow.
    if (size >= 2 && screen_x + column_count - 4 < column) {
      ch = readChar_U8FromCharArray("_");
      // size = 1;
    }

    // determine if the char is selected or not.
    bool selected_style = isCursorDisabled(select_cursor) == false && isCursorBetweenOthers(ch_cursor, select_cursor, cursor);

    // get current highlight.
    TextPartHighlightDescriptor* current_highlight = whd_tphd_forCursorWithOffsetIndex(
      highlight_descriptor, ch_cursor, whd_offset);


    // default style
    attr_t attr = A_NORMAL;
    NCURSES_PAIRS_T color = DEFAULT_COLOR_PAIR;

    // override default style if a textPartHighlight was found.
    if (current_highlight != NULL) {
      attr = current_highlight->attributes;
      color = current_highlight->color;
    }

    // add the offset if the current text is selected
    if (selected_style) {
      color += COLOR_HOVER_OFFSET;
    }

    // setting the char attribute.
    wattr_set(gui_context->ftw, attr, color, 0);

    if (ch.t[0] == '\t') {
      Char_U8 space = readChar_U8FromInput(' ');
      for (int i = 0; i < TAB_SIZE; i++) {
        printChar_U8ToNcurses(gui_context->ftw, space);
      }
    }
    else {
      printChar_U8ToNcurses(gui_context->ftw, ch);
    }

    // move to next column
    begin_screen_line_cur.relative_column++;
    begin_screen_line_cur.absolute_column++;
    column += size;
  }

  // show empty line selected.
  if (begin_screen_line_cur.absolute_column == end_screen_line_cur.absolute_column && hasElementAfterLine(end_screen_line_cur)
      == false) {
    if (isCursorDisabled(select_cursor) == false
        && isCursorBetweenOthers(cursorOf(file_cur, begin_screen_line_cur), select_cursor, cursor)) {
      // if line selected
      wattr_set(gui_context->ftw, A_NORMAL, DEFAULT_COLOR_HOVER_PAIR, 0);
      wprintw(gui_context->ftw, " ");
      wattr_set(gui_context->ftw, A_NORMAL, 0, NULL);
    }
  }

  // If the line is not fully display show '>'
  if (hasElementAfterLine(end_screen_line_cur)) {
    wattr_set(gui_context->ftw, A_BOLD | A_UNDERLINE | A_DIM, DEFAULT_COLOR_PAIR, 0);
    wprintw(gui_context->ftw, ">");
  }

  wprintw(gui_context->ftw, "\n");
}

void printEditor_printCursor(GUIContext* gui_context, Cursor cursor, int screen_x, int screen_y,
                             WindowHighlightDescriptor* highlight_descriptor, const int line_count, const int column_count) {
  // Check if cursor is in the screen and print it if needed.
  if (cursor.file_id.absolute_row >= screen_y && cursor.file_id.absolute_row < screen_y + line_count
      && cursor.line_id.absolute_column >= screen_x - 1 && cursor.line_id.absolute_column <= screen_x + column_count - 3) {
    int x = getScreenXForCursor(cursor, screen_x);
    wmove(gui_context->ftw, cursor.file_id.absolute_row - screen_y, x);

    TextPartHighlightDescriptor* current_highlight = NULL;

    char size = 1;
    if (hasElementAfterLine(cursor.line_id) == true) {
      // Check the size of the the char which is under cursor.
      Cursor tmp = moveRight(cursor);
      size = charPrintSize(getCharForLineIdentifier(tmp.line_id));
      int unused = 0;
      current_highlight = whd_tphd_forCursorWithOffsetIndex(highlight_descriptor, tmp, &unused);
    }
    else {
      int unused = 0;
      current_highlight = whd_tphd_forCursorWithOffsetIndex(highlight_descriptor, cursor, &unused);
    }


    attr_t attr = A_STANDOUT;
    NCURSES_PAIRS_T color = DEFAULT_COLOR_PAIR;

    if (current_highlight != NULL) {
      attr |= current_highlight->attributes;
      color = current_highlight->color;
    }

    wchgat(gui_context->ftw, size, attr, color, NULL);
  }
}

void printEditor(GUIContext* gui_context, Cursor cursor, Cursor select_cursor, int screen_x, int screen_y,
                 WindowHighlightDescriptor* highlight_descriptor) {
  wmove(gui_context->ftw, 0, 0);
  FileIdentifier file_cur = cursor.file_id;

  const int line_count = getmaxy(gui_context->ftw);
  const int column_count = getmaxx(gui_context->ftw);


  // ===============  FOR EACH LINE  ===============
  int whd_offset = 0;
  for (int row = screen_y; row < screen_y + line_count; row++) {
    // getting the row to print.
    file_cur = tryToReachAbsRow(file_cur, row);

    // if the row is couldn't be reached skip it.
    if (file_cur.absolute_row != row) {
      // show empty line number.
      wmove(gui_context->lnw, row - screen_y, 0);
      for (int i = 0; i < getmaxx(gui_context->lnw); i++) {
        wprintw(gui_context->lnw, " ");
      }

      // show ~ as content in file text window
      wattr_set(gui_context->ftw, A_NORMAL, DEFAULT_COLOR_PAIR, NULL);
      wprintw(gui_context->ftw, "~\n");

      continue;
    }

    // ===============  Print line number  ===============
    printEditor_printLineNumber(gui_context, cursor, screen_y, file_cur, row);


    // ===============  Print File Content  ===============

    printEditor_printFileContent(gui_context, cursor, select_cursor, screen_x, highlight_descriptor, file_cur, column_count,
                                 &whd_offset);
  }

  // ===============  Print Cursor  ===============
  printEditor_printCursor(gui_context, cursor, screen_x, screen_y, highlight_descriptor, line_count, column_count);
  // box(ofw, 0, 0);
}

void printOpenedFile(GUIContext* gui_context, FileContainer* files, int file_count, int current_file) {
  // The current position of the cursor for the first line.
  wmove(gui_context->ofw, 0, 0);
  int current_offset = getbegx(gui_context->ofw);
  if (gui_context->current_file_offset != 0) {
    current_offset += strlen("< | ");
    wattron(gui_context->ofw, A_DIM);
    wprintw(gui_context->ofw, "< | ");
    wattroff(gui_context->ofw, A_DIM);
  }
  // Move to the top left corner.
  for (int i = gui_context->current_file_offset; i < file_count; i++) {
    // Style file names.
    if (i == current_file)
      wattron(gui_context->ofw, A_BOLD);
    else
      wattron(gui_context->ofw, A_DIM);
    // Print file name
    char* file_name = basename(files[i].io_file.path_args);
    current_offset += strlen(file_name);
    wprintw(gui_context->ofw, "%s", file_name);
    // Style file names.
    if (i == current_file)
      wattroff(gui_context->ofw, A_BOLD);
    else
      wattroff(gui_context->ofw, A_DIM);
    // Print file name separator
    if (i < file_count - 1) {
      wprintw(gui_context->ofw, FILE_NAME_SEPARATOR);
      current_offset += strlen(FILE_NAME_SEPARATOR);
    }
    // If the file names overflow the line, print the move right text.
    if (current_offset + strlen(FILE_NAME_SEPARATOR) > COLS) {
      wmove(gui_context->ofw, 0, COLS - getbegx(gui_context->ofw) - strlen("... | >"));
      wattron(gui_context->ofw, A_DIM);
      wprintw(gui_context->ofw, "... | >");
      wattroff(gui_context->ofw, A_DIM);
      // assert((i < argc && i < max_opened_file + 1) == false);
      break;
    }
  }
  // To erase the end of the line to avoid garbage on scroll to right.
  for (int i = current_offset; i < COLS; i++) {
    wprintw(gui_context->ofw, " ");
  }
  // Print the bottom line.
  wmove(gui_context->ofw, 1, 0);
  for (int i = 0; i < COLS; i++) {
    wprintw(gui_context->ofw, "🭸");
  }
}


void internalPrintExplorerRec(ExplorerFolder* folder, WINDOW* few, int* few_x_offset, int* few_y_offset, int tree_offset_rec,
                              int* selected_line) {
  // Don't print if not in window.
  if (getcury(few) + 1 >= getmaxy(few)) return;

  if (folder->open && folder->discovered == false) {
    discoverFolder(folder);
  }

  (*selected_line)--;
  if (*few_y_offset == 0) {
    // Print current folder name

    if (*selected_line == 0) {
      wattron(few, A_STANDOUT);
    }

    for (int i = 0; i < tree_offset_rec && i < getmaxx(few); i++) {
      printToNcursesNCharFromString(few, " ", getmaxx(few) - (getcurx(few) + 1));
    }

    // Print decoration of folder. The decoration describe if the folder is open or not.
    if (folder->open) printToNcursesNCharFromString(few, "⌄", getmaxx(few) - (getcurx(few) + 1));
    else printToNcursesNCharFromString(few, "›", getmaxx(few) - (getcurx(few) + 1));

    printToNcursesNCharFromString(few, "📁", getmaxx(few) - (getcurx(few) + 1));
    printToNcursesNCharFromString(few, basename(folder->path), getmaxx(few) - (getcurx(few) + 1));
    if (*selected_line == 0) {
      for (int j = getcurx(few) + 1; j < getmaxx(few); j++) {
        printToNcursesNCharFromString(few, " ", getmaxx(few) - (getcurx(few) + 1));
      }
      wattroff(few, A_STANDOUT);
    }
    wprintw(few, "\n");
  }
  else {
    (*few_y_offset)--;
  }

  if (folder->open == false)
    return;

  // Print sub folders
  for (int i = 0; i < folder->folder_count; i++) {
    internalPrintExplorerRec(folder->folders + i, few, few_x_offset, few_y_offset, tree_offset_rec + FILE_EXPLORER_TREE_OFFSET,
                             selected_line);
  }
  // Print sub files
  for (int i = 0; i < folder->file_count; i++) {
    // Don't print if not in window.
    if (getcury(few) + 1 >= getmaxy(few)) return;

    (*selected_line)--;
    if (*few_y_offset == 0) {
      // Print file name
      if (*selected_line == 0) {
        wattron(few, A_STANDOUT);
      }
      for (int j = 0; j < tree_offset_rec + FILE_EXPLORER_TREE_OFFSET + 1/*Add one to balance with the folder decoration*/; j++) {
        printToNcursesNCharFromString(few, " ", getmaxx(few) - (getcurx(few) + 1));
      }
      printToNcursesNCharFromString(few, "📄", getmaxx(few) - (getcurx(few) + 1));
      printToNcursesNCharFromString(few, basename(folder->files[i].path), getmaxx(few) - (getcurx(few) + 1));
      if (*selected_line == 0) {
        for (int j = getcurx(few) + 1; j < getmaxx(few); j++) {
          printToNcursesNCharFromString(few, " ", getmaxx(few) - (getcurx(few) + 1));
        }
        wattroff(few, A_STANDOUT);
      }
      wprintw(few, "\n");
    }
    else {
      (*few_y_offset)--;
    }
  }
}

void printFileExplorer(GUIContext* gui_context, ExplorerFolder* pwd) {
  wmove(gui_context->few, 0, 0);

  // the internal fct need edit this var to lake them.
  // We need to make a copy of them to keep the value right in the gui_context.
  int tmp_few_x_offset = gui_context->few_x_offset;
  int tmp_few_y_offset = gui_context->few_y_offset;
  int tmp_few_selected_line = gui_context->few_selected_line;
  internalPrintExplorerRec(pwd, gui_context->few, &tmp_few_x_offset, &tmp_few_y_offset, 0, &tmp_few_selected_line);
  // Clear end of window
  for (int i = getcury(gui_context->few) + 1; i < getmaxy(gui_context->few); i++) {
    wprintw(gui_context->few, "\n");
  }
  for (int i = getbegy(gui_context->few); i < getmaxy(gui_context->few); i++) {
    mvwprintw(gui_context->few, i, getmaxx(gui_context->few) - 1, "│");
  }
}

////// -------------- RESIZE FUNCTIONS --------------

void resizeEditorWindows(GUIContext* gui_context, int lnw_new_width) {
  delwin(gui_context->ftw);
  delwin(gui_context->lnw);
  gui_context->ftw = newwin(0, 0, gui_context->ofw_height, lnw_new_width + gui_context->few_width);
  gui_context->lnw = newwin(0, lnw_new_width, gui_context->ofw_height, gui_context->few_width);
}

void resizeOpenedFileWindow(GUIContext* gui_context) {
  delwin(gui_context->ofw);
  gui_context->ofw = newwin(gui_context->ofw_height, 0, 0, gui_context->few_width);
  gui_context->refresh_ofw = true;
}

void switchShowFew(GUIContext* gui_context) {
  if (gui_context->few == NULL) {
    // Open File Explorer Window
    gui_context->few_width = gui_context->saved_few_width;
    gui_context->few = newwin(0, gui_context->few_width, 0, 0);
    gui_context->refresh_few = true;
  }
  else {
    // Close File Explorer Window
    gui_context->saved_few_width = getmaxx(gui_context->few);
    delwin(gui_context->few);
    gui_context->few = NULL;
    gui_context->few_width = 0;
  }
  // Resize Opened File Window
  resizeOpenedFileWindow(gui_context);
  // Resize Editor Window
  resizeEditorWindows(gui_context, getmaxx(gui_context->lnw));
}


////// -------------- UTILS FUNCTIONS --------------


void moveScreenToMatchCursor(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y) {
  int current_lines = getmaxy(w);
  int current_columns = getmaxx(w);

  if (cursor.file_id.absolute_row - (*screen_y + current_lines) + 1 >= 0) {
    *screen_y = cursor.file_id.absolute_row - current_lines + 2;
    if (*screen_y < 1) *screen_y = 1;
  }
  else if (cursor.file_id.absolute_row < *screen_y + 1) {
    *screen_y = cursor.file_id.absolute_row - 1;
    if (*screen_y < 1) *screen_y = 1;
  }

  int screen_x_wide_char = getScreenXForCursor(cursor, *screen_x) + *screen_x;
  if (screen_x_wide_char - (*screen_x + current_columns - 8) >= 0) {
    *screen_x = screen_x_wide_char - current_columns + 8;
    if (*screen_x < 1) *screen_x = 1;
  }
  else if (screen_x_wide_char - 5 < *screen_x) {
    *screen_x = screen_x_wide_char - 5;
    if (*screen_x < 1) {
      *screen_x = 1;
    }
  }
}

void centerCursorOnScreen(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y) {
  // center for y, but right for x.
  *screen_x = cursor.line_id.absolute_column - (COLS /*/ 2*/);
  *screen_y = cursor.file_id.absolute_row - (LINES / 2);

  if (*screen_x < 1)
    *screen_x = 1;
  if (*screen_y < 1)
    *screen_y = 1;

  // To match right for x.
  moveScreenToMatchCursor(w, cursor, screen_x, screen_y);
}


int getScreenXForCursor(Cursor cursor, int screen_x) {
  Cursor initial = cursor;
  Cursor old_cursor = cursor;
  int atAdd = 0;
  int size;


  if (cursor.line_id.absolute_column != 0 && (size = charPrintSize(getCharForLineIdentifier(cursor.line_id))) >= 2) {
    atAdd += size - 1;
  }
  cursor = moveLeft(cursor);


  while (screen_x <= cursor.line_id.absolute_column && areCursorEqual(cursor, old_cursor) == false
         && cursor.file_id.absolute_row == old_cursor.file_id.absolute_row) {
    assert(cursor.line_id.absolute_column != 0);
    Char_U8 current_ch = getCharForLineIdentifier(cursor.line_id);
    if ((size = charPrintSize(current_ch)) >= 2) {
      atAdd += size - 1;
    }

    old_cursor = cursor;
    cursor = moveLeft(cursor);
  }

  return initial.line_id.absolute_column - screen_x + 1 + atAdd;
}

LineIdentifier getLineIdForScreenX(LineIdentifier line_id, int screen_x, int x_click) {
  line_id = tryToReachAbsColumn(line_id, screen_x - 1);

  int current_column = 0;
  int x_el = 0;

  while (hasElementAfterLine(line_id) == true && current_column <= x_click) {
    line_id = tryToReachAbsColumn(line_id, line_id.absolute_column + 1);
    int size = charPrintSize(getCharForLineIdentifier(line_id));
    if (size <= 0) size = 1; // TODO handle non UTF_8 char.
    current_column += size;
    x_el++;
  }

  if (x_click >= current_column)
    x_el++;

  return tryToReachAbsColumn(line_id, screen_x + x_el - 2);
}


void setDesiredColumn(Cursor cursor, int* desired_column) {
  *desired_column = cursor.line_id.absolute_column;
}
