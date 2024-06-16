#include <assert.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>

#include "file_management.h"
#include "term_handler.h"
#include "../utils/constants.h"


////// -------------- PRINT FUNCTIONS --------------


void printChar_U8ToNcurses(WINDOW* w, Char_U8 ch) {
  int size = sizeChar_U8(ch);
  for (int i = 0; i < size; i++) {
    wprintw(w, "%c", ch.t[i]);
  }
}

void printEditor(WINDOW* ftw, WINDOW* lnw, WINDOW* ofw, Cursor cursor, Cursor select_cursor, int screen_x, int screen_y) {
  wmove(ftw, 0, 0);
  FileIdentifier file_cur = cursor.file_id;

  int current_lines = ftw->_maxy + 1;
  int current_columns = ftw->_maxx + 1;

  // print text
  for (int row = screen_y; row < screen_y + current_lines; row++) {
    // getting the row to print.
    file_cur = tryToReachAbsRow(file_cur, row);

    // if the row is couldn't be reached.
    if (file_cur.absolute_row != row) {
      wmove(lnw, row - screen_y, 0);
      for (int i = 0; i < lnw->_maxx + 1; i++) {
        wprintw(lnw, " ");
      }

      wprintw(ftw, "~\n");

      continue;
    }


    // Print line number
    char line_number[40];
    sprintf(line_number, "%d", file_cur.absolute_row);
    int lineNumberSize = strlen(line_number);

    if (file_cur.absolute_row != cursor.file_id.absolute_row)
      wattron(lnw, A_DIM);
    else
      wattron(lnw, A_BOLD);

    wmove(lnw, row - screen_y, 0);
    for (int i = 0; i < lnw->_maxx - lineNumberSize; i++) {
      wprintw(lnw, " ");
    }
    wprintw(lnw, "%s", line_number);
    if (file_cur.absolute_row == cursor.file_id.absolute_row)
      wprintw(lnw, "🭳");
    else
      wprintw(lnw, "🭵");

    if (file_cur.absolute_row != cursor.file_id.absolute_row)
      wattroff(lnw, A_DIM);
    else
      wattroff(lnw, A_BOLD);


    // Print line chars

    LineIdentifier begin_screen_line_cur = tryToReachAbsColumn(moduloLineIdentifierR(getLineForFileIdentifier(file_cur), 0), screen_x);
    LineIdentifier end_screen_line_cur = tryToReachAbsColumn(begin_screen_line_cur, screen_x + current_columns - 3);


    int column = screen_x;
    while (end_screen_line_cur.absolute_column >= screen_x && begin_screen_line_cur.absolute_column <= end_screen_line_cur.absolute_column && screen_x + current_columns - 3 >=
           column) {
      Char_U8 ch = getCharForLineIdentifier(begin_screen_line_cur);

      int size = charPrintSize(ch);
      // If the char is detected as not printable char.
      if (size == 0 || size == -1) {
        ch = readChar_U8FromCharArray("�");
        size = 1;
      }

      // If a char of size 2 is at the end of the line replace it by '_' to avoid line overflow.
      if (size >= 2 && screen_x + current_columns - 4 < column) {
        ch = readChar_U8FromCharArray("_");
        // size = 1;
      }

      // determine if the char is selected or not.
      bool selected_style = isCursorDisabled(select_cursor) == false && isCursorBetweenOthers(cursorOf(file_cur, begin_screen_line_cur), select_cursor, cursor);

      if (selected_style)
        wattron(ftw, A_STANDOUT|A_DIM);

      if (ch.t[0] == '\t') {
        for (int i = 0; i < TAB_SIZE; i++) {
          Char_U8 space = readChar_U8FromInput(' ');
          printChar_U8ToNcurses(ftw, space);
        }
      }
      else {
        printChar_U8ToNcurses(ftw, ch);
      }

      if (selected_style)
        wattroff(ftw, A_STANDOUT|A_DIM);

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
        wattron(ftw, A_STANDOUT|A_DIM);
        wprintw(ftw, " ");
        wattroff(ftw, A_STANDOUT|A_DIM);
      }
    }

    // If the line is not fully display show '>'
    if (hasElementAfterLine(end_screen_line_cur)) {
      wattron(ftw, A_BOLD|A_UNDERLINE|A_DIM);
      wprintw(ftw, ">");
      wattroff(ftw, A_BOLD|A_UNDERLINE|A_DIM);
    }

    wprintw(ftw, "\n");
  }

  // Check if cursor is in the screen and print it if needed.
  if (cursor.file_id.absolute_row >= screen_y && cursor.file_id.absolute_row < screen_y + current_lines
      && cursor.line_id.absolute_column >= screen_x - 1 && cursor.line_id.absolute_column <= screen_x + current_columns - 3) {
    int x = getScreenXForCursor(cursor, screen_x);
    wmove(ftw, cursor.file_id.absolute_row - screen_y, x);

    char size = 1;
    if (hasElementAfterLine(cursor.line_id) == true) {
      // Check the size of the the char which is under cursor.
      Cursor tmp = moveRight(cursor);
      size = charPrintSize(getCharForLineIdentifier(tmp.line_id));
    }

    wchgat(ftw, size, A_STANDOUT, 0, NULL);
  }
  // box(ofw, 0, 0);
}

