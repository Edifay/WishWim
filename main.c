#include <assert.h>
#include <ctype.h>
#include <locale.h>
#include <ncurses.h>
#include <wchar.h>

#include "data-structure/file_management.h"
#include "data-structure/file_structure.h"
#include "data-structure/state_control.h"
#include "data-structure/term_handler.h"
#include "io_management/file_history.h"
#include "io_management/io_manager.h"
#include "utils/clipboard_manager.h"
#include "utils/key_management.h"

#define CTRL_KEY(k) ((k)&0x1f)
#define SCROLL_SPEED 3


int main(int argc, char** args) {
  setlocale(LC_ALL, "");

  WINDOW* ftw = NULL; // File Text Window
  WINDOW* lnw = NULL; // Line Number Window
  int edw_yoffset = 0; // Offset of the y for ftw and lnw.

  // Init ncurses
  initscr();
  ftw = newwin(0, 0, edw_yoffset, 0); // lnw_width will auto-resize.
  lnw = newwin(0, 0, 0, 0);
  raw();
  keypad(stdscr, TRUE);
  mouseinterval(0);
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  noecho();
  curs_set(0);
  printf("\033[?1003h"); // enable mouse tracking
  fflush(stdout);

  FileContainer file_container;
  setupFileContainer(argc, args, &file_container);

  // setups current file vars
  IO_FileID* io_file = &file_container.io_file; // Describe the IO file on OS
  FileNode** root = &file_container.root; // The root of the File object
  Cursor* cursor = &file_container.cursor; // The current cursor for the root File
  Cursor* select_cursor = &file_container.select_cursor; // The cursor used to make selection
  Cursor* old_cur = &file_container.old_cur; // Old cursor used to flag cursor change
  int* desired_column = &file_container.desired_column; // Used on line change to try to reach column
  int* screen_x = &file_container.screen_x; // The x coord of the top left corner of the current viewport of the file
  int* screen_y = &file_container.screen_y; // The y coord of the top left corner of the current viewport of the file
  int* old_screen_x = &file_container.old_screen_x; // old screen_x used to flag screen_x changes
  int* old_screen_y = &file_container.old_screen_y; // old screen_y used to flag screen_y changes
  History* history_root = &file_container.history_root; // Root of History object for the current File
  History** history_frame = &file_container.history_frame; // Current node of the History. Before -> Undo, After -> Redo.

  // First print
  printFile(ftw, lnw, *cursor, *select_cursor, *screen_x, *screen_y);
  refresh();
  wrefresh(ftw);
  wrefresh(lnw);

  // Start automated-machine
  *old_cur = *cursor;
  Cursor tmp;
  MEVENT m_event;
  bool button1_down = false;
  while (true) {
    assert(checkFileIntegrity(*root) == true);

    int c = getch();
    switch (c) {
      // ---------------------- NCURSES THINGS ----------------------

      case BEGIN_MOUSE_LISTEN:
      case MOUSE_IN_OUT:
      case KEY_RESIZE:
        resizeEditorWindows(&ftw, &lnw, edw_yoffset, ftw->_begx);
        break;

      // ---------------------- MOUSE ----------------------

      case KEY_MOUSE:
        if (getmouse(&m_event) == OK) {
          detectComplexEvents(&m_event);

          // Avoid refreshing when it's just mouse movement with no change.
          if (m_event.bstate == 268435456 /*No event state*/ && button1_down == false) {
            continue;
          }

          if (m_event.y - edw_yoffset < 0 && button1_down == true) {
            m_event.y = edw_yoffset;
          }


          // ---------- CURSOR ACTION ------------

          if (m_event.bstate & BUTTON1_RELEASED) {
            button1_down = false;
          }

          if (button1_down) {
            // printf("Drag detected !\n");
            FileIdentifier new_file_id = tryToReachAbsRow(cursor->file_id, *screen_y + m_event.y - edw_yoffset);
            LineIdentifier new_line_id = getLineIdForScreenX(moduloLineIdentifierR(getLineForFileIdentifier(new_file_id), 0), *screen_x, m_event.x - ftw->_begx);

            if (isCursorDisabled(*select_cursor) == false) {
              *cursor = cursorOf(new_file_id, new_line_id);
            }
            else if (areCursorEqual(*cursor, cursorOf(new_file_id, new_line_id)) == false) {
              *select_cursor = *cursor;
              *cursor = cursorOf(new_file_id, new_line_id);
            }
            setDesiredColumn(*cursor, desired_column);
          }

          if (m_event.bstate & BUTTON1_PRESSED) {
            setSelectCursorOff(cursor, select_cursor, SELECT_OFF_RIGHT);
            FileIdentifier new_file_id = tryToReachAbsRow(cursor->file_id, *screen_y + m_event.y - edw_yoffset);
            LineIdentifier new_line_id = getLineIdForScreenX(moduloLineIdentifierR(getLineForFileIdentifier(new_file_id), 0), *screen_x, m_event.x - ftw->_begx);
            *cursor = cursorOf(new_file_id, new_line_id);
            setDesiredColumn(*cursor, desired_column);
            button1_down = true;
          }

          if (m_event.bstate & BUTTON1_DOUBLE_CLICKED) {
            selectWord(cursor, select_cursor);
          }


          // ---------- SCROLL ------------
          if (m_event.bstate & BUTTON4_PRESSED && !(m_event.bstate & BUTTON_SHIFT)) {
            // Move Up
            *screen_y -= SCROLL_SPEED;
            if (*screen_y < 1) *screen_y = 1;
          }
          else if (m_event.bstate & BUTTON4_PRESSED && m_event.bstate & BUTTON_SHIFT) {
            // Move Left
            *screen_x -= SCROLL_SPEED;
            if (*screen_x < 1) *screen_x = 1;
          }

          if (m_event.bstate & BUTTON5_PRESSED && !(m_event.bstate & BUTTON_SHIFT)) {
            // Move Down
            *screen_y += SCROLL_SPEED;
          }
          else if (m_event.bstate & BUTTON5_PRESSED && m_event.bstate & BUTTON_SHIFT) {
            // Move Right
            *screen_x += SCROLL_SPEED;
          }
        }
        break;

      // ---------------------- MOVEMENT ----------------------


      case KEY_RIGHT:
        if (isCursorDisabled(*select_cursor))
          *cursor = moveRight(*cursor);
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_RIGHT);
        setDesiredColumn(*cursor, desired_column);
        break;
      case KEY_LEFT:
        if (isCursorDisabled(*select_cursor))
          *cursor = moveLeft(*cursor);
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_LEFT);
        setDesiredColumn(*cursor, desired_column);
        break;
      case KEY_UP:
        *cursor = moveUp(*cursor, *desired_column);
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_LEFT);
        break;
      case KEY_DOWN:
        *cursor = moveDown(*cursor, *desired_column);
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_RIGHT);
        break;
      case KEY_MAJ_RIGHT:
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveRight(*cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case KEY_MAJ_LEFT:
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveLeft(*cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case KEY_MAJ_UP:
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveUp(*cursor, *desired_column);
        break;
      case KEY_MAJ_DOWN:
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveDown(*cursor, *desired_column);
        break;
      case KEY_CTRL_RIGHT:
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_RIGHT);
        *cursor = moveToNextWord(*cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case KEY_CTRL_LEFT:
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_LEFT);
        *cursor = moveToPreviousWord(*cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case KEY_CTRL_DOWN:
        selectWord(cursor, select_cursor);
        break;
      case KEY_CTRL_UP:
        selectWord(cursor, select_cursor);
        break;
      case KEY_CTRL_MAJ_RIGHT:
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveToNextWord(*cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case KEY_CTRL_MAJ_LEFT:
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveToPreviousWord(*cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case KEY_CTRL_MAJ_DOWN:
        // Do something with this.
        break;
      case KEY_CTRL_MAJ_UP:
        // Do something with this.
        break;

      // ---------------------- FILE MANAGEMENT ----------------------

      case CTRL_KEY('z'):
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_LEFT);
        *cursor = undo(history_frame, *cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case CTRL_KEY('y'):
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_LEFT);
        *cursor = redo(history_frame, *cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case CTRL_KEY('c'):
        saveToClipBoard(*cursor, *select_cursor);
        break;
      case CTRL_KEY('x'):
        saveToClipBoard(*cursor, *select_cursor);
        deleteSelectionWithHist(history_frame, cursor, select_cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case CTRL_KEY('v'):
        deleteSelectionWithHist(history_frame, cursor, select_cursor);
        tmp = *cursor;
        *cursor = loadFromClipBoard(*cursor);
        saveAction(history_frame, createInsertAction(*cursor, tmp));
        setDesiredColumn(*cursor, desired_column);
        break;
      case CTRL_KEY('q'):
        goto end;
      case CTRL_KEY('s'):
        if (io_file->status == NONE) {
          printf("\r\nNo specified file\r\n");
          goto end;
        }
        saveFile(*root, io_file);
        assert(io_file->status == EXIST);
        setlastFilePosition(io_file->path_abs, cursor->file_id.absolute_row, cursor->line_id.absolute_column, *screen_x, *screen_y);
        saveCurrentStateControl(*history_root, *history_frame, io_file->path_abs);
        break;


      // ---------------------- FILE EDITING ----------------------


      case KEY_CTRL_DELETE:
        *cursor = moveToPreviousWord(*cursor);
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveToNextWord(*cursor);
        deleteSelectionWithHist(history_frame, cursor, select_cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case '\n':
      case KEY_ENTER:
        deleteSelectionWithHist(history_frame, cursor, select_cursor);
        tmp = *cursor;
        *cursor = insertNewLineInLineC(*cursor);
        saveAction(history_frame, createInsertAction(tmp, *cursor));
        setDesiredColumn(*cursor, desired_column);
        break;
      case KEY_BACKSPACE:
        if (isCursorDisabled(*select_cursor)) {
          *select_cursor = moveLeft(*cursor);
        }
        deleteSelectionWithHist(history_frame, cursor, select_cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case KEY_SUPPR:
        if (isCursorDisabled(*select_cursor)) {
          *select_cursor = moveRight(*cursor);
        }
        deleteSelectionWithHist(history_frame, cursor, select_cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case KEY_TAB:
        deleteSelectionWithHist(history_frame, cursor, select_cursor);
        tmp = *cursor;
        *cursor = insertCharInLineC(*cursor, readChar_U8FromInput(' '));
        *cursor = insertCharInLineC(*cursor, readChar_U8FromInput(' '));
        saveAction(history_frame, createInsertAction(tmp, *cursor));
        setDesiredColumn(*cursor, desired_column);
        break;
      case CTRL_KEY('d'):
        if (isCursorDisabled(*select_cursor) == true) {
          selectLine(cursor, select_cursor);
        }
        deleteSelectionWithHist(history_frame, cursor, select_cursor);
        setDesiredColumn(*cursor, desired_column);
        break;


      default:
        // printf("%d\r\n", c);
        // exit(0);
        if (iscntrl(c)) {
          printf("Unsupported touch %d\r\n", c);
          // if (file.status != NONE) saveFile(root, &file);
          // goto end;
        }
        else {
          deleteSelectionWithHist(history_frame, cursor, select_cursor);
          *cursor = insertCharInLineC(*cursor, readChar_U8FromInput(c));
          setDesiredColumn(*cursor, desired_column);
          saveAction(history_frame, createInsertAction(*old_cur, *cursor));
        }
        break;
    }

    // flag cursor change
    if (!areCursorEqual(*cursor, *old_cur)) {
      *old_cur = *cursor;
      moveScreenToMatchCursor(ftw, *cursor, screen_x, screen_y);
    }

    // flag screen_x change
    if (*old_screen_x != *screen_x) {
      *old_screen_x = *screen_x;
    }

    // flag screen_y change
    if (*old_screen_y != *screen_y) {
      *old_screen_y = *screen_y;
      // resize line_w to match with line_number_length
      int new_lnw_width = numberOfDigitOfNumber(*screen_y + ftw->_maxy) + 1 /* +1 for the line */;
      if (new_lnw_width != ftw->_begx) {
        resizeEditorWindows(&ftw, &lnw, edw_yoffset, new_lnw_width);
      }
    }


    printFile(ftw, lnw, *cursor, *select_cursor, *screen_x, *screen_y);
    refresh();
    wrefresh(ftw);
    wrefresh(lnw);
  }

end:

  printf("\033[?1003l\n"); // Disable mouse movement events, as l = low
  fflush(stdout);

  if (io_file->status == EXIST) {
    setlastFilePosition(io_file->path_abs, cursor->file_id.absolute_row, cursor->line_id.absolute_column, *screen_x, *screen_y);
  }

  endwin();
  destroyFullFile(*root);
  destroyEndOfHistory(history_root);
  return 0;
}
