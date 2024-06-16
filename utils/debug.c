#include <ncurses.h>
#include <stdbool.h>

#include "../data-structure/file_structure.h"
#include "../data-structure/file_management.h"
#include "debug.h"

bool isCursorDisabled2(Cursor cursor) {
  return cursor.file_id.absolute_row == -1;
}

Cursor disableCursor2(Cursor cursor) {
  cursor.file_id.absolute_row = -1;
  return cursor;
}

void printChar_U8ToNcurses2(Char_U8 ch) {
  int size = sizeChar_U8(ch);
  for (int i = 0; i < size; i++) {
    printw("%c", ch.t[i]);
  }
}


void printFile2(Cursor cursor, Cursor select_cursor, int screen_x, int screen_y) {
  move(0, 0);
  FileIdentifier file_cur = cursor.file_id;

  // print text
  for (int row = screen_y; row < screen_y + LINES; row++) {
    // getting the row to print.
    file_cur = tryToReachAbsRow(file_cur, row);

    // if the row is couldn't be reached.
    if (file_cur.absolute_row != row) {
      printw("~\n");
      continue;
    }

    LineIdentifier begin_screen_line_cur = tryToReachAbsColumn(
      moduloLineIdentifierR(getLineForFileIdentifier(file_cur), 0), screen_x);
    LineIdentifier end_screen_line_cur = tryToReachAbsColumn(begin_screen_line_cur, screen_x + COLS - 3);


    int column = screen_x;
    while (end_screen_line_cur.absolute_column >= column) {
      // determine if the char is selected or not.
      bool selected_style = isCursorDisabled2(select_cursor) == false
                            && isCursorBetweenOthers(cursorOf(file_cur, begin_screen_line_cur), select_cursor, cursor);

      if (selected_style)
        attron(A_STANDOUT|A_DIM);

      printChar_U8ToNcurses2(getCharForLineIdentifier(begin_screen_line_cur));

      if (selected_style)
        attroff(A_STANDOUT|A_DIM);

      // move to next column
      begin_screen_line_cur.relative_column++;
      begin_screen_line_cur.absolute_column++;
      column++;
    }

    // If the line is not fully display show >
    if (hasElementAfterLine(end_screen_line_cur)) {
      attron(A_BOLD|A_UNDERLINE|A_DIM);
      printw(">");
      attroff(A_BOLD|A_UNDERLINE|A_DIM);
    }

    printw("\n");
  }

  // Check if cursor is in the screen and print it.
  if (cursor.file_id.absolute_row >= screen_y && cursor.file_id.absolute_row < screen_y + LINES
      && cursor.line_id.absolute_column >= screen_x - 1 && cursor.line_id.absolute_column <= screen_x + COLS - 3) {
    move(cursor.file_id.absolute_row - screen_y, cursor.line_id.absolute_column - screen_x + 1);
    chgat(1, A_STANDOUT, 0, NULL);
  }
}


void debugPrintFullFile() {
  if (root == NULL)
    return;

  Cursor cursor_tmp = cursorOf(moduloFileIdentifierR(root, 1), moduloLineIdentifierR(getLineForFileIdentifier(moduloFileIdentifierR(root, 1)), 0));
  Cursor select = disableCursor2(cursor_tmp);
  int screen_x = 1;
  int screen_y = 1;
  printFile2(cursor_tmp, select, screen_x, screen_y);
  getch();
}
