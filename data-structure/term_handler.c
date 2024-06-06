#include <assert.h>
#include <string.h>


#include "term_handler.h"
#include "file_management.h"

void printChar_U8ToNcurses(WINDOW* w, Char_U8 ch) {
  int size = sizeChar_U8(ch);
  for (int i = 0; i < size; i++) {
    wprintw(w, "%c", ch.t[i]);
  }
}

void printFile(WINDOW* file_w, WINDOW* line_w, Cursor cursor, Cursor select_cursor, int screen_x, int screen_y) {
  wmove(file_w, 0, 0);
  FileIdentifier file_cur = cursor.file_id;

  int current_lines = file_w->_maxy + 1;
  int current_columns = file_w->_maxx + 1;

  // print text
  for (int row = screen_y; row < screen_y + current_lines; row++) {
    // getting the row to print.
    file_cur = tryToReachAbsRow(file_cur, row);

    // if the row is couldn't be reached.
    if (file_cur.absolute_row != row) {
      wmove(line_w, row - screen_y, 0);
      for (int i = 0; i < line_w->_maxx + 1; i++) {
        wprintw(line_w, " ");
      }

      wprintw(file_w, "~\n");

      continue;
    }


    // Print line number
    char line_number[40];
    sprintf(line_number, "%d", file_cur.absolute_row);
    int lineNumberSize = strlen(line_number);

    if (file_cur.absolute_row != cursor.file_id.absolute_row)
      wattron(line_w, A_DIM);
    else
      wattron(line_w, A_BOLD);

    wmove(line_w, row - screen_y, 0);
    for (int i = 0; i < line_w->_maxx - lineNumberSize; i++) {
      wprintw(line_w, " ");
    }
    wprintw(line_w, "%s", line_number);
    if (file_cur.absolute_row == cursor.file_id.absolute_row)
      wprintw(line_w, "🭳");
    else
      wprintw(line_w, "🭵");

    if (file_cur.absolute_row != cursor.file_id.absolute_row)
      wattroff(line_w, A_DIM);
    else
      wattroff(line_w, A_BOLD);


    // Print line chars

    LineIdentifier begin_screen_line_cur = tryToReachAbsColumn(
      moduloLineIdentifierR(getLineForFileIdentifier(file_cur), 0), screen_x);
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
      if (size == 2 && screen_x + current_columns - 4 < column) {
        ch = readChar_U8FromCharArray("_");
        size = 1;
      }

      // determine if the char is selected or not.
      bool selected_style = isCursorDisabled(select_cursor) == false
                            && isCursorBetweenOthers(cursorOf(file_cur, begin_screen_line_cur), select_cursor, cursor);

      if (selected_style)
        wattron(file_w, A_STANDOUT|A_DIM);

      printChar_U8ToNcurses(file_w, ch);

      if (selected_style)
        wattroff(file_w, A_STANDOUT|A_DIM);

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
        wattron(file_w, A_STANDOUT|A_DIM);
        wprintw(file_w, " ");
        wattroff(file_w, A_STANDOUT|A_DIM);
      }
    }

    // If the line is not fully display show '>'
    if (hasElementAfterLine(end_screen_line_cur)) {
      wattron(file_w, A_BOLD|A_UNDERLINE|A_DIM);
      wprintw(file_w, ">");
      wattroff(file_w, A_BOLD|A_UNDERLINE|A_DIM);
    }

    wprintw(file_w, "\n");
  }

  // Check if cursor is in the screen and print it if needed.
  if (cursor.file_id.absolute_row >= screen_y && cursor.file_id.absolute_row < screen_y + current_lines
      && cursor.line_id.absolute_column >= screen_x - 1 && cursor.line_id.absolute_column <= screen_x + current_columns - 3) {
    int x = getScreenXForCursor(cursor, screen_x);
    wmove(file_w, cursor.file_id.absolute_row - screen_y, x);

    char size = 1;
    if (hasElementAfterLine(cursor.line_id) == true) {
      // Check the size of the the char which is under cursor.
      Cursor tmp = moveRight(cursor);
      size = charPrintSize(getCharForLineIdentifier(tmp.line_id));
    }

    wchgat(file_w, size, A_STANDOUT, 0, NULL);
  }
}

void moveScreenToMatchCursor(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y) {
  int current_lines = w->_maxy;
  int current_columns = w->_maxx;

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

Cursor createRoot(IO_FileID file) {
  if (file.status == EXIST) {
    return initWrittableFileFromFile(file.path_abs);
  }
  return initNewWrittableFile();
}


int getScreenXForCursor(Cursor cursor, int screen_x) {
  Cursor initial = cursor;
  Cursor old_cursor = cursor;
  int atAdd = 0;

  if (cursor.line_id.absolute_column != 0 && charPrintSize(getCharForLineIdentifier(cursor.line_id)) == 2) {
    atAdd++;
  }
  cursor = moveLeft(cursor);


  while (screen_x <= cursor.line_id.absolute_column && areCursorEqual(cursor, old_cursor) == false && cursor.file_id.absolute_row == old_cursor.file_id.absolute_row) {
    assert(cursor.line_id.absolute_column != 0);
    if (charPrintSize(getCharForLineIdentifier(cursor.line_id)) == 2) {
      atAdd++;
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

void resizeEditorWindows(WINDOW** file_w, WINDOW** line_w, int y_file_editor, int new_start_x) {
  delwin(*file_w);
  delwin(*line_w);
  *file_w = newwin(0, 0, y_file_editor, new_start_x);
  *line_w = newwin(0, new_start_x, y_file_editor, 0);
}
