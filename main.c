#include <assert.h>
#include <ctype.h>
#include <libgen.h>
#include <locale.h>
#include <ncurses.h>
#include <string.h>
#include <wchar.h>

#include "data-structure/file_management.h"
#include "data-structure/file_structure.h"
#include "data-structure/state_control.h"
#include "data-structure/term_handler.h"
#include "io_management/viewport_history.h"
#include "io_management/io_manager.h"
#include "utils/clipboard_manager.h"
#include "utils/key_management.h"
#include "utils/constants.h"

#define CTRL_KEY(k) ((k)&0x1f)
#define MAX_OPENED_FILE 100


void setupLocalVars(FileContainer* files, int current_file, IO_FileID** io_file, FileNode*** root, Cursor** cursor, Cursor** select_cursor, Cursor** old_cur, int** desired_column,
                    int** screen_x, int** screen_y, int** old_screen_x, int** old_screen_y, History** history_root, History*** history_frame);


int main(int argc, char** args) {
  setlocale(LC_ALL, "");

  FileContainer files[MAX_OPENED_FILE];
  int current_file = 0;

  // Init GUI vars
  WINDOW* ftw = NULL; // File Text Window
  WINDOW* lnw = NULL; // Line Number Window
  WINDOW* ofw = NULL; // Opened Files Window
  int edws_offset_y = 2; // Offset of the y for ftw and lnw, and height of ofw.

  // Init ncurses
  initscr();
  ftw = newwin(0, 0, edws_offset_y, 0);
  lnw = newwin(0, 0, 0, 0);
  ofw = newwin(edws_offset_y, 0, 0, 0);
  raw();
  keypad(stdscr, TRUE);
  mouseinterval(0);
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  noecho();
  curs_set(0);
  printf("\033[?1003h"); // enable mouse tracking
  fflush(stdout);

  // Fill files with args
  for (int i = 1; i < argc && i < MAX_OPENED_FILE + 1; i++) {
    setupFileContainer(args[i], files + i - 1);
  }
  // Fill rest of un used FileContainer allocated by new buffers.
  for (int i = argc; i < MAX_OPENED_FILE + 1; i++) {
    setupFileContainer("", files + i - 1);
  }

  // Setup redirection of vars to use with out pass though file_container obj.
  IO_FileID* io_file; // Describe the IO file on OS
  FileNode** root; // The root of the File object
  Cursor* cursor; // The current cursor for the root File
  Cursor* select_cursor; // The cursor used to make selection
  Cursor* old_cur; // Old cursor used to flag cursor change
  int* desired_column; // Used on line change to try to reach column
  int* screen_x; // The x coord of the top left corner of the current viewport of the file
  int* screen_y; // The y coord of the top left corner of the current viewport of the file
  int* old_screen_x; // old screen_x used to flag screen_x changes
  int* old_screen_y; // old screen_y used to flag screen_y changes
  History* history_root; // Root of History object for the current File
  History** history_frame; // Current node of the History. Before -> Undo, After -> Redo.

  // Setup current locals vars to files[current_file]
  setupLocalVars(files, current_file, &io_file, &root, &cursor, &select_cursor, &old_cur, &desired_column, &screen_x, &screen_y, &old_screen_x, &old_screen_y, &history_root,
                 &history_frame);

  // First print
  printEditor(ftw, lnw, ofw, *cursor, *select_cursor, *screen_x, *screen_y);

  // print file opened explorer
  printOpenedFileExplorer(argc, files, MAX_OPENED_FILE, current_file, ofw);

  refresh();
  wrefresh(ftw);
  wrefresh(lnw);
  wrefresh(ofw);

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
        resizeEditorWindows(&ftw, &lnw, edws_offset_y, ftw->_begx);
        printOpenedFileExplorer(argc, files, MAX_OPENED_FILE, current_file, ofw);
        break;

      // ---------------------- MOUSE ----------------------

      case KEY_MOUSE:
        if (getmouse(&m_event) == OK) {
          detectComplexEvents(&m_event);

          // Avoid refreshing when it's just mouse movement with no change.
          if (m_event.bstate == 268435456 /*No event state*/ && button1_down == false) {
            continue;
          }


          if (m_event.y - edws_offset_y < 0 && button1_down == false) {
            // Click on opened file window

            int current_offset = 0;
            for (int i = 0; i < argc - 1 && i < MAX_OPENED_FILE; i++) {
              if (files[i].io_file.status == NONE)
                break;

              current_offset += strlen(basename(files[i].io_file.path_args));
              if (m_event.x < current_offset + (strlen(FILE_NAME_SEPARATOR) / 2)) {
                // Don't change anything if the file clicked is currently the file opened.
                if (current_file == i)
                  break;

                current_file = i;
                setupLocalVars(files, current_file, &io_file, &root, &cursor, &select_cursor, &old_cur, &desired_column, &screen_x, &screen_y, &old_screen_x, &old_screen_y,
                               &history_root, &history_frame);
                printOpenedFileExplorer(argc, files, MAX_OPENED_FILE, current_file, ofw);
                break;
              }

              current_offset += strlen(FILE_NAME_SEPARATOR);
            }
          }
          else {
            // Click on editor windows
            handleEditorClick(ftw->_begx, edws_offset_y, cursor, select_cursor, desired_column, screen_x, screen_y, &m_event, &button1_down);
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
        if (current_file != 0)
          current_file--;
        setupLocalVars(files, current_file, &io_file, &root, &cursor, &select_cursor, &old_cur, &desired_column, &screen_x, &screen_y, &old_screen_x, &old_screen_y, &history_root,
                       &history_frame);
        printOpenedFileExplorer(argc, files, MAX_OPENED_FILE, current_file, ofw);
        break;
      case KEY_CTRL_MAJ_UP:
        // Do something with this.
        if (current_file != MAX_OPENED_FILE - 1)
          current_file++;
        setupLocalVars(files, current_file, &io_file, &root, &cursor, &select_cursor, &old_cur, &desired_column, &screen_x, &screen_y, &old_screen_x, &old_screen_y, &history_root,
                       &history_frame);
        printOpenedFileExplorer(argc, files, MAX_OPENED_FILE, current_file, ofw);
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
        if (TAB_CHAR_USE) {
          *cursor = insertCharInLineC(*cursor, readChar_U8FromInput('\t'));
        }
        else {
          for (int i = 0; i < TAB_SIZE; i++) {
            *cursor = insertCharInLineC(*cursor, readChar_U8FromInput(' '));
          }
        }

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
        if (iscntrl(c)) {
          printf("Unsupported touch %d\r\n", c);
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
        resizeEditorWindows(&ftw, &lnw, edws_offset_y, new_lnw_width);
      }
    }


    printEditor(ftw, lnw, ofw, *cursor, *select_cursor, *screen_x, *screen_y);
    refresh();
    wrefresh(ftw);
    wrefresh(lnw);
    wrefresh(ofw);
  }

end:

  printf("\033[?1003l\n"); // Disable mouse movement events, as l = low
  fflush(stdout);

  endwin();
  // Destroy all files
  for (int i = 0; i < MAX_OPENED_FILE; i++) {
    if (files[i].io_file.status == EXIST) {
      setlastFilePosition(files[i].io_file.path_abs, files[i].cursor.file_id.absolute_row, files[i].cursor.line_id.absolute_column, files[i].screen_x, files[i].screen_y);
    }
    destroyFullFile(files[i].root);
    destroyEndOfHistory(&files[i].history_root);
  }
  return 0;
}