void printOpenedFile(FileContainer* files, int file_count, int current_file, int current_file_offset, WINDOW* ofw) {
  // The current position of the cursor for the first line.
  wmove(ofw, 0, 0);
  int current_offset = ofw->_begx;
  if (current_file_offset != 0) {
    current_offset += strlen("< | ");
    wattron(ofw, A_DIM);
    wprintw(ofw, "< | ");
    wattroff(ofw, A_DIM);
  }
  // Move to the top left corner.
  for (int i = current_file_offset; i < file_count; i++) {
    // Style file names.
    if (i == current_file)
      wattron(ofw, A_BOLD);
    else
      wattron(ofw, A_DIM);
    // Print file name
    char* file_name = basename(files[i].io_file.path_args);
    current_offset += strlen(file_name);
    wprintw(ofw, "%s", file_name);
    // Style file names.
    if (i == current_file)
      wattroff(ofw, A_BOLD);
    else
      wattroff(ofw, A_DIM);
    // Print file name separator
    if (i < file_count - 1) {
      wprintw(ofw, FILE_NAME_SEPARATOR);
      current_offset += strlen(FILE_NAME_SEPARATOR);
    }
    // If the file names overflow the line, print the move right text.
    if (current_offset + strlen(FILE_NAME_SEPARATOR) > COLS) {
      wmove(ofw, 0, COLS - ofw->_begx - strlen("... | >"));
      wattron(ofw, A_DIM);
      wprintw(ofw, "... | >");
      wattroff(ofw, A_DIM);
      // assert((i < argc && i < max_opened_file + 1) == false);
      break;
    }
  }
  // To erase the end of the line to avoid garbage on scroll to right.
  for (int i = current_offset; i < COLS; i++) {
    wprintw(ofw, " ");
  }
  // Print the bottom line.
  wmove(ofw, 1, 0);
  for (int i = 0; i < COLS; i++) {
    wprintw(ofw, "🭸");
  }
}


void internalPrintExplorerRec(ExplorerFolder* folder, WINDOW* few, int* few_x_offset, int* few_y_offset, int tree_offset_rec, int* selected_line) {
  // Don't print if not in window.
  if (few->_cury >= few->_maxy) return;

  if (folder->open && folder->discovered == false) {
    discoverFolder(folder);
  }

  (*selected_line)--;
  if (*few_y_offset == 0) {
    // Print current folder name

    if (*selected_line == 0) {
      wattron(few, A_STANDOUT);
    }

    for (int i = 0; i < tree_offset_rec; i++) {
      printToNcursesNCharFromString(few, " ", few->_maxx - few->_curx);
    }

    // Print decoration of folder. The decoration describe if the folder is open or not.
    if (folder->open) printToNcursesNCharFromString(few, "⌄", few->_maxx - few->_curx);
    else printToNcursesNCharFromString(few, "›", few->_maxx - few->_curx);

    printToNcursesNCharFromString(few, "📁", few->_maxx - few->_curx);
    printToNcursesNCharFromString(few, basename(folder->path), few->_maxx - few->_curx);
    if (*selected_line == 0) {
      for (int j = few->_curx; j < few->_maxx; j++) {
        printToNcursesNCharFromString(few, " ", few->_maxx - few->_curx);
      }
      wattroff(few, A_STANDOUT);
    }
    wprintw(few, "\n");
  }
  else {
    (*few_y_offset)--;
  }

  if (folder->open == false)
    return;

  // Print sub folders
  for (int i = 0; i < folder->folder_count; i++) {
    internalPrintExplorerRec(folder->folders + i, few, few_x_offset, few_y_offset, tree_offset_rec + FILE_EXPLORER_TREE_OFFSET, selected_line);
  }
  // Print sub files
  for (int i = 0; i < folder->file_count; i++) {
    // Don't print if not in window.
    if (few->_cury >= few->_maxy) return;

    (*selected_line)--;
    if (*few_y_offset == 0) {
      // Print file name
      if (*selected_line == 0) {
        wattron(few, A_STANDOUT);
      }
      for (int j = 0; j < tree_offset_rec + FILE_EXPLORER_TREE_OFFSET + 1/*Add one to balance with the folder decoration*/; j++) {
        printToNcursesNCharFromString(few, " ", few->_maxx - few->_curx);
      }
      printToNcursesNCharFromString(few, "📄", few->_maxx - few->_curx);
      printToNcursesNCharFromString(few, basename(folder->files[i].path), few->_maxx - few->_curx);
      if (*selected_line == 0) {
        for (int j = few->_curx; j < few->_maxx; j++) {
          printToNcursesNCharFromString(few, " ", few->_maxx - few->_curx);
        }
        wattroff(few, A_STANDOUT);
      }
      wprintw(few, "\n");
    }
    else {
      (*few_y_offset)--;
    }
  }
}

