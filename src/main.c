#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include <bits/time.h>

#include "advanced/theme.h"
#include "advanced/tree-sitter/tree_manager.h"
#include "data-management/file_management.h"
#include "data-management/file_structure.h"
#include "data-management/state_control.h"
#include "io_management/io_explorer.h"
#include "io_management/viewport_history.h"
#include "io_management/io_manager.h"
#include "utils/clipboard_manager.h"
#include "utils/key_management.h"
#include "utils/constants.h"
#include "terminal/term_handler.h"
#include "terminal/highlight.h"
#include "terminal/click_handler.h"
#include "../lib/tree-sitter/lib/include/tree_sitter/api.h"
#include "config/config.h"

#define CTRL_KEY(k) ((k)&0x1f)

int color_pair = 2;
int color_index = 100;
cJSON* config;
ParserList parsers;


int main(int file_count, char** file_names) {
  // remove first args which is the executable file name.
  file_names++;
  file_count--;
  setlocale(LC_ALL, "");

  // Load config
  config = loadConfig();

  // Parser Datas
  initParserList(&parsers);

  // Containers of current opened buffers.
  FileContainer* files = malloc(sizeof(FileContainer) * max(1, file_count));
  int current_file = 0; // The current showed file.

  // Folder used for file explorer.
  ExplorerFolder pwd;
  initFolder(getenv("PWD"), &pwd);
  pwd.open = true;

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
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(1001, COLOR_WHITE, COLOR_HOVER);

  // Fill files with args
  for (int i = 0; i < file_count; i++) {
    setupFileContainer(file_names[i], files + i);
  }
  // If no args setup first untitled file.
  if (file_count == 0) {
    file_count = 1;
    setupFileContainer("", files);
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
  FileHighlightDatas* highlight_data;

  bool refresh_local_vars = true; // Need to re-set local vars

  // Start automated-machine
  char* tmp_file_dump = NULL; // used to realloc instead of malloc and free.
  Cursor tmp;
  MEVENT m_event;
  History* old_history_frame;
  bool mouse_drag = false;
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
      int new_lnw_width = numberOfDigitOfNumber(*screen_y + ftw->_maxy) + 1 /* +1 for the line */;
      if (new_lnw_width != ftw->_begx) {
        resizeEditorWindows(&ftw, &lnw, ofw_height, new_lnw_width, few_width);
      }
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

      ParserContainer* parser = getParserForLanguage(&parsers, highlight_data->lang_name);

      double time_taken;
      double inter_time_taken = 0;

      // If highlight is enable on this file.
      if (highlight_data->is_active == true) {
        clock_t t;
        t = clock();
        // If it needed to reparse the current file. Looking for state changes.
        if (old_history_frame != *history_frame || highlight_data->tree == NULL) {
          //printf("Parse !\r\n");
          clock_t inter_t;
          inter_t = clock();

          int relative_loc = 0;

          History* current_hist = *history_frame;
          while (current_hist != NULL && current_hist != old_history_frame) {
            current_hist = current_hist->next;
            relative_loc++;
          }
          if (current_hist == NULL) {
            relative_loc = 0;
          }
          // If it wasn't found on the right.
          if (relative_loc == 0) {
            current_hist = *history_frame;
            while (current_hist != NULL && current_hist != old_history_frame) {
              current_hist = current_hist->prev;
              relative_loc--;
            }
            // Wasn't found on the right AND on the left.
            if (current_hist == NULL) {
              printf("File state problem.\r\n");
              exit(0);
            }
          }

          // printf("Rela%dtive_loc \r\n", relative_loc);

          ActionImprovedWithBytes improved_history[abs(relative_loc)];
          current_hist = *history_frame;
          for (int i = 0; i < abs(relative_loc); i++) {
            improved_history[i].history_frame = current_hist;
            improved_history[i].byte_start = -1;
            improved_history[i].byte_end = -1;
            if (relative_loc > 0) {
              current_hist = current_hist->next;
            }
            else if (relative_loc < 0) {
              current_hist = current_hist->prev;
            }
          }

          relative_loc = -relative_loc;


          int n_bytes = 0;
          FileNode* current_file_node = *root;
          int relative_file = 0;
          int abs_file = 1;
          while (current_file_node != NULL) {
            if (relative_file == current_file_node->element_number) {
              current_file_node = current_file_node->next;
              relative_file = 0;
              continue;
            }
            n_bytes++;
            LineNode* current_line_node = current_file_node->lines + relative_file;

            // Cur at begin of the line representing none char.
            for (int i = 0; i < abs(relative_loc); i++) {
              switch (improved_history[i].history_frame->action.action) {
                case ACTION_NONE:
                  break;
                case INSERT:
                  if (improved_history[i].history_frame->action.cur_end.file_id.absolute_row == abs_file
                      && improved_history[i].history_frame->action.cur_end.line_id.absolute_column == 0) {
                    improved_history[i].byte_end = n_bytes;
                  }
                case DELETE:
                case DELETE_ONE:
                  if (improved_history[i].history_frame->action.cur.file_id.absolute_row == abs_file
                      && improved_history[i].history_frame->action.cur.line_id.absolute_column == 0) {
                    improved_history[i].byte_start = n_bytes;
                  }
                  break;
              }
            }

            int relative_line = 0;
            int abs_line = 1;
            while (current_line_node != NULL) {
              if (relative_line == current_line_node->element_number) {
                current_line_node = current_line_node->next;
                relative_line = 0;
                continue;
              }
              n_bytes += sizeChar_U8(current_line_node->ch[relative_line]);
              // Cursor representing chars.
              for (int i = 0; i < abs(relative_loc); i++) {
                switch (improved_history[i].history_frame->action.action) {
                  case ACTION_NONE:
                    break;
                  case INSERT:
                    if (improved_history[i].history_frame->action.cur_end.file_id.absolute_row == abs_file
                        && improved_history[i].history_frame->action.cur_end.line_id.absolute_column == abs_line) {
                      improved_history[i].byte_end = n_bytes;
                    }
                  case DELETE:
                  case DELETE_ONE:
                    if (improved_history[i].history_frame->action.cur.file_id.absolute_row == abs_file
                        && improved_history[i].history_frame->action.cur.line_id.absolute_column == abs_line) {
                      improved_history[i].byte_start = n_bytes;
                      // printf("MATCH ON [%d, %d]\r\n", abs_file, abs_line);
                    }
                    break;
                }
              }
              relative_line++;
              abs_line++;
            }

            relative_file++;
            abs_file++;
          }

          // printf("Size :%d\r\n", n_bytes);
          tmp_file_dump = realloc(tmp_file_dump, n_bytes);

          current_file_node = *root;
          relative_file = 0;
          int current_index = 0;
          while (current_file_node != NULL) {
            if (relative_file == current_file_node->element_number) {
              current_file_node = current_file_node->next;
              relative_file = 0;
              continue;
            }
            LineNode* current_line_node = current_file_node->lines + relative_file;

            int relative_line = 0;
            while (current_line_node != NULL) {
              if (relative_line == current_line_node->element_number) {
                current_line_node = current_line_node->next;
                relative_line = 0;
                continue;
              }
              for (int i = 0; i < sizeChar_U8(current_line_node->ch[relative_line]); i++) {
                tmp_file_dump[current_index++] = current_line_node->ch[relative_line].t[i];
              }
              relative_line++;
            }

            relative_file++;
            tmp_file_dump[current_index++] = '\n';
          }


          // TODO implement ts_tree_edit using state_control to get the action dones.
          assert(abs(relative_loc) <= 1);
          for (int i = 0; i < abs(relative_loc); i++) {
            if (relative_loc > 0) {
              TSInputEdit edit;
              switch (improved_history[i].history_frame->action.action) {
                case INSERT:
                  assert(improved_history[i].byte_start != -1);
                  assert(improved_history[i].byte_end != -1);
                  edit.start_byte = improved_history[i].byte_start;
                  edit.start_point.row = improved_history[i].history_frame->action.cur.file_id.absolute_row;
                  edit.start_point.column = improved_history[i].history_frame->action.cur.line_id.absolute_column;

                  edit.old_end_byte = improved_history[i].byte_start;
                  edit.old_end_point.row = edit.start_point.row;
                  edit.old_end_point.column = edit.start_point.column;

                  edit.new_end_byte = improved_history[i].byte_end;
                  edit.new_end_point.row = improved_history[i].history_frame->action.cur_end.file_id.absolute_row;
                  edit.new_end_point.column = improved_history[i].history_frame->action.cur_end.line_id.absolute_column;
                  ts_tree_edit(highlight_data->tree, &edit);
                  break;
                case DELETE:
                  assert(improved_history[i].byte_start != -1);
                  edit.start_byte = improved_history[i].byte_start;
                  edit.start_point.row = improved_history[i].history_frame->action.cur.file_id.absolute_row;
                  edit.start_point.column = improved_history[i].history_frame->action.cur.line_id.absolute_column;

                  int ch_len = strlen(improved_history[i].history_frame->action.ch);
                  edit.old_end_byte = improved_history[i].byte_start + ch_len;

                  char* ch = improved_history[i].history_frame->action.ch;
                  int current_row = edit.start_point.row;
                  int current_column = edit.start_point.column;

                  int current_ch_index = 0;
                  while (current_ch_index < ch_len) {
                    if (TAB_CHAR_USE == false) {
                      assert(ch[current_ch_index] != '\t');
                    }
                    if (ch[current_ch_index] == '\n') {
                      current_row++;
                      current_column = 0;
                    }
                    else {
                      Char_U8 tmp_ch = readChar_U8FromCharArray(ch + current_ch_index);
                      current_ch_index += sizeChar_U8(tmp_ch) - 1;
                      current_column++;
                    }
                    current_ch_index++;
                  }


                  edit.old_end_point.row = current_row;
                  edit.old_end_point.column = current_column;

                  edit.new_end_byte = improved_history[i].byte_start;
                  edit.new_end_point.row = improved_history[i].history_frame->action.cur.file_id.absolute_row;
                  edit.new_end_point.column = improved_history[i].history_frame->action.cur.line_id.absolute_column;
                  ts_tree_edit(highlight_data->tree, &edit);
                  break;
                case DELETE_ONE:
                  assert(improved_history[i].byte_start != -1);
                  edit.start_byte = improved_history[i].byte_start;
                  edit.start_point.row = improved_history[i].history_frame->action.cur.file_id.absolute_row;
                  edit.start_point.column = improved_history[i].history_frame->action.cur.line_id.absolute_column;

                  edit.old_end_byte = improved_history[i].byte_start + 1;
                  edit.old_end_point.row = edit.start_point.row;
                  edit.old_end_point.column = edit.start_point.column;
                  if (improved_history[i].history_frame->action.unique_ch == '\n') {
                    edit.old_end_point.row++;
                    edit.old_end_point.column = 0;
                  }
                  else {
                    edit.old_end_point.column++;
                  }

                  edit.new_end_byte = improved_history[i].byte_start;
                  edit.new_end_point.row = improved_history[i].history_frame->action.cur.file_id.absolute_row;
                  edit.new_end_point.column = improved_history[i].history_frame->action.cur.line_id.absolute_column;
                  ts_tree_edit(highlight_data->tree, &edit);
                  break;
                case ACTION_NONE:
                  break;
              }
            }
            else {
              // TODO implement undo.
              assert(false);
            }
          }

          // TSInputEdit edit;
          // ts_tree_edit(highlight_data->tree, )
          TSTree* old_tree = highlight_data->tree;
          highlight_data->tree = ts_parser_parse_string(
            parser->parser,
            highlight_data->tree,
            tmp_file_dump,
            n_bytes
          );
          ts_tree_delete(old_tree);

          inter_t = clock() - inter_t;
          inter_time_taken = ((double)inter_t) / CLOCKS_PER_SEC; // in seconds

          // printf("fun() took %f seconds to parse \n\r", time_taken);

          // exit(0);
          old_history_frame = *history_frame;
        }

        TSNode root_node = ts_tree_root_node(highlight_data->tree);

        TreePath path[100];
        assert(path != NULL);


        long* args_fct = malloc(11 * sizeof(long *));
        args_fct[0] = (long)&parser->highlight_queries;
        args_fct[1] = (long)&parser->theme_list;
        args_fct[2] = (long)tmp_file_dump;
        args_fct[3] = (long)ftw;
        args_fct[4] = (long)screen_x;
        args_fct[5] = (long)screen_y;
        args_fct[6] = (long)ftw->_maxx;
        args_fct[7] = (long)ftw->_maxy;
        args_fct[8] = (long)cursor;
        args_fct[9] = (long)select_cursor;
        args_fct[10] = (long)NULL;


        treeForEachNodeSized(*screen_y, *screen_x, ftw->_maxy + 1, ftw->_maxx + 1, root_node, path, 0, checkMatchForHighlight, args_fct);
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
    switch (c) {
      // ---------------------- NCURSES THINGS ----------------------

      case ERR:
        // TODO Here check to do background operation.
        goto read_input;

      case BEGIN_MOUSE_LISTEN:
      case MOUSE_IN_OUT:
      case KEY_RESIZE:
        // Avoid biggest size only used on time before automated resize.
        if (lnw->_maxx + few_width + 1 >= COLS)
          break;
      // Resize Opened File Window
        resizeOpenedFileWindow(&ofw, &refresh_ofw, ofw_height, few_width);
        refresh_ofw = true;
        resizeEditorWindows(&ftw, &lnw, ofw_height, lnw->_maxx + 1, few_width);
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


          // If pressed enable drag
          if (m_event.bstate & BUTTON1_PRESSED) {
            mouse_drag = true;
          }

          if ((m_event.x < lnw->_begx && focus_w == NULL) || (few != NULL && focus_w == few)) {
            // Click in File Explorer Window
            if (m_event.bstate & BUTTON1_PRESSED) {
              focus_w = few;
            }
            handleFileExplorerClick(&files, &file_count, &current_file, &pwd, &few_y_offset, &few_x_offset, &few_width, &few_selected_line, ofw_height, &few, &ofw, &lnw, &ftw,
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
            handleEditorClick(ftw->_begx, ofw_height, cursor, select_cursor, desired_column, screen_x, screen_y, &m_event, mouse_drag);
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


      // ---------------------- EDITOR SHORTCUTS ----------------------


      case CTRL_KEY('e'): // File Explorer Window Switch
        if (few_width == 0) {
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
          saved_few_width = few->_maxx + 1;
          delwin(few);
          few_width = 0;
        }
      // Resize Opened File Window
        resizeOpenedFileWindow(&ofw, &refresh_ofw, ofw_height, few_width);
      // Resize Editor Window
        resizeEditorWindows(&ftw, &lnw, ofw_height, lnw->_maxx + 1, few_width);
        break;
      case CTRL_KEY('l'): // Opened File Window Switch
        if (ofw_height == OPENED_FILE_WINDOW_HEIGHT) {
          ofw_height = 0;
        }
        else {
          ofw_height = OPENED_FILE_WINDOW_HEIGHT;
        }
        resizeOpenedFileWindow(&ofw, &refresh_ofw, ofw_height, few_width);
        resizeEditorWindows(&ftw, &lnw, ofw_height, lnw->_maxx + 1, few_width);
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

  endwin();
  // Destroy all files
  for (int i = 0; i < file_count; i++) {
    destroyFileContainer(files + i);
  }
  destroyFolder(&pwd);
  free(files);
  // TODO change
  free(tmp_file_dump);
  cJSON_Delete(config);
  destroyParserList(&parsers);
  return 0;
}
