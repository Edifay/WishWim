#include "click_handler.h"

#include <assert.h>
#include <libgen.h>
#include <string.h>

#include "../data-management/file_management.h"
#include "term_handler.h"
#include "../utils/constants.h"


////// -------------- CLICK FUNCTIONS --------------


void handleEditorClick(int edws_offset_x, int edws_offset_y, Cursor* cursor, Cursor* select_cursor, int* desired_column, int* screen_x, int* screen_y, MEVENT* m_event,
                       bool button1_down) {
  if (m_event->y - edws_offset_y < 0) {
    m_event->y = edws_offset_y;
  }


  // ---------- CURSOR ACTION ------------

  if (button1_down) {
    FileIdentifier new_file_id = tryToReachAbsRow(cursor->file_id, *screen_y + m_event->y - edws_offset_y);
    LineIdentifier new_line_id = getLineIdForScreenX(moduloLineIdentifierR(getLineForFileIdentifier(new_file_id), 0), *screen_x, m_event->x - edws_offset_x);

    if (isCursorDisabled(*select_cursor) == false) {
      *cursor = cursorOf(new_file_id, new_line_id);
    }
    else if (areCursorEqual(*cursor, cursorOf(new_file_id, new_line_id)) == false) {
      *select_cursor = *cursor;
      *cursor = cursorOf(new_file_id, new_line_id);
    }
    setDesiredColumn(*cursor, desired_column);
  }

  if (m_event->bstate & BUTTON1_PRESSED) {
    setSelectCursorOff(cursor, select_cursor, SELECT_OFF_RIGHT);
    FileIdentifier new_file_id = tryToReachAbsRow(cursor->file_id, *screen_y + m_event->y - edws_offset_y);
    LineIdentifier new_line_id = getLineIdForScreenX(moduloLineIdentifierR(getLineForFileIdentifier(new_file_id), 0), *screen_x, m_event->x - edws_offset_x);
    *cursor = cursorOf(new_file_id, new_line_id);
    setDesiredColumn(*cursor, desired_column);
  }

  if (m_event->bstate & BUTTON1_DOUBLE_CLICKED) {
    selectWord(cursor, select_cursor);
  }


  // ---------- SCROLL ------------
  if (m_event->bstate & BUTTON4_PRESSED && !(m_event->bstate & BUTTON_SHIFT)) {
    // Move Up
    *screen_y -= SCROLL_SPEED;
    if (*screen_y < 1) *screen_y = 1;
  }
  else if (m_event->bstate & BUTTON4_PRESSED && m_event->bstate & BUTTON_SHIFT) {
    // Move Left
    *screen_x -= SCROLL_SPEED;
    if (*screen_x < 1) *screen_x = 1;
  }

  if (m_event->bstate & BUTTON5_PRESSED && !(m_event->bstate & BUTTON_SHIFT)) {
    // Move Down
    *screen_y += SCROLL_SPEED;
    FileIdentifier last_line_cur = tryToReachAbsRow(cursor->file_id, *screen_y + 2);
    if (*screen_y + 2 != last_line_cur.absolute_row) {
      *screen_y = last_line_cur.absolute_row - 2;
      if (*screen_y < 1) *screen_y = 1;
    }
  }
  else if (m_event->bstate & BUTTON5_PRESSED && m_event->bstate & BUTTON_SHIFT) {
    // Move Right
    *screen_x += SCROLL_SPEED;
  }
}

int handleOpenedFileSelectClick(FileContainer* files, int* file_count, int* current_file, MEVENT m_event, int* current_file_offset, WINDOW* ofw, bool* refresh_ofw) {
  // Char offset for the window
  int current_char_offset = ofw->_begx;

  if (*current_file_offset != 0) {
    current_char_offset += strlen("< | ");
  }

  for (int i = *current_file_offset; i < *file_count; i++) {
    current_char_offset += strlen(basename(files[i].io_file.path_args));

    if (*current_file_offset != 0 && m_event.x < ofw->_begx + 3) {
      (*current_file_offset)--;
      assert(*current_file_offset >= 0);
      *refresh_ofw = true;
      break;
    }

    if (current_char_offset + strlen(FILE_NAME_SEPARATOR) > COLS && m_event.x > COLS - 4) {
      (*current_file_offset)++;
      assert(*current_file_offset < *file_count);
      *refresh_ofw = true;
      break;
    }

    if (m_event.x < current_char_offset + (strlen(FILE_NAME_SEPARATOR) / 2)) {
      // Don't change anything if the file clicked is currently the file opened.
      if (*current_file == i)
        break;

      return i;
    }
    current_char_offset += strlen(FILE_NAME_SEPARATOR);
  }

  return -1;
}


