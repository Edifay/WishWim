#include <assert.h>
#include <ctype.h>
#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>

#include "data-structure/file_management.h"
#include "data-structure/file_structure.h"
#include "io_management/file_manager.h"
#include "utils/key_management.h"

#define CTRL_KEY(k) ((k)&0x1f)
#define SCROLL_SPEED 3

Cursor createFile(int argc, char** args);

void printFile(Cursor cursor, Cursor select_cursor, int screen_x, int screen_y);

void moveScreenToMatchCursor(Cursor cursor, int* screen_x, int* screen_y);

bool isCursorDisabled(Cursor cursor);

Cursor disableCursor(Cursor cursor);

FileNode* root = NULL;

void setSelectCursorOn(Cursor cursor, Cursor* select_cursor);

void setSelectCursorOff(Cursor* select_cursor) {
  *select_cursor = disableCursor(*select_cursor);
}

int main(int argc, char** args) {
  setlocale(LC_ALL, "");

  Cursor cursor = createFile(argc, args);
  root = cursor.file_id.file;
  assert(root->prev == NULL);

  int screen_x = 1;
  int screen_y = 1;

  Cursor select_cursor = disableCursor(cursor);

  initscr();
  raw();
  keypad(stdscr, TRUE);
  mouseinterval(0);
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  noecho();
  curs_set(0);

  printf("\033[?1003h"); // enable mouse tracking
  fflush(stdout);


  printFile(cursor, select_cursor, screen_x, screen_y);
  refresh();


  Cursor old_cur = cursor;
  MEVENT m_event;
  bool button1_pressed = false;
  while (true) {
    assert(checkFileIntegrity(root) == true);

    int c = getch();
    switch (c) {
      // ---------------------- NCURSES THINGS ----------------------

      case BEGIN_MOUSE_LISTEN:
      case MOUSE_IN_OUT:
      case KEY_RESIZE:
        break;

      // ---------------------- MOUSE ----------------------

      case KEY_MOUSE:
        if (getmouse(&m_event) == OK) {
          detectComplexEvents(&m_event);

          // ---------- MOVE CURSOR ------------

          if (m_event.bstate & BUTTON1_RELEASED) {
            button1_pressed = false;
          }

          if (button1_pressed) {
            // printf("Drag detected !\n");
            FileIdentifier new_file_id = tryToReachAbsRow(cursor.file_id, screen_y + m_event.y);
            LineIdentifier new_line_id = tryToReachAbsColumn(moduloLineIdentifierR(getLineForFileIdentifier(new_file_id), 0), screen_x + m_event.x - 1);

            if (isCursorDisabled(select_cursor) == false) {
              cursor = cursorOf(new_file_id, new_line_id);
            }
            else if (areCursorEqual(cursor, cursorOf(new_file_id, new_line_id)) == false) {
              select_cursor = cursor;
              cursor = cursorOf(new_file_id, new_line_id);
            }
          }

          if (m_event.bstate & BUTTON1_PRESSED) {
            FileIdentifier new_file_id = tryToReachAbsRow(cursor.file_id, screen_y + m_event.y);
            LineIdentifier new_line_id = tryToReachAbsColumn(moduloLineIdentifierR(getLineForFileIdentifier(new_file_id), 0), screen_x + m_event.x - 1);
            cursor = cursorOf(new_file_id, new_line_id);
            button1_pressed = true;
            setSelectCursorOff(&select_cursor);
          }

          if (m_event.bstate & BUTTON1_DOUBLE_CLICKED) {
            if (cursor.line_id.absolute_column != 0 && isInvisible(getCharForLineIdentifier(cursor.line_id)) == false) {
              cursor = moveToPreviousWord(cursor);
            }
            setSelectCursorOn(cursor, &select_cursor);
            cursor = moveToNextWord(cursor);
          }


          // ---------- SCROLL ------------
          if (m_event.bstate & BUTTON4_PRESSED && !(m_event.bstate & BUTTON_SHIFT)) {
            // Move Up
            screen_y -= SCROLL_SPEED;
            if (screen_y < 1) screen_y = 1;
          }
          else if (m_event.bstate & BUTTON4_PRESSED && m_event.bstate & BUTTON_SHIFT) {
            // Move Left
            screen_x -= SCROLL_SPEED;
            if (screen_x < 1) screen_x = 1;
          }

          if (m_event.bstate & BUTTON5_PRESSED && !(m_event.bstate & BUTTON_SHIFT)) {
            // Move Down
            screen_y += SCROLL_SPEED;
          }
          else if (m_event.bstate & BUTTON5_PRESSED && m_event.bstate & BUTTON_SHIFT) {
            // Move Right
            screen_x += SCROLL_SPEED;
          }
        }
        break;

      // ---------------------- MOVEMENT ----------------------


      case KEY_RIGHT:
        cursor = moveRight(cursor);
        setSelectCursorOff(&select_cursor);
        break;
      case KEY_LEFT:
        cursor = moveLeft(cursor);
        setSelectCursorOff(&select_cursor);
        break;
      case KEY_UP:
        cursor = moveUp(cursor);
        setSelectCursorOff(&select_cursor);
        break;
      case KEY_DOWN:
        cursor = moveDown(cursor);
        setSelectCursorOff(&select_cursor);
        break;
      case KEY_MAJ_RIGHT:
        setSelectCursorOn(cursor, &select_cursor);
        cursor = moveRight(cursor);
        break;
      case KEY_MAJ_LEFT:
        setSelectCursorOn(cursor, &select_cursor);
        cursor = moveLeft(cursor);
        break;
      case KEY_MAJ_UP:
        setSelectCursorOn(cursor, &select_cursor);
        cursor = moveUp(cursor);
        break;
      case KEY_MAJ_DOWN:
        setSelectCursorOn(cursor, &select_cursor);
        cursor = moveDown(cursor);
        break;
      case KEY_CTRL_RIGHT:
        cursor = moveToNextWord(cursor);
        setSelectCursorOff(&select_cursor);
        break;
      case KEY_CTRL_LEFT:
        cursor = moveToPreviousWord(cursor);
        setSelectCursorOff(&select_cursor);
        break;
      case KEY_CTRL_DOWN:
        // Do something with this.
        break;
      case KEY_CTRL_UP:
        // Do something with this.
        break;
      case KEY_CTRL_MAJ_RIGHT:
        setSelectCursorOn(cursor, &select_cursor);
        cursor = moveToNextWord(cursor);
        break;
      case KEY_CTRL_MAJ_LEFT:
        setSelectCursorOn(cursor, &select_cursor);
        cursor = moveToPreviousWord(cursor);
        break;
      case KEY_CTRL_MAJ_DOWN:
        // Do something with this.
        break;
      case KEY_CTRL_MAJ_UP:
        // Do something with this.
        break;

      // ---------------------- FILE INPUT ----------------------

      case '\n':
      case KEY_ENTER:
        setSelectCursorOff(&select_cursor);
        cursor = insertNewLineInLineC(cursor);
        break;
      case KEY_BACKSPACE:
        setSelectCursorOff(&select_cursor);
        cursor = deleteCharAtCursor(cursor);
        break;
      case KEY_SUPPR:
        setSelectCursorOff(&select_cursor);
        cursor = supprCharAtCursor(cursor);
        break;
      case '\t':
        setSelectCursorOff(&select_cursor);
        cursor = insertCharInLineC(cursor, readChar_U8FromInput(' '));
        cursor = insertCharInLineC(cursor, readChar_U8FromInput(' '));
        break;
      case CTRL_KEY('q'):
        goto end;
      case CTRL_KEY('d'):
        setSelectCursorOff(&select_cursor);
        cursor = deleteLineAtCursor(cursor);
        break;
      case CTRL_KEY('s'):
        if (argc < 2) {
          printf("\r\nNo opened file\r\n");
          exit(0);
        }
        saveFile(root, args[1]);
        break;
      default:
        // printf("%d\r\n", c);
        if (iscntrl(c)) {
          printf("Unsupported touch %d\r\n", c);
          if (argc >= 2) saveFile(root, args[1]);
          // exit(0);
          goto end;
        }
        else {
          setSelectCursorOff(&select_cursor);
          Char_U8 ch = readChar_U8FromInput(c);
          cursor = insertCharInLineC(cursor, ch);
        }
        break;
    }

    if (!areCursorEqual(cursor, old_cur)) {
      old_cur = cursor;
      moveScreenToMatchCursor(cursor, &screen_x, &screen_y);
    }

    printFile(cursor, select_cursor, screen_x, screen_y);
    refresh();
  }

end:

  printf("\033[?1003l\n"); // Disable mouse movement events, as l = low
  fflush(stdout);

  endwin();
  destroyFullFile(root);
  return 0;
}

