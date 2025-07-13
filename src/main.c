#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
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

#define SHOW_ERROR true
// #define SHOW_ERROR false


/**   TODO list :
 *       - Patch Makefile highlight, probably an error from the recent refactor for highlight.
 *       - Patch conditional jump (valgrind) when opening untitled file from a workspace.
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


void dispatcher(cJSON* packet, long* payload) {
  if (packet != NULL) {
    fprintf(stderr, "\n\n <<< ================ %s ================\n",
            cJSON_GetStringValue(cJSON_GetObjectItem(packet, "method")));
    cJSON* params = cJSON_GetObjectItem(packet, "params");
    char* text = cJSON_Print(params);
    fprintf(stderr, "%s\n", text);
    free(text);
  }
}

int main(int file_count, char** args) {
  // manage logs
  if (!SHOW_ERROR) {
    FILE* f = fopen("/dev/null", "w");
    dup2(fileno(f), STDERR_FILENO);
    fclose(f);
  }
  else {
    FILE* f = fopen("./logs.txt", "w");
    if (f != NULL) {
      dup2(fileno(f), STDERR_FILENO);
      fclose(f);
    }
  }

  setlocale(LC_ALL, "");
  // TODO Remove when lsp_logs.txt will be unused.
  // system("echo "" > lsp_logs.txt");
  // system("echo "" > tree_logs.txt");

  // remove first args which is the executable file name.
  char** file_names = args;
  file_names++;
  file_count--;

  /// --- Init global vars ---
  // Load config
  config = loadConfig();
  // Parser Datas
  initParserList(&parsers);
  // LSP Datas
  initLSPServerList(&lsp_servers);

  /// --- Init TUI ---
  // Init GUIContext
  GUIContext gui_context;
  initGUIContext(&gui_context);
  // Init terminal with ncurses
  initNCurses(&gui_context);

  /// --- Setup Files ---
  // Containers of current opened buffers.
  FileContainer* files;
  int current_file_index = 0; // The current showed file.
  // Detect workspace settings
  setupWorkspace(&loaded_settings, &file_count, &file_names, &gui_context, &current_file_index);
  // Opening files and setup vars
  setupOpenedFiles(&file_count, file_names, &files);

  /// --- Init Folder Explorer ---
  // Folder used for file explorer.
  ExplorerFolder pwd;
  char* dir_path = getenv("PWD");
  if (dir_path == NULL) dir_path = getenv("HOME");
  initFolder(loaded_settings.is_used == true ? loaded_settings.dir_path : dir_path, &pwd);
  pwd.open = true;


  // Setup redirection of vars to use without pass though file_container obj.
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

  //  --- Begin of the Automaton ---
  WindowHighlightDescriptor highlight_descriptor;
  whd_init(&highlight_descriptor);
  Cursor tmp;
  MEVENT m_event;
  History* old_history_frame;
  PayloadStateChange payload_state_change;
  bool mouse_drag = false;
  time_val last_time_mouse_drag = timeInMilliseconds();
  while (true) {
    //// --------------- Post Processing -----------------

    if (refresh_local_vars == true) {
      setupLocalVars(files, current_file_index, &io_file, &root, &cursor, &select_cursor, &old_cur, &desired_column, &screen_x,
                     &screen_y, &old_screen_x, &old_screen_y,
                     &history_root,
                     &history_frame, &highlight_data);
      refresh_local_vars = false;
      old_history_frame = *history_frame;
      payload_state_change = getPayloadStateChange(highlight_data);
    }

    // flag cursor change
    if (!areCursorEqual(*cursor, *old_cur)) {
      *old_cur = *cursor;
      moveScreenToMatchCursor(gui_context.ftw, *cursor, screen_x, screen_y);
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
      int new_lnw_width = numberOfDigitOfNumber(*screen_y + getmaxy(gui_context.ftw)) + 1 /* +1 for the line */;
      if (new_lnw_width != getbegx(gui_context.ftw)) {
        resizeEditorWindows(&gui_context, new_lnw_width);
      }
    }

    // If it needed to reparse the current file for tree. Looking for state changes.
    if (highlight_data->is_active == true && (old_history_frame != *history_frame || highlight_data->tree == NULL)) {
      // edit_and_parse_tree(root, history_frame, highlight_data, &old_history_frame);
      parseTree(root, history_frame, highlight_data, &old_history_frame);
      optimizeHistory(*history_root, history_frame);
      old_history_frame = *history_frame;
    }

    //// --------------- Paint GUI -----------------

    refresh();

    // Refresh File Explorer Window
    if (gui_context.refresh_few == true && gui_context.few_width != 0 && gui_context.few != NULL) {
      printFileExplorer(&gui_context, &pwd);
      wrefresh(gui_context.few);
      gui_context.refresh_few = false;
    }

    // Refresh File Opened Window
    if ((gui_context.refresh_ofw == true || files[current_file_index].io_file.status == NONE) && gui_context.ofw_height != 0) {
      printOpenedFile(&gui_context, files, file_count, current_file_index);
      wrefresh(gui_context.ofw);
      gui_context.refresh_ofw = false;
    }

    // Refresh Editor Windows
    if (gui_context.refresh_edw == true) {
      whd_reset(&highlight_descriptor);

      // calculate tree_sitter Highlight
      highlightCurrentFile(highlight_data, gui_context.ftw, *screen_x, *screen_y, *cursor, &highlight_descriptor);

      printEditor(&gui_context, *cursor, *select_cursor, *screen_x, *screen_y, &highlight_descriptor);

      wrefresh(gui_context.lnw);
      wrefresh(gui_context.ftw);
    }


    assert(checkFileIntegrity(*root) == true);
    assert(checkByteCountIntegrity(*root) == true);

  read_input:
    int c = getch();
    int hash = c;

    // When available use keyname instead of key_code which is not portable.
    if (c != KEY_MOUSE && c != -1) {
      // fprintf(stderr, "Code %d, Key : '%s' hash into %d.\n", c, keyname(c), hashString(keyname(c)));
      const char* key_str = keyname(c);
      if (key_str != NULL && key_str[0] != '\0') {
        if (key_str[0] != '^') {
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
        assert((getmaxx(gui_context.lnw) + gui_context.few_width >= COLS) == false);
        // Resize Opened File Window
        resizeOpenedFileWindow(&gui_context);
        resizeEditorWindows(&gui_context, getmaxx(gui_context.lnw));
        gui_context.refresh_ofw = gui_context.refresh_edw = gui_context.refresh_few = true;
        break;

      // ---------------------- MOUSE ----------------------

      case KEY_MOUSE:
        if (getmouse(&m_event) != OK) {
          fprintf(stderr, "MOUVE_EVENT_NOT_OK !\r\n");
          exit(0);
          break;
        }

        detectComplexMouseEvents(&m_event);

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

        if ((m_event.x < getbegx(gui_context.lnw) && gui_context.focus_w == NULL) || (
              gui_context.few != NULL && gui_context.focus_w == gui_context.few)) {
          // Click in File Explorer Window
          if (m_event.bstate & BUTTON1_PRESSED) {
            gui_context.focus_w = gui_context.few;
          }
          handleFileExplorerClick(&gui_context, &files, &file_count, &current_file_index, &pwd, m_event, &refresh_local_vars);
        }
        else if ((m_event.y - gui_context.ofw_height < 0 && gui_context.focus_w == NULL) || (
                   gui_context.ofw != NULL && gui_context.focus_w == gui_context.ofw)) {
          // Click on opened file window
          if (m_event.bstate & BUTTON1_PRESSED) {
            gui_context.focus_w = gui_context.ofw;
          }
          handleOpenedFileClick(&gui_context, files, &file_count, &current_file_index, m_event, &refresh_local_vars, mouse_drag);
        }
        else {
          // Click on editor windows
          if (m_event.bstate & BUTTON1_PRESSED) {
            gui_context.focus_w = gui_context.ftw;
          }
          handleEditorClick(&gui_context, cursor, select_cursor, desired_column, screen_x, screen_y, &m_event, mouse_drag);
        }

        if (m_event.bstate & BUTTON1_RELEASED) {
          gui_context.focus_w = NULL;
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
        gui_context.refresh_ofw = true;
        break;
      case H_KEY_CTRL_MAJ_UP:
        // Do something with this.
        if (current_file_index != file_count - 1)
          current_file_index++;
        refresh_local_vars = true;
        // TODO check if the file selected is showing in ofw. If not move it in.
        gui_context.refresh_ofw = true;
        break;
      case H_KEY_BEGIN:
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_LEFT);
        *cursor = goToBegin(*cursor);
        setDesiredColumn(*cursor, desired_column);
        // Next is to go to the first non empty char and not to the begin of the line.
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_RIGHT);
        *cursor = moveToNextWord(*cursor);
        setDesiredColumn(*cursor, desired_column);
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_LEFT);
        *cursor = moveToPreviousWord(*cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case H_KEY_END:
        setSelectCursorOff(cursor, select_cursor, SELECT_OFF_RIGHT);
        *cursor = goToEnd(*cursor);
        setDesiredColumn(*cursor, desired_column);
        break;
      case H_KEY_MAJ_END:
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = goToEnd(*cursor);
        setDesiredColumn(*cursor, desired_column);
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
            setlastFilePosition(files[i].io_file.path_abs, files[i].cursor.file_id.absolute_row,
                                files[i].cursor.line_id.absolute_column, files[i].screen_x, files[i].screen_y);
            saveCurrentStateControl(*files[i].history_root, files[i].history_frame, files[i].io_file.path_abs);
          }
        goto end;
      case CTRL('w'):
        closeFile(&files, &file_count, &current_file_index, &gui_context.refresh_ofw, &gui_context.refresh_edw,
                  &refresh_local_vars);
        break;
      case CTRL('s'):
        if (io_file->status == NONE) {
          printf("\r\nNo specified file\r\n");
          goto end;
        }
        saveFile(*root, io_file);
        assert(io_file->status == EXIST);
        setlastFilePosition(io_file->path_abs, cursor->file_id.absolute_row, cursor->line_id.absolute_column, *screen_x,
                            *screen_y);
        saveCurrentStateControl(**history_root, *history_frame, io_file->path_abs);
        break;


      // ---------------------- FILE EDITING ----------------------


      case H_KEY_CTRL_DELETE:
      case CTRL('H'):
        setSelectCursorOn(*cursor, select_cursor);
        *cursor = moveToPreviousWord(*cursor);
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
        switchShowFew(&gui_context);
        break;
      case CTRL('l'): // Opened File Window Switch
        if (gui_context.ofw_height == OPENED_FILE_WINDOW_HEIGHT) {
          gui_context.ofw_height = 0;
        }
        else {
          gui_context.ofw_height = OPENED_FILE_WINDOW_HEIGHT;
        }
        resizeOpenedFileWindow(&gui_context);
        resizeEditorWindows(&gui_context, getmaxx(gui_context.lnw));
        gui_context.refresh_ofw = true;
        gui_context.refresh_edw = true;
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

    // While end.
  }

end:

  printf("\033[?1003l\n"); // Disable mouse movement events, as l = low
  fflush(stdout);

  whd_free(&highlight_descriptor);

  if (loaded_settings.is_used == true) {
    WorkspaceSettings new_settings;
    getWorkspaceSettingsForCurrentDir(&new_settings, files, file_count, current_file_index, gui_context.ofw_height != 0,
                                      gui_context.few_width != 0, FILE_EXPLORER_WIDTH);
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