void printFileExplorer(ExplorerFolder* pwd, WINDOW* few, int few_x_offset, int few_y_offset, int selected_line) {
  wmove(few, 0, 0);

  internalPrintExplorerRec(pwd, few, &few_x_offset, &few_y_offset, 0, &selected_line);
  // Clear end of window
  for (int i = few->_cury; i < few->_maxy + 1; i++) {
    wprintw(few, "\n");
  }
  for (int i = few->_begy; i < few->_maxy + 1; i++) {
    mvwprintw(few, i, few->_maxx + 1 - 1, "│");
  }
}


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

int handleOpenedFileSelectClick(FileContainer* files, int* file_count, int* current_file, MEVENT m_event, int* current_file_offset, WINDOW* ofw, bool *refresh_ofw) {
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


////// -------------- RESIZE FUNCTIONS --------------

void resizeEditorWindows(WINDOW** ftw, WINDOW** lnw, int y_file_editor, int lnw_width, int few_width) {
  delwin(*ftw);
  delwin(*lnw);
  *ftw = newwin(0, 0, y_file_editor, lnw_width + few_width);
  *lnw = newwin(0, lnw_width, y_file_editor, few_width);
}

void resizeOpenedFileWindow(WINDOW** ofw, bool* refresh_ofw, int edws_offset_y, int few_width) {
  delwin(*ofw);
  *ofw = newwin(edws_offset_y, 0, 0, few_width);
  *refresh_ofw = true;
}


////// -------------- UTILS FUNCTIONS --------------


void moveScreenToMatchCursor(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y) {
  int current_lines = w->_maxy;
  int current_columns = w->_maxx;

  if (cursor.file_id.absolute_row - (*screen_y + current_lines) + 1 >= 0) {
    *screen_y = cursor.file_id.absolute_row - current_lines + 2;
    if (*screen_y < 1) *screen_y = 1;
  }
  else if (cursor.file_id.absolute_row < *screen_y + 1) {
    *screen_y = cursor.file_id.absolute_row - 1;
    if (*screen_y < 1) *screen_y = 1;
  }

  int screen_x_wide_char = getScreenXForCursor(cursor, *screen_x) + *screen_x;
  if (screen_x_wide_char - (*screen_x + current_columns - 8) >= 0) {
    *screen_x = screen_x_wide_char - current_columns + 8;
    if (*screen_x < 1) *screen_x = 1;
  }
  else if (screen_x_wide_char - 5 < *screen_x) {
    *screen_x = screen_x_wide_char - 5;
    if (*screen_x < 1) {
      *screen_x = 1;
    }
  }
}

void centerCursorOnScreen(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y) {
  // center for y, but right for x.
  *screen_x = cursor.line_id.absolute_column - (COLS /*/ 2*/);
  *screen_y = cursor.file_id.absolute_row - (LINES / 2);

  if (*screen_x < 1)
    *screen_x = 1;
  if (*screen_y < 1)
    *screen_y = 1;

  // To match right for x.
  moveScreenToMatchCursor(w, cursor, screen_x, screen_y);
}


int getScreenXForCursor(Cursor cursor, int screen_x) {
  Cursor initial = cursor;
  Cursor old_cursor = cursor;
  int atAdd = 0;
  int size;


  if (cursor.line_id.absolute_column != 0 && (size = charPrintSize(getCharForLineIdentifier(cursor.line_id))) >= 2) {
    atAdd += size - 1;
  }
  cursor = moveLeft(cursor);


  while (screen_x <= cursor.line_id.absolute_column && areCursorEqual(cursor, old_cursor) == false
         && cursor.file_id.absolute_row == old_cursor.file_id.absolute_row) {
    assert(cursor.line_id.absolute_column != 0);
    Char_U8 current_ch = getCharForLineIdentifier(cursor.line_id);
    if ((size = charPrintSize(current_ch)) >= 2) {
      atAdd += size - 1;
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
