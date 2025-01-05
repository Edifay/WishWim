#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ttydefaults.h>

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
#include "advanced/lsp/lsp_client.h"
#include "config/config.h"


/**   TODO list :
 *      - For action, add byte_start && byte_end in the struct.
 *      - Rework tree_edit, create a callback in state_control to apply edit on tree.
 *      - Rework the printEditor for colors. Currently we override char with colored char, that's not optimized.
 *          Create a struct to define a coloration in a file, and calculate it before each paint.
 *
 *
 *
 */


// Global vars.
int color_pair = 3;
int color_index = 100;
cJSON* config;
ParserList parsers;
LSPServerLinkedList lsp_servers;
WorkspaceSettings loaded_settings;

// copy from http://www.cse.yorku.ca/~oz/hash.html
int hashString(unsigned char* str) {
  unsigned long hash = 5381;
  int c;

  while (c = *str++)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}


void dispatcher(cJSON* packet, long* payload) {
  if (packet != NULL) {
    fprintf(stderr, "\n\n <<< ================ %s ================\n", cJSON_GetStringValue(cJSON_GetObjectItem(packet, "method")));
    cJSON* params = cJSON_GetObjectItem(packet, "params");
    char* text = cJSON_Print(params);
    fprintf(stderr, "%s\n", text);
    free(text);
  }
}


