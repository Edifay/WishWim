#include <assert.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>

#include "../data-management/file_management.h"
#include "term_handler.h"
#include "../utils/constants.h"


////// -------------- PRINT FUNCTIONS --------------


void printChar_U8ToNcurses(WINDOW* w, Char_U8 ch) {
  int size = sizeChar_U8(ch);
  for (int i = 0; i < size; i++) {
    wprintw(w, "%c", ch.t[i]);
  }
}

void printEditor(WINDOW* ftw, WINDOW* lnw, WINDOW* ofw, Cursor cursor, Cursor select_cursor, int screen_x, int screen_y) {
  wmove(ftw, 0, 0);
  FileIdentifier file_cur = cursor.file_id;

  int current_lines = getmaxy(ftw);
  int current_columns = getmaxx(ftw);

  // print text
  for (int row = screen_y; row < screen_y + current_lines; row++) {
    // getting the row to print.
    file_cur = tryToReachAbsRow(file_cur, row);

    // if the row is couldn't be reached.
    if (file_cur.absolute_row != row) {
      wmove(lnw, row - screen_y, 0);
      for (int i = 0; i < getmaxx(lnw); i++) {
        wprintw(lnw, " ");
      }

      wprintw(ftw, "~\n");

      continue;
    }


    // Print line number
    char line_number[40];
    sprintf(line_number, "%d", file_cur.absolute_row);
    int lineNumberSize = strlen(line_number);

    if (file_cur.absolute_row != cursor.file_id.absolute_row)
      wattron(lnw, A_DIM);
    else
      wattron(lnw, A_BOLD);

    wmove(lnw, row - screen_y, 0);
    for (int i = 0; i < getmaxx(lnw) - lineNumberSize - 1; i++) {
      wprintw(lnw, " ");
    }
    wprintw(lnw, "%s", line_number);
    if (file_cur.absolute_row == cursor.file_id.absolute_row)
      wprintw(lnw, "🭳");
    else
      wprintw(lnw, "🭵");

    if (file_cur.absolute_row != cursor.file_id.absolute_row)
      wattroff(lnw, A_DIM);
    else
      wattroff(lnw, A_BOLD);


    // Print line chars

    LineIdentifier begin_screen_line_cur = tryToReachAbsColumn(moduloLineIdentifierR(getLineForFileIdentifier(file_cur), 0), screen_x);
    LineIdentifier end_screen_line_cur = tryToReachAbsColumn(begin_screen_line_cur, screen_x + current_columns - 3);


    int column = screen_x;
    while (end_screen_line_cur.absolute_column >= screen_x && begin_screen_line_cur.absolute_column <= end_screen_line_cur.absolute_column && screen_x + current_columns - 3 >=
           column) {
      Char_U8 ch = getCharForLineIdentifier(begin_screen_line_cur);

      int size = charPrintSize(ch);
      // If the char is detected as not printable char.
      if (size == 0 || size == -1) {
        ch = readChar_U8FromCharArray("�");
        size = 1;
      }

      // If a char of size 2 is at the end of the line replace it by '_' to avoid line overflow.
      if (size >= 2 && screen_x + current_columns - 4 < column) {
        ch = readChar_U8FromCharArray("_");
        // size = 1;
      }

      // determine if the char is selected or not.
      bool selected_style = isCursorDisabled(select_cursor) == false && isCursorBetweenOthers(cursorOf(file_cur, begin_screen_line_cur), select_cursor, cursor);

      if (selected_style) {
        wattr_set(ftw, A_NORMAL, DEFAULT_COLOR_HOVER_PAIR, 0);
      }

      if (ch.t[0] == '\t') {
        for (int i = 0; i < TAB_SIZE; i++) {
          Char_U8 space = readChar_U8FromInput(' ');
          printChar_U8ToNcurses(ftw, space);
        }
      }
      else {
        printChar_U8ToNcurses(ftw, ch);
      }

      if (selected_style) {
        wattr_set(ftw, A_NORMAL, 0, NULL);
      }

      // move to next column
      begin_screen_line_cur.relative_column++;
      begin_screen_line_cur.absolute_column++;
      column += size;
    }

    // show empty line selected.
    if (begin_screen_line_cur.absolute_column == end_screen_line_cur.absolute_column && hasElementAfterLine(end_screen_line_cur) == false) {
      if (isCursorDisabled(select_cursor) == false
          && isCursorBetweenOthers(cursorOf(file_cur, begin_screen_line_cur), select_cursor, cursor)) {
        // if line selected
        wattr_set(ftw, A_NORMAL, DEFAULT_COLOR_HOVER_PAIR, 0);
        wprintw(ftw, " ");
        wattr_set(ftw, A_NORMAL, 0, NULL);
      }
    }

    // If the line is not fully display show '>'
    if (hasElementAfterLine(end_screen_line_cur)) {
      wattron(ftw, A_BOLD|A_UNDERLINE|A_DIM);
      wprintw(ftw, ">");
      wattroff(ftw, A_BOLD|A_UNDERLINE|A_DIM);
    }

    wprintw(ftw, "\n");
  }

  // Check if cursor is in the screen and print it if needed.
  if (cursor.file_id.absolute_row >= screen_y && cursor.file_id.absolute_row < screen_y + current_lines
      && cursor.line_id.absolute_column >= screen_x - 1 && cursor.line_id.absolute_column <= screen_x + current_columns - 3) {
    int x = getScreenXForCursor(cursor, screen_x);
    wmove(ftw, cursor.file_id.absolute_row - screen_y, x);

    char size = 1;
    if (hasElementAfterLine(cursor.line_id) == true) {
      // Check the size of the the char which is under cursor.
      Cursor tmp = moveRight(cursor);
      size = charPrintSize(getCharForLineIdentifier(tmp.line_id));
    }

    wchgat(ftw, size, A_STANDOUT, 0, NULL);
  }
  // box(ofw, 0, 0);
}