void printChar_U8ToNcurses(Char_U8 ch) {
  int size = sizeChar_U8(ch);
  for (int i = 0; i < size; i++) {
    printw("%c", ch.t[i]);
  }
}

void moveScreenToMatchCursor(Cursor cursor, int* screen_x, int* screen_y) {
  if (cursor.file_id.absolute_row - (*screen_y + LINES) + 1 >= 0) {
    *screen_y = cursor.file_id.absolute_row - LINES + 2;
    if (*screen_y < 1) *screen_y = 1;
  }
  else if (cursor.file_id.absolute_row < *screen_y + 1) {
    *screen_y = cursor.file_id.absolute_row - 1;
    if (*screen_y < 1) *screen_y = 1;
  }

  if (cursor.line_id.absolute_column - (*screen_x + COLS - 8) >= 0) {
    *screen_x = cursor.line_id.absolute_column - COLS + 8;
    if (*screen_x < 1) *screen_x = 1;
  }
  else if (cursor.line_id.absolute_column - 5 < *screen_x) {
    *screen_x = cursor.line_id.absolute_column - 5;
    if (*screen_x < 1) {
      *screen_x = 1;
    }
  }
}


void printFile(Cursor cursor, Cursor select_cursor, int screen_x, int screen_y) {
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
      bool selected_style = isCursorDisabled(select_cursor) == false
                            && isCursorBetweenOthers(cursorOf(file_cur, begin_screen_line_cur), select_cursor, cursor);

      if (selected_style)
        attron(A_STANDOUT|A_DIM);

      printChar_U8ToNcurses(getCharForLineIdentifier(begin_screen_line_cur));

      if (selected_style)
        attroff(A_STANDOUT|A_DIM);

      // move to next column
      begin_screen_line_cur.relative_column++;
      begin_screen_line_cur.absolute_column++;
      column++;
    }

    // show empty line selected.
    if (begin_screen_line_cur.absolute_column == end_screen_line_cur.absolute_column && begin_screen_line_cur.line->element_number == 0) {
      if (isCursorDisabled(select_cursor) == false
          && isCursorBetweenOthers(cursorOf(file_cur, begin_screen_line_cur), select_cursor, cursor)) {
        // if line selected
        attron(A_STANDOUT|A_DIM);
        printw(" ");
        attroff(A_STANDOUT|A_DIM);
      }
    }

    // If the line is not fully display show '>'
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


Cursor createFile(int argc, char** args) {
  if (argc >= 2) {
    return initWrittableFileFromFile(args[1]);
  }
  return initNewWrittableFile();
}

bool isCursorDisabled(Cursor cursor) {
  return cursor.file_id.absolute_row == -1;
}

Cursor disableCursor(Cursor cursor) {
  cursor.file_id.absolute_row = -1;
  return cursor;
}

void setSelectCursorOn(Cursor cursor, Cursor* select_cursor) {
  if (isCursorDisabled(*select_cursor) == true) {
    *select_cursor = cursor;
  }
}
