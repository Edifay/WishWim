#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <bits/time.h>

#include "advanced/theme.h"
#include "advanced/tree-sitter/tree_manager.h"
#include "data-management/file_management.h"
#include "data-management/file_structure.h"
#include "data-management/state_control.h"
#include "io_management/io_explorer.h"
#include "io_management/viewport_history.h"
#include "io_management/io_manager.h"
#include "io_management/workspace_settings.h"
#include "utils/clipboard_manager.h"
#include "utils/key_management.h"
#include "utils/constants.h"
#include "terminal/term_handler.h"
#include "terminal/highlight.h"
#include "terminal/click_handler.h"
#include "../lib/tree-sitter/lib/include/tree_sitter/api.h"
#include "config/config.h"

#define CTRL_KEY(k) ((k)&0x1f)

int color_pair = 3;
int color_index = 100;
cJSON* config;
ParserList parsers;


int main(int file_count, char** args) {
  // remove first args which is the executable file name.
  char** file_names = args;
  file_names++;
  file_count--;
  setlocale(LC_ALL, "");

  // Load config
  config = loadConfig();

  // Parser Datas
  initParserList(&parsers);

  // Init GUI vars
  WINDOW* ftw = NULL; // File Text Window
  WINDOW* lnw = NULL; // Line Number Window
  WINDOW* ofw = NULL; // Opened Files Window
  WINDOW* few = NULL; // File Explorer Window
  bool refresh_edw = true; // Need to reprint editor window
  bool refresh_ofw = true; // Need to reprint opened file window
  bool refresh_few = true; // Need to reprint file explorer window
  WINDOW* focus_w = NULL; // Used to set the window where start mouse drag

  // EDW Datas

  // OFW Datas
  int current_file_offset = 0;
  int ofw_height = 0; // Height of Opened Files Window. 0 => Disabled on start.   OPENED_FILE_WINDOW_HEIGHT => Enabled on start.

  // Few Datas
  int few_width = 0; // File explorer width
  int saved_few_width = FILE_EXPLORER_WIDTH;
  int few_x_offset = 0; /* TODO unused */
  int few_y_offset = 0; // Y Scroll state of File Explorer Window
  int few_selected_line = -1;

  // Init ncurses
  initscr();
  ftw = newwin(0, 0, ofw_height, few_width);
  lnw = newwin(0, 0, 0, few_width);
  ofw = newwin(ofw_height, 0, 0, few_width);
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

  // Containers of current opened buffers.
  FileContainer* files;
  int current_file = 0; // The current showed file.

  // Detect workspace settings
  WorkspaceSettings loaded_settings;
  bool usingWorkspace = false;
  if (file_count == 1 || file_count == 0) {
    char* dir_name = file_count == 0 ? getenv("PWD") : file_names[0];

    if (isDir(dir_name)) {
      loaded_settings.dir_path = dir_name;
      usingWorkspace = true;

      bool settings_exist = loadWorkspaceSettings(dir_name, &loaded_settings);

      // consume dir name
      if (file_count == 1) {
        file_count--;
        file_names++;
      }
      assert(file_count == 0);

      if (settings_exist) {
        // Setup opened files.
        file_count = loaded_settings.file_count;
        file_names = loaded_settings.files;
        current_file = loaded_settings.current_opened_file;

        // File Opened Window state.
        if (loaded_settings.showing_opened_file_window == true) {
          ofw_height = OPENED_FILE_WINDOW_HEIGHT;
        }
        else {
          ofw_height = 0;
        }
        // File Explorer Window state.
        if (loaded_settings.showing_file_explorer_window == true) {
          ungetch(CTRL_KEY('e'));
        }
      }
    }
  }


  // allocating space for file at open.
  files = malloc(sizeof(FileContainer) * max(1, file_count));


  // Fill files with args
  for (int i = 0; i < file_count; i++) {
    setupFileContainer(file_names[i], files + i);
  }
  // If no args setup first untitled file.
  if (file_count == 0) {
    file_count = 1;
    setupFileContainer("", files);
  }

  // Folder used for file explorer.
  ExplorerFolder pwd;
  if (usingWorkspace == true) {
    initFolder(loaded_settings.dir_path, &pwd);
  }
  else {
    initFolder(getenv("PWD"), &pwd);
  }
  pwd.open = true;


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
  History** history_root; // Root of History object for the current File
  History** history_frame; // Current node of the History. Before -> Undo, After -> Redo.
  FileHighlightDatas* highlight_data;

  bool refresh_local_vars = true; // Need to re-set local vars

  // Start automated-machine
  Cursor tmp;
  MEVENT m_event;
  History* old_history_frame;
  long* payload;
  bool mouse_drag = false;
  time_val last_time_mouse_drag = timeInMilliseconds();
  while (true) {
    //// --------------- Post Processing -----------------

    if (refresh_local_vars == true) {
      setupLocalVars(files, current_file, &io_file, &root, &cursor, &select_cursor, &old_cur, &desired_column, &screen_x, &screen_y, &old_screen_x, &old_screen_y, &history_root,
                     &history_frame, &highlight_data);
      refresh_local_vars = false;
      old_history_frame = *history_frame;
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
      int new_lnw_width = numberOfDigitOfNumber(*screen_y + getmaxy(ftw)) + 1 /* +1 for the line */;
      if (new_lnw_width != getbegx(ftw)) {
        resizeEditorWindows(&ftw, &lnw, ofw_height, new_lnw_width, few_width);
      }
    }

    // If it needed to reparse the current file for tree. Looking for state changes.
    if (highlight_data->is_active == true && (old_history_frame != *history_frame || highlight_data->tree == NULL)) {
      edit_and_parse_tree(root, history_frame, highlight_data, &old_history_frame);
      optimizeHistory(*history_root, history_frame);
      old_history_frame = *history_frame;
    }

    //// --------------- Paint GUI -----------------

    refresh();

    // Refresh File Explorer Window
    if (refresh_few == true && few_width != 0 && few != NULL) {
      printFileExplorer(&pwd, few, few_x_offset, few_y_offset, few_selected_line);
      wrefresh(few);
      refresh_few = false;
    }

    // Refresh File Opened Window
    if ((refresh_ofw == true || files[current_file].io_file.status == NONE) && ofw_height != 0) {
      printOpenedFile(files, file_count, current_file, current_file_offset, ofw);
      wrefresh(ofw);
      refresh_ofw = false;
    }

    // Refresh Editor Windows
    if (refresh_edw == true) {
      printEditor(ftw, lnw, ofw, *cursor, *select_cursor, *screen_x, *screen_y);

      double time_taken;

      // If highlight is enable on this file.
      if (highlight_data->is_active == true) {
        clock_t t;
        t = clock();

        ParserContainer* parser = getParserForLanguage(&parsers, highlight_data->lang_name);
        assert(parser != NULL);

        long* args_fct = malloc(11 * sizeof(long *));
        args_fct[0] = (long)&parser->highlight_queries;
        args_fct[1] = (long)&parser->theme_list;
        args_fct[2] = (long)highlight_data->tmp_file_dump;
        args_fct[3] = (long)ftw;
        args_fct[4] = (long)screen_x;
        args_fct[5] = (long)screen_y;
        args_fct[6] = (long)getmaxx(ftw);
        args_fct[7] = (long)getmaxy(ftw);
        args_fct[8] = (long)cursor;
        args_fct[9] = (long)select_cursor;
        args_fct[10] = (long)NULL;

        TSNode root_node = ts_tree_root_node(highlight_data->tree);
        TreePath path[100]; // TODO refactor this, there is a problem if the depth of the tree is bigger than 100.

        treeForEachNodeSized(*screen_y, *screen_x, getmaxy(ftw), getmaxx(ftw), root_node, path, 0, checkMatchForHighlight, args_fct);

        t = clock() - t;
        time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds

        free((Cursor *)args_fct[10]);
        free(args_fct);
      }

      wrefresh(lnw);
      wrefresh(ftw);
      // printf("fun() took %f seconds to execute \n\r", time_taken);
    }


    assert(checkFileIntegrity(*root) == true);

  read_input:
    int c = getch();

    // TODO Here check to do background operation like lsp_servers.

    switch (c) {
      // ---------------------- NCURSES THINGS ----------------------

      case ERR:
        goto read_input;

      case BEGIN_MOUSE_LISTEN:
      case MOUSE_IN_OUT:
      case KEY_RESIZE:
        // Was there but idk why... => Avoid biggest size only used on time before automated resize.
        assert((getmaxx(lnw) + few_width >= COLS) == false);
      // Resize Opened File Window
        resizeOpenedFileWindow(&ofw, &refresh_ofw, ofw_height, few_width);
        refresh_ofw = true;
        resizeEditorWindows(&ftw, &lnw, ofw_height, getmaxx(lnw), few_width);
        refresh_edw = true;
        refresh_few = true;
        break;

      // ---------------------- MOUSE ----------------------

      case KEY_MOUSE:
        if (getmouse(&m_event) == OK) {
          detectComplexEvents(&m_event);

          // Avoid refreshing when it's just mouse movement with no change.
          if (m_event.bstate == NO_EVENT_MOUSE /*No event state*/ && mouse_drag == false) {
            goto read_input;
          }

          // Avoid too much refresh, to avoid input buffer full.
          if (m_event.bstate == NO_EVENT_MOUSE && mouse_drag == true) {
            if (diff2Time(last_time_mouse_drag, timeInMilliseconds()) < 30) {
              goto read_input;
            }
            last_time_mouse_drag = timeInMilliseconds();
          }


          // If pressed enable drag
          if (m_event.bstate & BUTTON1_PRESSED) {
            mouse_drag = true;
          }

          if ((m_event.x < getbegx(lnw) && focus_w == NULL) || (few != NULL && focus_w == few)) {
            // Click in File Explorer Window
            if (m_event.bstate & BUTTON1_PRESSED) {
              focus_w = few;
            }
            handleFileExplorerClick(&files, &file_count, &current_file, &pwd, &few_y_offset, &few_x_offset, &few_width, &few_selected_line, ofw_height, ofw_height, &few, &ofw,
                                    &lnw, &ftw,
                                    m_event, &refresh_few, &refresh_ofw, &refresh_edw, &refresh_local_vars);
          }
          else if ((m_event.y - ofw_height < 0 && focus_w == NULL) || (ofw != NULL && focus_w == ofw)) {
            // Click on opened file window
            if (m_event.bstate & BUTTON1_PRESSED) {
              focus_w = ofw;
            }
            handleOpenedFileClick(files, &file_count, &current_file, m_event, &current_file_offset, ofw, &refresh_ofw, &refresh_local_vars, mouse_drag);
          }
          else {
            // Click on editor windows
            if (m_event.bstate & BUTTON1_PRESSED) {
              focus_w = ftw;
            }
            handleEditorClick(getbegx(ftw), ofw_height, cursor, select_cursor, desired_column, screen_x, screen_y, &m_event, mouse_drag);
          }

          if (m_event.bstate & BUTTON1_RELEASED) {
            focus_w = NULL;
            mouse_drag = false;
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
        refresh_local_vars = true;
      // TODO check if the file selected is showing in ofw. If not move it in.
        refresh_ofw = true;
        break;
      case KEY_CTRL_MAJ_UP:
        // Do something with this.
        if (current_file != file_count - 1)
          current_file++;
        refresh_local_vars = true;
      // TODO check if the file selected is showing in ofw. If not move it in.
        refresh_ofw = true;
        break;

      // ---------------------- FILE MANAGEMENT ----------------------

      case CTRL_KEY('z'):
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_LEFT);
        payload = get_payload_edit_and_parse_tree(&root, &highlight_data);
        *cursor = undo(history_frame, *cursor, edit_and_parse_tree_from_payload, payload);
        free(payload);
        old_history_frame = *history_frame;
        setDesiredColumn(*cursor, desired_column);
        break;
      case CTRL_KEY('y'):
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_LEFT);
        payload = get_payload_edit_and_parse_tree(&root, &highlight_data);
        *cursor = redo(history_frame, *cursor, edit_and_parse_tree_from_payload, payload);
        free(payload);
        old_history_frame = *history_frame;
        setDesiredColumn(*cursor, desired_column);
        break;
      case CTRL_KEY('c'):
        saveToClipBoard(*cursor, *select_cursor);
        break;
      case CTRL_KEY('a'):
        *select_cursor = tryToReachAbsPosition(*cursor, 1, 0);
        *cursor = tryToReachAbsPosition(*cursor, INT_MAX, INT_MAX);
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
        // TODO refactor
        if (false)
          for (int i = 0; i < file_count; i++) {
            if (files[i].io_file.status == NONE) {
              continue;
            }
            saveFile(files[i].root, &files[i].io_file);
            assert(io_file->status == EXIST);
            setlastFilePosition(files[i].io_file.path_abs, files[i].cursor.file_id.absolute_row, files[i].cursor.line_id.absolute_column, files[i].screen_x, files[i].screen_y);
            saveCurrentStateControl(*files[i].history_root, files[i].history_frame, files[i].io_file.path_abs);
          }
        goto end;
      case CTRL_KEY('w'):
        closeFile(&files, &file_count, &current_file, &refresh_ofw, &refresh_edw, &refresh_local_vars);
        break;
      case CTRL_KEY('s'):
        if (io_file->status == NONE) {
          printf("\r\nNo specified file\r\n");
          goto end;
        }
        saveFile(*root, io_file);
        assert(io_file->status == EXIST);
        setlastFilePosition(io_file->path_abs, cursor->file_id.absolute_row, cursor->line_id.absolute_column, *screen_x, *screen_y);
        saveCurrentStateControl(**history_root, *history_frame, io_file->path_abs);
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


      // ---------------------- EDITOR SHORTCUTS ----------------------


      case CTRL_KEY('e'): // File Explorer Window Switch
        if (few == NULL) {
          // Open File Explorer Window
          few_width = saved_few_width;
          few = newwin(0, few_width, 0, 0);
          if (pwd.open == false) {
            discoverFolder(&pwd);
          }
          refresh_few = true;
        }
        else {
          // Close File Explorer Window
          saved_few_width = getmaxx(few);
          delwin(few);
          few = NULL;
          few_width = 0;
        }
      // Resize Opened File Window
        resizeOpenedFileWindow(&ofw, &refresh_ofw, ofw_height, few_width);
      // Resize Editor Window
        resizeEditorWindows(&ftw, &lnw, ofw_height, getmaxx(lnw), few_width);
        break;
      case CTRL_KEY('l'): // Opened File Window Switch
        if (ofw_height == OPENED_FILE_WINDOW_HEIGHT) {
          ofw_height = 0;
        }
        else {
          ofw_height = OPENED_FILE_WINDOW_HEIGHT;
        }
        resizeOpenedFileWindow(&ofw, &refresh_ofw, ofw_height, few_width);
        resizeEditorWindows(&ftw, &lnw, ofw_height, getmaxx(lnw), few_width);
        refresh_ofw = true;
        refresh_edw = true;
        break;


      default:
        if (iscntrl(c)) {
          printf("Unsupported touch %d\r\n", c);
        }
        else {
          deleteSelectionWithHist(history_frame, cursor, select_cursor);
          tmp = *cursor;
          *cursor = insertCharInLineC(*cursor, readChar_U8FromInput(c));
          setDesiredColumn(*cursor, desired_column);
          saveAction(history_frame, createInsertAction(tmp, *cursor));
        }
        break;
    }

    // While end.
  }

end:

  printf("\033[?1003l\n"); // Disable mouse movement events, as l = low
  fflush(stdout);


  if (usingWorkspace == true) {
    WorkspaceSettings new_settings;
    getWorkspaceSettingsForCurrentDir(&new_settings, files, file_count, current_file, ofw_height != 0, few_width != 0, FILE_EXPLORER_WIDTH);
    saveWorkspaceSettings(loaded_settings.dir_path, &new_settings);
    destroyWorkspaceSettings(&new_settings);
  }

  // Destroy all files
  for (int i = 0; i < file_count; i++) {
    destroyFileContainer(files + i);
  }
  destroyFolder(&pwd);
  free(files);
  cJSON_Delete(config);
  destroyParserList(&parsers);
  destroyWorkspaceSettings(&loaded_settings);

  // We need to sleep a bit before flush input to wait for the terminal to disable mouse tracking.
  usleep(30000);
  flushinp();
  endwin();
  return 0;
}