int main(int file_count, char** args) {
  // TODO Remove when lsp_logs.txt will be unused.
  system("echo "" > lsp_logs.txt");
  system("echo "" > tree_logs.txt");
  // remove first args which is the executable file name.
  char** file_names = args;
  file_names++;
  file_count--;
  setlocale(LC_ALL, "");

  // Load config
  config = loadConfig();

  // Parser Datas
  initParserList(&parsers);
  initLSPServerList(&lsp_servers);

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
  int current_file_index = 0; // The current showed file.

  // Detect workspace settings
  loaded_settings.is_used = false;
  if (file_count == 1 || file_count == 0) {
    char* dir_name = file_count == 0 ? getenv("PWD") : file_names[0];

    if (isDir(dir_name)) {
      loaded_settings.dir_path = dir_name;
      loaded_settings.is_used = true;

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


        // --- UI State ---

        // Current showed file.
        current_file_index = loaded_settings.current_opened_file;

        // File Opened Window state.
        if (loaded_settings.showing_opened_file_window == true) {
          ofw_height = OPENED_FILE_WINDOW_HEIGHT;
        }
        else {
          ofw_height = 0;
        }

        // File Explorer Window state.
        if (loaded_settings.showing_file_explorer_window == true) {
          ungetch(CTRL('e'));
        }
      }
    }
  }


  // allocating space for file at open.
  files = malloc(sizeof(FileContainer) * max(1, file_count));


  // Fill files with args
  for (int i = 0; i < file_count; i++) {
    setupFileContainer(file_names[i], files + i);

    if (files[i].lsp_datas.is_enable) {
      char* dump = dumpSelection(tryToReachAbsPosition(files[i].cursor, 1, 0), tryToReachAbsPosition(files[i].cursor, INT_MAX, INT_MAX));
      LSP_notifyLspFileDidOpen(*getLSPServerForLanguage(&lsp_servers, files[i].lsp_datas.name), files[i].io_file.path_args, dump);
      free(dump);
    }
  }
  // If no args setup first untitled file.
  if (file_count == 0) {
    file_count = 1;
    setupFileContainer("", files);
    if (files[0].lsp_datas.is_enable) {
      LSP_notifyLspFileDidOpen(*getLSPServerForLanguage(&lsp_servers, files[0].lsp_datas.name), files[0].io_file.path_args, "");
    }
  }

  // Folder used for file explorer.
  ExplorerFolder pwd;
  if (loaded_settings.is_used == true) {
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
  FileHighlightDatas* highlight_data; // Contain the configuration for file higlight.

  bool refresh_local_vars = true; // Need to re-set local vars

  // Start automated-machine
  Cursor tmp;
  MEVENT m_event;
  History* old_history_frame;
  PayloadStateChange payload_state_change;
  bool mouse_drag = false;
  time_val last_time_mouse_drag = timeInMilliseconds();
  while (true) {
    //// --------------- Post Processing -----------------

    if (refresh_local_vars == true) {
      setupLocalVars(files, current_file_index, &io_file, &root, &cursor, &select_cursor, &old_cur, &desired_column, &screen_x, &screen_y, &old_screen_x, &old_screen_y,
                     &history_root,
                     &history_frame, &highlight_data);
      refresh_local_vars = false;
      old_history_frame = *history_frame;
      payload_state_change = getPayloadStateChange(highlight_data);
    }

    // flag cursor change
    if (!areCursorEqual(*cursor, *old_cur)) {
      *old_cur = *cursor;
      moveScreenToMatchCursor(ftw, *cursor, screen_x, screen_y);
    }

    // flag screen_x change
    if (*old_screen_x != *screen_x) {
      *old_screen_x = *screen_x;
      // UNUSED
    }

    // flag screen_y change
    if (*old_screen_y != *screen_y) {
      *old_screen_y = *screen_y;
      // resize lnw_w to match with line_number_length
      int new_lnw_width = numberOfDigitOfNumber(*screen_y + getmaxy(ftw)) + 1 /* +1 for the line */;
      if (new_lnw_width != getbegx(ftw)) {
        resizeEditorWindows(&ftw, &lnw, ofw_height, new_lnw_width, few_width);
      }
    }

    // If it needed to reparse the current file for tree. Looking for state changes.
    if (highlight_data->is_active == true && (old_history_frame != *history_frame || highlight_data->tree == NULL)) {
      // edit_and_parse_tree(root, history_frame, highlight_data, &old_history_frame);
      fprintf(stderr, "Parse !\n");
      parse_tree(root, history_frame, highlight_data, &old_history_frame);
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
    if ((refresh_ofw == true || files[current_file_index].io_file.status == NONE) && ofw_height != 0) {
      printOpenedFile(files, file_count, current_file_index, current_file_offset, ofw);
      wrefresh(ofw);
      refresh_ofw = false;
    }

    // Refresh Editor Windows
    if (refresh_edw == true) {
      printEditor(ftw, lnw, ofw, *cursor, *select_cursor, *screen_x, *screen_y);

      // If highlight is enable on this file.
      if (highlight_data->is_active == true) {
        highlightCurrentFile(highlight_data, ftw, screen_x, screen_y, cursor, select_cursor);
      }

      wrefresh(lnw);
      wrefresh(ftw);
    }


    assert(checkFileIntegrity(*root) == true);

  read_input:
    int c = getch();
    int hash = c;

    if (c != KEY_MOUSE && c != -1) {
      // fprintf(stderr, "Code %d, Key : '%s' hash into %d.\n", c, keyname(c), hashString(keyname(c)));
    }


    if (c != KEY_MOUSE && c != -1) {
      const char* key_str = keyname(c);
      if (key_str != NULL && key_str[0] != '\0') {
        if (key_str[0] == '^') {
        }
        else {
          hash = hashString(key_str);
        }
      }
      else {
        fprintf(stderr, "keyname is NULL");
      }
    }

    // TODO Here check to do background operation like lsp_servers.

    LSPServerLinkedList_Cell* cell = lsp_servers.head;
    while (cell != NULL) {
      LSP_dispatchOnReceive(&cell->lsp_server, dispatcher, NULL);
      cell = cell->next;
    }

    switch (hash) {
      // ---------------------- NCURSES THINGS ----------------------

      case ERR:
        goto read_input;

      case BEGIN_MOUSE_LISTEN:
      case MOUSE_IN_OUT:
      case H_KEY_RESIZE:
        // Was there but idk why... => Avoid biggest size only used on time before automated resize.
        assert((getmaxx(lnw) + few_width >= COLS) == false);
      // Resize Opened File Window
        resizeOpenedFileWindow(&ofw, &refresh_ofw, ofw_height, few_width);
        resizeEditorWindows(&ftw, &lnw, ofw_height, getmaxx(lnw), few_width);
        refresh_ofw = refresh_edw = refresh_few = true;
        break;

      // ---------------------- MOUSE ----------------------

      case KEY_MOUSE:
        if (getmouse(&m_event) != OK) {
          fprintf(stderr, "MOUVE_EVENT_NOT_OK !\r\n");
          exit(0);
          break;
        }

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
          handleFileExplorerClick(&files, &file_count, &current_file_index, &pwd, &few_y_offset, &few_x_offset, &few_width, &few_selected_line, ofw_height, ofw_height, &few, &ofw,
                                  &lnw, &ftw,
                                  m_event, &refresh_few, &refresh_ofw, &refresh_edw, &refresh_local_vars);
        }
        else if ((m_event.y - ofw_height < 0 && focus_w == NULL) || (ofw != NULL && focus_w == ofw)) {
          // Click on opened file window
          if (m_event.bstate & BUTTON1_PRESSED) {
            focus_w = ofw;
          }
          handleOpenedFileClick(files, &file_count, &current_file_index, m_event, &current_file_offset, ofw, &refresh_ofw, &refresh_local_vars, mouse_drag);
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

        break;

      // ---------------------- MOVEMENT ----------------------


      case H_KEY_RIGHT:
        if (isCursorDisabled(*select_cursor))
          *cursor = moveRight(*cursor);
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_RIGHT);
        setDesiredColumn(*cursor, desired_column);
        break;
      case H_KEY_LEFT:
        if (isCursorDisabled(*select_cursor))
          *cursor = moveLeft(*cursor);
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_LEFT);
        setDesiredColumn(*cursor, desired_column);
        break;
      case H_KEY_UP:
        *cursor = moveUp(*cursor, *desired_column);
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_LEFT);
        break;
      case H_KEY_DOWN:
        *cursor = moveDown(*cursor, *desired_column);
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_RIGHT);
        break;
      case H_KEY_MAJ_RIGHT:
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveRight(*cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case H_KEY_MAJ_LEFT:
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveLeft(*cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case H_KEY_MAJ_UP:
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveUp(*cursor, *desired_column);
        break;
      case H_KEY_MAJ_DOWN:
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveDown(*cursor, *desired_column);
        break;
      case H_KEY_CTRL_RIGHT:
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_RIGHT);
        *cursor = moveToNextWord(*cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case H_KEY_CTRL_LEFT:
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_LEFT);
        *cursor = moveToPreviousWord(*cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case H_KEY_CTRL_DOWN:
        selectWord(cursor, select_cursor);
        break;
      case H_KEY_CTRL_UP:
        selectWord(cursor, select_cursor);
        break;
      case H_KEY_CTRL_MAJ_RIGHT:
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveToNextWord(*cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case H_KEY_CTRL_MAJ_LEFT:
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveToPreviousWord(*cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case H_KEY_CTRL_MAJ_DOWN:
        // Do something with this.
        if (current_file_index != 0)
          current_file_index--;
        refresh_local_vars = true;
      // TODO check if the file selected is showing in ofw. If not move it in.
        refresh_ofw = true;
        break;
      case H_KEY_CTRL_MAJ_UP:
        // Do something with this.
        if (current_file_index != file_count - 1)
          current_file_index++;
        refresh_local_vars = true;
      // TODO check if the file selected is showing in ofw. If not move it in.
        refresh_ofw = true;
        break;

      // ---------------------- FILE MANAGEMENT ----------------------

      case CTRL('z'):
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_LEFT);
        *cursor = undo(history_frame, *cursor, onStateChangeTS, (long *)&payload_state_change);
        old_history_frame = NULL;
        setDesiredColumn(*cursor, desired_column);
        break;
      case CTRL('y'):
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_LEFT);
        *cursor = redo(history_frame, *cursor, onStateChangeTS, (long *)&payload_state_change);
        old_history_frame = NULL;
        setDesiredColumn(*cursor, desired_column);
        break;
      case CTRL('c'):
        saveToClipBoard(*cursor, *select_cursor);
        break;
      case CTRL('a'):
        *select_cursor = tryToReachAbsPosition(*cursor, 1, 0);
        *cursor = tryToReachAbsPosition(*cursor, INT_MAX, INT_MAX);
        break;
      case CTRL('x'):
        saveToClipBoard(*cursor, *select_cursor);
        deleteSelectionWithState(history_frame, cursor, select_cursor, payload_state_change);
        setDesiredColumn(*cursor, desired_column);
        break;
      case CTRL('v'):
        deleteSelectionWithState(history_frame, cursor, select_cursor, payload_state_change);
        tmp = *cursor;
        *cursor = loadFromClipBoard(*cursor);
        saveAction(history_frame, createInsertAction(*cursor, tmp), onStateChangeTS, (long *)&payload_state_change);
        setDesiredColumn(*cursor, desired_column);
        break;
      case CTRL('q'):
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
      case CTRL('w'):
        closeFile(&files, &file_count, &current_file_index, &refresh_ofw, &refresh_edw, &refresh_local_vars);
        break;
      case CTRL('s'):
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


      case H_KEY_CTRL_DELETE:
      case CTRL('H'):
        *cursor = moveToPreviousWord(*cursor);
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveToNextWord(*cursor);
        deleteSelectionWithState(history_frame, cursor, select_cursor, payload_state_change);
        setDesiredColumn(*cursor, desired_column);
        break;
      case '\n':
      case KEY_ENTER:
        deleteSelectionWithState(history_frame, cursor, select_cursor, payload_state_change);
        tmp = *cursor;
        *cursor = insertNewLineInLineC(*cursor);
        saveAction(history_frame, createInsertAction(tmp, *cursor), onStateChangeTS, (long *)&payload_state_change);
        setDesiredColumn(*cursor, desired_column);
        break;
      case H_KEY_DELETE:
        if (isCursorDisabled(*select_cursor)) {
          *select_cursor = moveLeft(*cursor);
        }
        deleteSelectionWithState(history_frame, cursor, select_cursor, payload_state_change);
        setDesiredColumn(*cursor, desired_column);
        break;
      case H_KEY_SUPPR:
        if (isCursorDisabled(*select_cursor)) {
          *select_cursor = moveRight(*cursor);
        }
        deleteSelectionWithState(history_frame, cursor, select_cursor, payload_state_change);
        setDesiredColumn(*cursor, desired_column);
        break;
      case H_KEY_CTRL_SUPPR:
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveToNextWord(*cursor);
        deleteSelectionWithState(history_frame, cursor, select_cursor, payload_state_change);
        setDesiredColumn(*cursor, desired_column);
        break;
      case KEY_TAB:
        deleteSelectionWithState(history_frame, cursor, select_cursor, payload_state_change);
        tmp = *cursor;
        if (TAB_CHAR_USE) {
          *cursor = insertCharInLineC(*cursor, readChar_U8FromInput('\t'));
        }
        else {
          for (int i = 0; i < TAB_SIZE; i++) {
            *cursor = insertCharInLineC(*cursor, readChar_U8FromInput(' '));
          }
        }
        saveAction(history_frame, createInsertAction(tmp, *cursor), onStateChangeTS, (long *)&payload_state_change);
        setDesiredColumn(*cursor, desired_column);
        break;
      case CTRL('d'):
        if (isCursorDisabled(*select_cursor) == true) {
          selectLine(cursor, select_cursor);
        }
        deleteSelectionWithState(history_frame, cursor, select_cursor, payload_state_change);
        setDesiredColumn(*cursor, desired_column);
        break;


      // ---------------------- EDITOR SHORTCUTS ----------------------


      case CTRL('e'): // File Explorer Window Switch
        switchShowFew(&few, &ofw, &ftw, &lnw, &few_width, &saved_few_width, ofw_height, &refresh_few, &refresh_ofw);
        break;
      case CTRL('l'): // Opened File Window Switch
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
      case CTRL(' '): // LSP_completion

        break;


      default:
        if (iscntrl(c)) {
          printf("Unsupported touch %d\r\n", c);
        }
        else {
          deleteSelectionWithState(history_frame, cursor, select_cursor, payload_state_change);
          tmp = *cursor;
          *cursor = insertCharInLineC(*cursor, readChar_U8FromInput(c));
          setDesiredColumn(*cursor, desired_column);
          saveAction(history_frame, createInsertAction(tmp, *cursor), onStateChangeTS, (long *)&payload_state_change);
        }
        break;
    }
    // fprintf(stderr, "----------------------------\n");
    // printByteCount(*root);
    // fprintf(stderr, "File Index : %u\n", getIndexForCursor(*cursor));
    // While end.
  }

end:

  printf("\033[?1003l\n"); // Disable mouse movement events, as l = low
  fflush(stdout);


  if (loaded_settings.is_used == true) {
    WorkspaceSettings new_settings;
    getWorkspaceSettingsForCurrentDir(&new_settings, files, file_count, current_file_index, ofw_height != 0, few_width != 0, FILE_EXPLORER_WIDTH);
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

  // We need to sleep a bit before flush input to wait for the terminal to disable mouse tracking.
  usleep(30000);
  flushinp();
  endwin();
  return 0;
}