void setupLocalVars(FileContainer* files, int current_file, IO_FileID** io_file, FileNode*** root, Cursor** cursor, Cursor** select_cursor, Cursor** old_cur, int** desired_column,
                    int** screen_x, int** screen_y, int** old_screen_x, int** old_screen_y, History** history_root, History*** history_frame) {
  *io_file = &files[current_file].io_file; // Describe the IO file on OS
  *root = &files[current_file].root; // The root of the File object
  *cursor = &files[current_file].cursor; // The current cursor for the root File
  *select_cursor = &files[current_file].select_cursor; // The cursor used to make selection
  *old_cur = &files[current_file].old_cur; // Old cursor used to flag cursor change
  *desired_column = &files[current_file].desired_column; // Used on line change to try to reach column
  *screen_x = &files[current_file].screen_x; // The x coord of the top left corner of the current viewport of the file
  *screen_y = &files[current_file].screen_y; // The y coord of the top left corner of the current viewport of the file
  *old_screen_x = &files[current_file].old_screen_x; // old screen_x used to flag screen_x changes
  *old_screen_y = &files[current_file].old_screen_y; // old screen_y used to flag screen_y changes
  *history_root = &files[current_file].history_root; // Root of History object for the current File
  *history_frame = &files[current_file].history_frame; // Current node of the History. Before -> Undo, After -> Redo.
  **old_screen_y = -1;
}