void handleOpenedFileClick(FileContainer* files, int* file_count, int* current_file, MEVENT m_event, int* current_file_offset, WINDOW* ofw, bool* refresh_ofw,
                           bool* refresh_local_vars, bool mouse_drag) {
  if (m_event.bstate & BUTTON4_PRESSED && !(m_event.bstate & BUTTON_SHIFT)) {
    // Move Up
    (*current_file_offset)--;
    if (*current_file_offset < 0)
      *current_file_offset = 0;
    *refresh_ofw = true;
    return;
  }
  if (m_event.bstate & BUTTON5_PRESSED && !(m_event.bstate & BUTTON_SHIFT)) {
    // Move Down
    (*current_file_offset)++;
    if (*current_file_offset > *file_count - 1)
      *current_file_offset = *file_count - 1;
    *refresh_ofw = true;
    return;
  }


  if (m_event.bstate & BUTTON1_PRESSED) {
    int result_ofw_click = handleOpenedFileSelectClick(files, file_count, current_file, m_event, current_file_offset, ofw, refresh_ofw);

    if (result_ofw_click == -1) {
      return;
    }

    *current_file = result_ofw_click;

    *refresh_local_vars = true;
    *refresh_ofw = true;
    return;
  }

  // ReOrder opened Files.
  if (m_event.bstate == NO_EVENT_MOUSE && mouse_drag == true) {
    int result_ofw_click = handleOpenedFileSelectClick(files, file_count, current_file, m_event, current_file_offset, ofw, refresh_ofw);

    if (result_ofw_click == -1 || result_ofw_click == *current_file) {
      return;
    }

    // Swap both files.
    FileContainer tmp = files[result_ofw_click];
    files[result_ofw_click] = files[*current_file];
    files[*current_file] = tmp;

    *current_file = result_ofw_click;

    *refresh_local_vars = true;
    *refresh_ofw = true;
    return;
  }
}

void handleFileExplorerClick(FileContainer** files, int* file_count, int* current_file, ExplorerFolder* pwd, int* few_y_offset, int* few_x_offset, int* few_width,
                             int* few_selected_line, int edws_offset_y, WINDOW** few, WINDOW** ofw, WINDOW** lnw, WINDOW** ftw, MEVENT m_event, bool* refresh_few,
                             bool* refresh_ofw, bool* refresh_edw, bool* refresh_local_vars) {
  // ---------- SCROLL ------------
  if (m_event.bstate & BUTTON4_PRESSED && !(m_event.bstate & BUTTON_SHIFT)) {
    // Move Up
    *few_y_offset -= SCROLL_SPEED;
    if (*few_y_offset < 0) *few_y_offset = 0;
  }
  else if (m_event.bstate & BUTTON4_PRESSED && m_event.bstate & BUTTON_SHIFT) {
    // Move Left
    *few_width -= 1;
    if (*few_width < 0)
      *few_width = 0;
  }

  if (m_event.bstate & BUTTON5_PRESSED && !(m_event.bstate & BUTTON_SHIFT)) {
    // Move Down
    *few_y_offset += SCROLL_SPEED;
  }
  else if (m_event.bstate & BUTTON5_PRESSED && m_event.bstate & BUTTON_SHIFT) {
    // Move Right
    *few_width += 1;
    if (*few_width > COLS - 8)
      *few_width = COLS - 8;
  }

  if (m_event.bstate & BUTTON5_PRESSED || m_event.bstate & BUTTON4_PRESSED) {
    if (m_event.bstate & BUTTON_SHIFT) {
      // Resize File Explorer Window
      delwin(*few);
      *few = newwin(0, *few_width, 0, 0);
      // Resize Opened File Window
      delwin(*ofw);
      *ofw = newwin(edws_offset_y, 0, 0, *few_width);
      *refresh_ofw = true;
      // Resize Editor Window
      resizeEditorWindows(ftw, lnw, edws_offset_y, (*lnw)->_maxx + 1, *few_width);
      *refresh_edw = true;
    }
    *refresh_few = true;
  }

  if (m_event.bstate & BUTTON1_PRESSED) {
    *few_selected_line = *few_y_offset + m_event.y + 1;
    *refresh_few = true;
  }

  if (!(m_event.bstate & BUTTON1_DOUBLE_CLICKED))
    return;

  ExplorerFolder* res_folder;
  int res_index;
  bool found = getFileClickedFileExplorer(pwd, m_event.y, *few_x_offset, *few_y_offset, &res_folder, &res_index);
  *few_selected_line = *few_y_offset + m_event.y + 1;
  // If click on nothing break;
  if (found == false) {
    return;
  }

  if (res_index == -1) {
    // Result is a folder
    // switch open.
    res_folder->open = !res_folder->open;
  }
  else {
    // Result is a file => open file in text editor.
    openNewFile(res_folder->files[res_index].path, files, file_count, current_file, refresh_ofw, refresh_local_vars);
  }

  *refresh_few = true;
}


bool internalGetClickedExplorerFile(ExplorerFolder* current_folder, int* y_click, int few_x_offset, int few_y_offset, ExplorerFolder** res_folder, int* file_index) {
  if (*y_click == 0) {
    *res_folder = current_folder;
    *file_index = -1;
    return true;
  }
  (*y_click)--;

  // Don't check childs if current_folder isn't opened.
  if (current_folder->open == false)
    return false;

  // Check for child folders click.
  for (int i = 0; i < current_folder->folder_count; i++) {
    if (internalGetClickedExplorerFile(current_folder->folders + i, y_click, few_x_offset, few_y_offset, res_folder, file_index) == true) {
      return true;
    }
  }

  // Check for child file click.
  for (int i = 0; i < current_folder->file_count; i++) {
    if (*y_click == 0) {
      *res_folder = current_folder;
      *file_index = i;
      return true;
    }
    (*y_click)--;
  }

  return false;
}

bool getFileClickedFileExplorer(ExplorerFolder* pwd, int y_click, int few_x_offset, int few_y_offset, ExplorerFolder** res_folder, int* file_index) {
  y_click += few_y_offset;
  return internalGetClickedExplorerFile(pwd, &y_click, few_x_offset, few_y_offset, res_folder, file_index);
}
