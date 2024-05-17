#include <assert.h>
#include <ctype.h>
#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
#include <wchar.h>

#include "data-structure/file_management.h"
#include "data-structure/file_structure.h"
#include "data-structure/state_control.h"
#include "io_management/file_history.h"
#include "io_management/file_manager.h"
#include "utils/clipboard_manager.h"
#include "utils/key_management.h"

#define CTRL_KEY(k) ((k)&0x1f)
#define SCROLL_SPEED 3

Cursor createRoot(int argc, char** args);

void printChar_U8ToNcurses(Char_U8 ch);

void printFile(Cursor cursor, Cursor select_cursor, int screen_x, int screen_y);

void moveScreenToMatchCursor(Cursor cursor, int* screen_x, int* screen_y);

void centerCursorOnScreen(Cursor cursor, int* screen_x, int* screen_y);

int getScreenXForCursor(Cursor cursor, int screen_x);

LineIdentifier getLineIdForScreenX(LineIdentifier line_id, int screen_x, int x_click);

void setDesiredColumn(Cursor cursor, int* desired_column);

FileNode* root = NULL;

/* Unix call, use 'man wcwidth' to see explication. */
int wcwidth(const wint_t wc);

int main(int argc, char** args) {
  setlocale(LC_ALL, "");

  initscr();
  raw();
  keypad(stdscr, TRUE);
  mouseinterval(0);
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  noecho();
  curs_set(0);
  printf("\033[?1003h"); // enable mouse tracking
  fflush(stdout);

  Cursor cursor = createRoot(argc, args);

  root = cursor.file_id.file;
  assert(root->prev == NULL);

  int screen_x = 1;
  int screen_y = 1;

  fetchSavedCursorPosition(argc, args, &cursor, &screen_x, &screen_y);
  Cursor select_cursor = disableCursor(cursor);

  printFile(cursor, select_cursor, screen_x, screen_y);
  refresh();

  History history_root;
  History* history_frame = &history_root;
  initHistory(history_frame);

  int desired_column = cursor.line_id.absolute_column; // Used on line change to try to reach column.
  Cursor old_cur = cursor;
  Cursor tmp = cursor;
  MEVENT m_event;
  bool button1_down = false;
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

          // Avoid refreshing when it's just mouse movement with no change.
          if (m_event.bstate == 268435456 /*No event state*/ && button1_down == false) {
            continue;
          }

          // ---------- CURSOR ACTION ------------

          if (m_event.bstate & BUTTON1_RELEASED) {
            button1_down = false;
          }

          if (button1_down) {
            // printf("Drag detected !\n");
            FileIdentifier new_file_id = tryToReachAbsRow(cursor.file_id, screen_y + m_event.y);
            LineIdentifier new_line_id = getLineIdForScreenX(moduloLineIdentifierR(getLineForFileIdentifier(new_file_id), 0), screen_x, m_event.x);

            if (isCursorDisabled(select_cursor) == false) {
              cursor = cursorOf(new_file_id, new_line_id);
            }
            else if (areCursorEqual(cursor, cursorOf(new_file_id, new_line_id)) == false) {
              select_cursor = cursor;
              cursor = cursorOf(new_file_id, new_line_id);
            }
            setDesiredColumn(cursor, &desired_column);
          }

          if (m_event.bstate & BUTTON1_PRESSED) {
            setSelectCursorOff(&cursor, &select_cursor, SELECT_OFF_RIGHT);
            FileIdentifier new_file_id = tryToReachAbsRow(cursor.file_id, screen_y + m_event.y);
            LineIdentifier new_line_id = getLineIdForScreenX(moduloLineIdentifierR(getLineForFileIdentifier(new_file_id), 0), screen_x, m_event.x);
            cursor = cursorOf(new_file_id, new_line_id);
            setDesiredColumn(cursor, &desired_column);
            button1_down = true;
          }

          if (m_event.bstate & BUTTON1_DOUBLE_CLICKED) {
            selectWord(&cursor, &select_cursor);
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
        if (isCursorDisabled(select_cursor))
          cursor = moveRight(cursor);
        setSelectCursorOff(&cursor, &select_cursor, SELECT_OFF_RIGHT);
        setDesiredColumn(cursor, &desired_column);
        break;
      case KEY_LEFT:
        if (isCursorDisabled(select_cursor))
          cursor = moveLeft(cursor);
        setSelectCursorOff(&cursor, &select_cursor, SELECT_OFF_LEFT);
        setDesiredColumn(cursor, &desired_column);
        break;
      case KEY_UP:
        cursor = moveUp(cursor, desired_column);
        setSelectCursorOff(&cursor, &select_cursor, SELECT_OFF_LEFT);
        break;
      case KEY_DOWN:
        cursor = moveDown(cursor, desired_column);
        setSelectCursorOff(&cursor, &select_cursor, SELECT_OFF_RIGHT);
        break;
      case KEY_MAJ_RIGHT:
        setSelectCursorOn(cursor, &select_cursor);
        cursor = moveRight(cursor);
        setDesiredColumn(cursor, &desired_column);
        break;
      case KEY_MAJ_LEFT:
        setSelectCursorOn(cursor, &select_cursor);
        cursor = moveLeft(cursor);
        setDesiredColumn(cursor, &desired_column);
        break;
      case KEY_MAJ_UP:
        setSelectCursorOn(cursor, &select_cursor);
        cursor = moveUp(cursor, desired_column);
        break;
      case KEY_MAJ_DOWN:
        setSelectCursorOn(cursor, &select_cursor);
        cursor = moveDown(cursor, desired_column);
        break;
      case KEY_CTRL_RIGHT:
        setSelectCursorOff(&cursor, &select_cursor, SELECT_OFF_RIGHT);
        cursor = moveToNextWord(cursor);
        setDesiredColumn(cursor, &desired_column);
        break;
      case KEY_CTRL_LEFT:
        setSelectCursorOff(&cursor, &select_cursor, SELECT_OFF_LEFT);
        cursor = moveToPreviousWord(cursor);
        setDesiredColumn(cursor, &desired_column);
        break;
      case KEY_CTRL_DOWN:
        selectWord(&cursor, &select_cursor);
        break;
      case KEY_CTRL_UP:
        selectWord(&cursor, &select_cursor);
        break;
      case KEY_CTRL_MAJ_RIGHT:
        setSelectCursorOn(cursor, &select_cursor);
        cursor = moveToNextWord(cursor);
        setDesiredColumn(cursor, &desired_column);
        break;
      case KEY_CTRL_MAJ_LEFT:
        setSelectCursorOn(cursor, &select_cursor);
        cursor = moveToPreviousWord(cursor);
        setDesiredColumn(cursor, &desired_column);
        break;
      case KEY_CTRL_MAJ_DOWN:
        // Do something with this.
        break;
      case KEY_CTRL_MAJ_UP:
        // Do something with this.
        break;

      // ---------------------- FILE MANAGEMENT ----------------------

      case CTRL_KEY('z'):
        setSelectCursorOff(&cursor, &select_cursor, SELECT_OFF_LEFT);
        cursor = undo(&history_frame, cursor);
        setDesiredColumn(cursor, &desired_column);
        break;
      case CTRL_KEY('y'):
        setSelectCursorOff(&cursor, &select_cursor, SELECT_OFF_LEFT);
        cursor = redo(&history_frame, cursor);
        setDesiredColumn(cursor, &desired_column);
        break;
      case CTRL_KEY('c'):
        saveToClipBoard(cursor, select_cursor);
        break;
      case CTRL_KEY('x'):
        saveToClipBoard(cursor, select_cursor);
        deleteSelectionWithHist(&history_frame, &cursor, &select_cursor);
        setDesiredColumn(cursor, &desired_column);
        break;
      case CTRL_KEY('v'):
        deleteSelectionWithHist(&history_frame, &cursor, &select_cursor);
        tmp = cursor;
        cursor = loadFromClipBoard(cursor);
        saveAction(&history_frame, createInsertAction(cursor, tmp));
        setDesiredColumn(cursor, &desired_column);
        break;
      case CTRL_KEY('q'):
        goto end;
      case CTRL_KEY('s'):
        if (argc < 2) {
          printf("\r\nNo opened file\r\n");
          exit(0);
        }
        saveFile(root, args[1]);
        break;


      // ---------------------- FILE EDITING ----------------------


      case KEY_CTRL_DELETE:
        cursor = moveToPreviousWord(cursor);
        setSelectCursorOn(cursor, &select_cursor);
        cursor = moveToNextWord(cursor);
        deleteSelectionWithHist(&history_frame, &cursor, &select_cursor);
        setDesiredColumn(cursor, &desired_column);
        break;
      case '\n':
      case KEY_ENTER:
        deleteSelectionWithHist(&history_frame, &cursor, &select_cursor);
        tmp = cursor;
        cursor = insertNewLineInLineC(cursor);
        saveAction(&history_frame, createInsertAction(tmp, cursor));
        setDesiredColumn(cursor, &desired_column);
        break;
      case KEY_BACKSPACE:
        if (isCursorDisabled(select_cursor)) {
          select_cursor = moveLeft(cursor);
        }
        deleteSelectionWithHist(&history_frame, &cursor, &select_cursor);
        setDesiredColumn(cursor, &desired_column);
        break;
      case KEY_SUPPR:
        if (isCursorDisabled(select_cursor)) {
          select_cursor = moveRight(cursor);
        }
        deleteSelectionWithHist(&history_frame, &cursor, &select_cursor);
        setDesiredColumn(cursor, &desired_column);
        break;
      case KEY_TAB:
        deleteSelectionWithHist(&history_frame, &cursor, &select_cursor);
        tmp = cursor;
        cursor = insertCharInLineC(cursor, readChar_U8FromInput(' '));
        cursor = insertCharInLineC(cursor, readChar_U8FromInput(' '));
        saveAction(&history_frame, createInsertAction(tmp, cursor));
        setDesiredColumn(cursor, &desired_column);
        break;
      case CTRL_KEY('d'):
        if (isCursorDisabled(select_cursor) == true) {
          selectLine(&cursor, &select_cursor);
        }
        deleteSelectionWithHist(&history_frame, &cursor, &select_cursor);
        setDesiredColumn(cursor, &desired_column);
        break;


      default:
        // printf("%d\r\n", c);
        // exit(0);
        if (iscntrl(c)) {
          printf("Unsupported touch %d\r\n", c);
          if (argc >= 2) saveFile(root, args[1]);
          // exit(0);
          goto end;
        }
        else {
          deleteSelectionWithHist(&history_frame, &cursor, &select_cursor);
          cursor = insertCharInLineC(cursor, readChar_U8FromInput(c));
          setDesiredColumn(cursor, &desired_column);
          saveAction(&history_frame, createInsertAction(old_cur, cursor));
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

  if (argc >= 2) {
    setlastFilePosition(args[1], cursor.file_id.absolute_row, cursor.line_id.absolute_column, screen_x, screen_y);
  }

  endwin();
  destroyFullFile(root);
  destroyEndOfHistory(&history_root);
  return 0;
}

void printChar_U8ToNcurses(Char_U8 ch) {
  int size = sizeChar_U8(ch);
  for (int i = 0; i < size; i++) {
    printw("%c", ch.t[i]);
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
    while (end_screen_line_cur.absolute_column >= screen_x && begin_screen_line_cur.absolute_column <= end_screen_line_cur.absolute_column && screen_x + COLS - 3 >= column) {
      Char_U8 ch = getCharForLineIdentifier(begin_screen_line_cur);

      int size = charPrintSize(ch);
      // If the char is detected as not printable char.
      if (size == 0 || size == -1) {
        ch = readChar_U8FromCharArray("�");
        size = 1;
      }

      // If a char of size 2 is at the end of the line replace it by '_' to avoid line overflow.
      if (size == 2 && screen_x + COLS - 4 < column) {
        ch = readChar_U8FromCharArray("_");
        size = 1;
      }

      // determine if the char is selected or not.
      bool selected_style = isCursorDisabled(select_cursor) == false
                            && isCursorBetweenOthers(cursorOf(file_cur, begin_screen_line_cur), select_cursor, cursor);

      if (selected_style)
        attron(A_STANDOUT|A_DIM);

      printChar_U8ToNcurses(ch);

      if (selected_style)
        attroff(A_STANDOUT|A_DIM);

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

  // Check if cursor is in the screen and print it if needed.
  if (cursor.file_id.absolute_row >= screen_y && cursor.file_id.absolute_row < screen_y + LINES
      && cursor.line_id.absolute_column >= screen_x - 1 && cursor.line_id.absolute_column <= screen_x + COLS - 3) {
    int x = getScreenXForCursor(cursor, screen_x);
    move(cursor.file_id.absolute_row - screen_y, x);

    char size = 1;
    if (hasElementAfterLine(cursor.line_id) == true) {
      // Check the size of the the char which is under cursor.
      Cursor tmp = moveRight(cursor);
      size = charPrintSize(getCharForLineIdentifier(tmp.line_id));
    }

    chgat(size, A_STANDOUT, 0, NULL);
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

  int screen_x_wide_char = getScreenXForCursor(cursor, *screen_x) + *screen_x;
  if (screen_x_wide_char - (*screen_x + COLS - 8) >= 0) {
    *screen_x = screen_x_wide_char - COLS + 8;
    if (*screen_x < 1) *screen_x = 1;
  }
  else if (screen_x_wide_char - 5 < *screen_x) {
    *screen_x = screen_x_wide_char - 5;
    if (*screen_x < 1) {
      *screen_x = 1;
    }
  }
}

void centerCursorOnScreen(Cursor cursor, int* screen_x, int* screen_y) {
  // center for y, but right for x.
  *screen_x = cursor.line_id.absolute_column - (COLS /*/ 2*/);
  *screen_y = cursor.file_id.absolute_row - (LINES / 2);

  if (*screen_x < 1)
    *screen_x = 1;
  if (*screen_y < 1)
    *screen_y = 1;

  // To match right for x.
  moveScreenToMatchCursor(cursor, screen_x, screen_y);
}

Cursor createRoot(int argc, char** args) {
  if (argc >= 2) {
    return initWrittableFileFromFile(args[1]);
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
