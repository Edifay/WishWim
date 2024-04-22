#include <assert.h>
#include <ctype.h>
#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>

#include "data-structure/file_structure.h"
#include "io_management/file_manager.h"

#define CTRL_KEY(k) ((k)&0x1f)

Cursor createFile(int argc, char** args);

void printFile(Cursor cursor, int screen_x, int screen_y);

void moveScreenToMatchCursor(Cursor cursor, int* screen_x, int* screen_y);

int main(int argc, char** args) {
  setlocale(LC_ALL, "");

  Cursor cursor = createFile(argc, args);
  FileNode* root = cursor.file_id.file;
  assert(root->prev == NULL);

  int screen_x = 1;
  int screen_y = 1;

  initscr();
  raw();
  keypad(stdscr, TRUE);
  mousemask(ALL_MOUSE_EVENTS, NULL);
  mouseinterval(0);
  noecho();

  printFile(cursor, screen_x, screen_y);
  refresh();

  Cursor old_cur = cursor;
  MEVENT m_event;
  while (1) {
    checkFileIntegrity(root);

    int c = getch();
    switch (c) {
      case 588 /*BEGIN MOUSE LISTEN*/:
      case 589 /*MOUSE IN-OUT*/:
        break;
      case KEY_MOUSE:
        if (getmouse(&m_event) == OK) {
          if (m_event.bstate & BUTTON1_PRESSED) {
            FileIdentifier new_file_id = tryToReachAbsRow(cursor.file_id, screen_y + m_event.y);
            LineIdentifier new_line_id = tryToReachAbsColumn(
              moduloLineIdentifierR(getLineForFileIdentifier(new_file_id), 0), screen_x + m_event.x - 1);
            cursor = cursorOf(new_file_id, new_line_id);
          }

          if (m_event.bstate & BUTTON4_PRESSED && !(m_event.bstate & BUTTON_SHIFT)) {
            // Move Up
            screen_y -= 2;
            if (screen_y < 1)
              screen_y = 1;
          }
          else if (m_event.bstate & BUTTON4_PRESSED && m_event.bstate & BUTTON_SHIFT) {
            // Move Left
            screen_x -= 2;
            if (screen_x < 1)
              screen_x = 1;
          }

          if (m_event.bstate & BUTTON5_PRESSED && !(m_event.bstate & BUTTON_SHIFT)) {
            // Move Down
            screen_y += 2;
          }
          else if (m_event.bstate & BUTTON5_PRESSED && m_event.bstate & BUTTON_SHIFT) {
            // Move Right
            screen_x += 2;
          }
        }
        break;
      case 402/*KEY_MAJ_RIGHT*/:

        break;
      case 567 /*KEY_CTRL_RIGHT*/:

        break;
      case KEY_RESIZE:
        break;
      case '\n':
      case KEY_ENTER:
        cursor = insertNewLineInLineC(cursor);
        break;
      case KEY_RIGHT:
        cursor = moveRight(cursor);
        break;
      case KEY_LEFT:
        cursor = moveLeft(cursor);
        break;
      case KEY_UP:
        cursor = moveUp(cursor);
        break;
      case KEY_DOWN:
        cursor = moveDown(cursor);
        break;
      case KEY_BACKSPACE:
        cursor = deleteCharAtCursor(cursor);
        break;
      case 330/*KEY_SUPPR*/:
        cursor = supprCharAtCursor(cursor);
        break;
      case '\t' /*TAB*/:
        cursor = insertCharInLineC(cursor, readChar_U8FromInput(' '));
        cursor = insertCharInLineC(cursor, readChar_U8FromInput(' '));
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
      case 0:
        break;
      default:
        // printf("%d\r\n", c);
        if (iscntrl(c)) {
          printf("Unsupported touch %d\r\n", c);
          exit(0);
          goto end;
        }
        else {
          Char_U8 ch = readChar_U8FromInput(c);
          cursor = insertCharInLineC(cursor, ch);
        }
        break;
    }

    if (!areCursorEqual(cursor, old_cur)) {
      old_cur = cursor;
      moveScreenToMatchCursor(cursor, &screen_x, &screen_y);
    }
    printFile(cursor, screen_x, screen_y);
    refresh();
  }

end:

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
  if (cursor.file_id.absolute_row - (*screen_y + LINES) >= 0) {
    *screen_y = cursor.file_id.absolute_row - LINES + 1;
  }
  else if (cursor.file_id.absolute_row < *screen_y) {
    *screen_y = cursor.file_id.absolute_row;
  }

  if (cursor.line_id.absolute_column - (*screen_x + COLS - 8) >= 0) {
    *screen_x = cursor.line_id.absolute_column - COLS + 8;
    if (*screen_x < 1)
      *screen_x = 1;
  }
  else if (cursor.line_id.absolute_column - 5 < *screen_x) {
    *screen_x = cursor.line_id.absolute_column - 5;
    if (*screen_x < 1) {
      *screen_x = 1;
    }
  }
}


void printFile(Cursor cursor, int screen_x, int screen_y) {
  move(0, 0);
  for (int row = screen_y; row < screen_y + LINES; row++) {
    FileIdentifier file_cur = tryToReachAbsRow(cursor.file_id, row);
    if (file_cur.absolute_row != row) {
      printw("~\n");
    }
    else {
      LineIdentifier begin_screen_line_cur = tryToReachAbsColumn(
        moduloLineIdentifierR(getLineForFileIdentifier(file_cur), 0), screen_x);
      LineIdentifier end_screen_line_cur = tryToReachAbsColumn(begin_screen_line_cur, screen_x + COLS - 3);
      int column = screen_x;
      while (end_screen_line_cur.absolute_column >= column) {
        printChar_U8ToNcurses(getCharForLineIdentifier(begin_screen_line_cur));
        begin_screen_line_cur.relative_column++;
        begin_screen_line_cur.absolute_column++;
        column++;
      }
      if (end_screen_line_cur.relative_column != end_screen_line_cur.line->element_number || isEmptyLine(
            end_screen_line_cur.line->next) == false) {
        printw(">");
      }
      printw("\n");
    }
  }

  move(cursor.file_id.absolute_row - screen_y, cursor.line_id.absolute_column - screen_x + 1);

  if (cursor.file_id.absolute_row < screen_y || cursor.file_id.absolute_row >= screen_y + LINES
      || cursor.line_id.absolute_column < screen_x - 1 || cursor.line_id.absolute_column > screen_x + COLS - 3
  ) {
    curs_set(0);
  }
  else {
    curs_set(2);
  }
}


Cursor createFile(int argc, char** args) {
  if (argc >= 2) {
    return initWrittableFileFromFile(args[1]);
  }
  return initNewWrittableFile();
}