void printOpenedFile(FileContainer* files, int file_count, int current_file, int current_file_offset, WINDOW* ofw) {
  // The current position of the cursor for the first line.
  wmove(ofw, 0, 0);
  int current_offset = getbegx(ofw);
  if (current_file_offset != 0) {
    current_offset += strlen("< | ");
    wattron(ofw, A_DIM);
    wprintw(ofw, "< | ");
    wattroff(ofw, A_DIM);
  }
  // Move to the top left corner.
  for (int i = current_file_offset; i < file_count; i++) {
    // Style file names.
    if (i == current_file)
      wattron(ofw, A_BOLD);
    else
      wattron(ofw, A_DIM);
    // Print file name
    char* file_name = basename(files[i].io_file.path_args);
    current_offset += strlen(file_name);
    wprintw(ofw, "%s", file_name);
    // Style file names.
    if (i == current_file)
      wattroff(ofw, A_BOLD);
    else
      wattroff(ofw, A_DIM);
    // Print file name separator
    if (i < file_count - 1) {
      wprintw(ofw, FILE_NAME_SEPARATOR);
      current_offset += strlen(FILE_NAME_SEPARATOR);
    }
    // If the file names overflow the line, print the move right text.
    if (current_offset + strlen(FILE_NAME_SEPARATOR) > COLS) {
      wmove(ofw, 0, COLS - getbegx(ofw) - strlen("... | >"));
      wattron(ofw, A_DIM);
      wprintw(ofw, "... | >");
      wattroff(ofw, A_DIM);
      // assert((i < argc && i < max_opened_file + 1) == false);
      break;
    }
  }
  // To erase the end of the line to avoid garbage on scroll to right.
  for (int i = current_offset; i < COLS; i++) {
    wprintw(ofw, " ");
  }
  // Print the bottom line.
  wmove(ofw, 1, 0);
  for (int i = 0; i < COLS; i++) {
    wprintw(ofw, "🭸");
  }
}


void internalPrintExplorerRec(ExplorerFolder* folder, WINDOW* few, int* few_x_offset, int* few_y_offset, int tree_offset_rec, int* selected_line) {
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
    internalPrintExplorerRec(folder->folders + i, few, few_x_offset, few_y_offset, tree_offset_rec + FILE_EXPLORER_TREE_OFFSET, selected_line);
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

void printFileExplorer(ExplorerFolder* pwd, WINDOW* few, int few_x_offset, int few_y_offset, int selected_line) {
  wmove(few, 0, 0);

  internalPrintExplorerRec(pwd, few, &few_x_offset, &few_y_offset, 0, &selected_line);
  // Clear end of window
  for (int i = getcury(few) + 1; i < getmaxy(few); i++) {
    wprintw(few, "\n");
  }
  for (int i = getbegy(few); i < getmaxy(few); i++) {
    mvwprintw(few, i, getmaxx(few) - 1, "│");
  }
}

////// -------------- RESIZE FUNCTIONS --------------

void resizeEditorWindows(WINDOW** ftw, WINDOW** lnw, int y_file_editor, int lnw_width, int few_width) {
  delwin(*ftw);
  delwin(*lnw);
  *ftw = newwin(0, 0, y_file_editor, lnw_width + few_width);
  *lnw = newwin(0, lnw_width, y_file_editor, few_width);
}

void resizeOpenedFileWindow(WINDOW** ofw, bool* refresh_ofw, int edws_offset_y, int few_width) {
  delwin(*ofw);
  *ofw = newwin(edws_offset_y, 0, 0, few_width);
  *refresh_ofw = true;
}

void switchShowFew(WINDOW** few, WINDOW** ofw, WINDOW** ftw, WINDOW** lnw, int* few_width, int* saved_few_width, int ofw_height, bool* refresh_few,
                   bool* refresh_ofw) {
  if (*few == NULL) {
    // Open File Explorer Window
    *few_width = *saved_few_width;
    *few = newwin(0, *few_width, 0, 0);
    *refresh_few = true;
  }
  else {
    // Close File Explorer Window
    *saved_few_width = getmaxx(*few);
    delwin(*few);
    *few = NULL;
    *few_width = 0;
  }
  // Resize Opened File Window
  resizeOpenedFileWindow(ofw, refresh_ofw, ofw_height, *few_width);
  // Resize Editor Window
  resizeEditorWindows(ftw, lnw, ofw_height, getmaxx(*lnw), *few_width);
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
