#include "file_management.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "../io_management/viewport_history.h"
#include "../utils/constants.h"
#include "../terminal/term_handler.h"


////// -------------- FILE CONTAINER --------------


void destroyFileContainer(FileContainer* container) {
  if (container->io_file.status == EXIST) {
    setlastFilePosition(container->io_file.path_abs, container->cursor.file_id.absolute_row, container->cursor.line_id.absolute_column, container->screen_x, container->screen_y);
  }
  destroyFullFile(container->root);
  destroyEndOfHistory(container->history_root);
  free(container->history_root);
  ts_tree_delete(container->highlight_data.tree);
  free(container->highlight_data.tmp_file_dump);
}

void openNewFile(char* file_path, FileContainer** files, int* file_count, int* current_file, bool* refresh_ofw, bool* refresh_local_vars) {
  // Update editors vars
  *refresh_local_vars = true;
  *refresh_ofw = true;

  char abs_path[PATH_MAX];
  char* res = realpath(file_path, abs_path);
  // Check for an existing buffer.
  if (res != NULL) {
    for (int i = 0; i < *file_count; i++) {
      if ((*files)[i].io_file.status != NONE && strcmp((*files)[i].io_file.path_abs, abs_path) == 0) {
        *current_file = i;
        return;
      }
    }
  }

  // Reallocate files to handle one more file.
  (*file_count)++;
  *files = realloc(*files, *file_count * sizeof(FileContainer));

  // Setup new file container with clicked file
  setupFileContainer(file_path, *files + *file_count - 1);

  if ((*files)[*file_count - 1].lsp_datas.is_enable) {
    char* dump = dumpSelection(tryToReachAbsPosition((*files)[*file_count - 1].cursor, 1, 0), tryToReachAbsPosition((*files)[*file_count - 1].cursor, INT_MAX, INT_MAX));
    LSP_notifyLspFileDidOpen(*getLSPServerForLanguage(&lsp_servers, (*files)[*file_count - 1].lsp_datas.name), (*files)[*file_count - 1].io_file.path_args, dump);
    free(dump);
  }

  *current_file = *file_count - 1;
}

void closeFile(FileContainer** files, int* file_count, int* current_file, bool* refresh_ofw, bool* refresh_edw, bool* refresh_local_vars) {
  if (*file_count == 1) // Always need to keep one file.
  {
    return;
  }
  // Clear current file
  destroyFileContainer(*files + *current_file);
  memmove(*files + *current_file, *files + *current_file + 1, (*file_count - *current_file - 1) * sizeof(FileContainer));

  // Change vars
  if (*current_file != 0)
    (*current_file)--;
  (*file_count)--;

  // realloc files to avoid excess of mem.
  *files = realloc(*files, *file_count * sizeof(FileContainer));
  *refresh_ofw = true;
  *refresh_edw = true;
  *refresh_local_vars = true;
}

Cursor createRoot(IO_FileID file) {
  if (file.status == EXIST) {
    return initWrittableFileFromFile(file.path_abs);
  }
  return initNewWrittableFile();
}

void setupFileContainer(char* path, FileContainer* container) {
  setupFile(path, &container->io_file);
  container->screen_x = 1;
  container->screen_y = 1;
  container->old_screen_x = -1;
  container->old_screen_y = -1;
  container->history_root = malloc(sizeof(History));
  container->history_frame = container->history_root;

  container->cursor = createRoot(container->io_file);
  container->select_cursor = disableCursor(container->cursor);
  setDesiredColumn(container->cursor, &container->desired_column);

  container->root = container->cursor.file_id.file;
  assert(container->root->prev == NULL);
  fetchSavedCursorPosition(container->io_file, &container->cursor, &container->screen_x, &container->screen_y);
  loadCurrentStateControl(container->history_root, &container->history_frame, container->io_file);

  setFileHighlightDatas(&container->highlight_data, container->io_file);
  setLspDatas(&container->lsp_datas, container->io_file);
}


void setupLocalVars(FileContainer* files, int current_file, IO_FileID** io_file, FileNode*** root, Cursor** cursor, Cursor** select_cursor, Cursor** old_cur, int** desired_column,
                    int** screen_x, int** screen_y, int** old_screen_x, int** old_screen_y, History*** history_root, History*** history_frame,
                    FileHighlightDatas** highlight_data) {
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
  *highlight_data = &files[current_file].highlight_data;
  **old_screen_y = -1;
}


bool isFileContainerEmpty(FileContainer* container) {
  if (container->io_file.status != NONE)
    return false;

  return container->cursor.file_id.absolute_row == 1 && container->cursor.line_id.absolute_column == 0
         && areCursorEqual(container->cursor, moveRight(container->cursor));
}

//// -------------- CURSOR MANAGEMENT --------------

Cursor moveRight(Cursor cursor) {
  if (hasElementAfterLine(cursor.line_id)) {
    cursor.line_id.relative_column++;
    cursor.line_id.absolute_column++;
    cursor = moduloCursor(cursor);
  }
  else if (hasElementAfterFile(cursor.file_id)) {
    // If cursor is at the end of the line try to go on next line.
    cursor.file_id.relative_row++;
    cursor.file_id.absolute_row++;
    cursor = cursorOf(cursor.file_id, moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), 0));
  }

  return cursor;
}

Cursor moveLeft(Cursor cursor) {
  if (hasElementBeforeLine(cursor.line_id)) {
    cursor.line_id.relative_column--;
    cursor.line_id.absolute_column--;
    cursor = moduloCursor(cursor);
  }
  else if (hasElementBeforeFile(cursor.file_id)) {
    // If the cursor is at the begin of the line try to reach previous line.
    cursor.file_id.relative_row--;
    cursor.file_id.absolute_row--;
    cursor = cursorOf(cursor.file_id, moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), sizeLineNode(getLineForFileIdentifier(cursor.file_id))));
  }

  return cursor;
}

Cursor moveUp(Cursor cursor, int desiredColumn) {
  if (hasElementBeforeFile(cursor.file_id)) {
    cursor.file_id.relative_row--;
    cursor.file_id.absolute_row--;
    cursor = cursorOf(cursor.file_id, tryToReachAbsColumn(moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), 0), desiredColumn));
  }

  return cursor;
}

Cursor moveDown(Cursor cursor, int desiredColumn) {
  if (hasElementAfterFile(cursor.file_id)) {
    cursor.file_id.relative_row++;
    cursor.file_id.absolute_row++;
    cursor = cursorOf(cursor.file_id, tryToReachAbsColumn(moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), 0), desiredColumn));
  }

  return cursor;
}

Cursor deleteCharAtCursor(Cursor cursor) {
  if (cursor.line_id.absolute_column == 0) {
    if (cursor.file_id.absolute_row != 1) {
      cursor = concatNeighbordsLinesC(cursor);
    }
  }
  else {
    cursor = removeCharInLineC(cursor);
  }
  return cursor;
}

Cursor supprCharAtCursor(Cursor cursor) {
  Cursor old_cur = cursor;
  cursor = moveRight(cursor);
  if (old_cur.file_id.absolute_row != cursor.file_id.absolute_row || old_cur.line_id.absolute_column !=
      cursor.line_id.absolute_column) {
    cursor = deleteCharAtCursor(cursor);
  }
  return cursor;
}

Cursor deleteLineAtCursor(Cursor cursor) {
  if (cursor.file_id.absolute_row != 1 || hasElementAfterFile(cursor.file_id)) {
    cursor = removeLineInFileC(cursor);
  }
  else {
    destroyFullLine(cursor.line_id.line);
    cursor.line_id = moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), 0);
  }
  return cursor;
}


Cursor skipRightInvisibleChar(Cursor cursor) {
  Cursor tmp_cur;
  while (hasElementAfterLine(cursor.line_id) && isInvisible(getCharForLineIdentifier((tmp_cur = moveRight(cursor)).line_id))) {
    cursor = tmp_cur;
  }

  return cursor;
}

Cursor skipLeftInvisibleChar(Cursor cursor) {
  while (cursor.line_id.absolute_column != 0 && isInvisible(getCharForLineIdentifier(cursor.line_id))) {
    cursor = moveLeft(cursor);
  }

  return cursor;
}

Cursor moveToNextWord(Cursor cursor) {
  cursor = skipRightInvisibleChar(cursor);
  Cursor old_cur = cursor;

  Char_U8 repeated;
  if (hasElementAfterLine(cursor.line_id)) {
    repeated = getCharForLineIdentifier(moveRight(cursor).line_id);
  }

  Cursor tmp_cur;
  bool canWorkWithLetter = true;
  while (hasElementAfterLine(cursor.line_id) && (
           (canWorkWithLetter = isAWordLetter(getCharForLineIdentifier((tmp_cur = moveRight(cursor)).line_id)) &&
                                canWorkWithLetter) || areChar_U8Equals(getCharForLineIdentifier(tmp_cur.line_id),
                                                                       repeated))) {
    cursor = tmp_cur;
  }

  if (areCursorEqual(old_cur, cursor)) {
    return moveRight(cursor);
  }

  return cursor;
}

Cursor moveToPreviousWord(Cursor cursor) {
  cursor = skipLeftInvisibleChar(cursor);
  Cursor old_cur = cursor;

  Char_U8 repeated;
  if (cursor.line_id.absolute_column != 0) {
    repeated = getCharForLineIdentifier(cursor.line_id);
  }

  bool canWorkWithLetter = true;
  while (cursor.line_id.absolute_column != 0 && (
           (canWorkWithLetter = isAWordLetter(getCharForLineIdentifier(cursor.line_id)) && canWorkWithLetter) ||
           areChar_U8Equals(
             getCharForLineIdentifier(cursor.line_id), repeated))) {
    cursor = moveLeft(cursor);
  }

  if (areCursorEqual(old_cur, cursor)) {
    return moveLeft(cursor);
  }

  return cursor;
}

Cursor insertCharArrayAtCursor(Cursor cursor, char* chs) {
  // Duplicated search in project DUP_SCAN.

  int index = 0;

  char c;
  while ((c = chs[index++]) != '\0') {
#ifdef LOGS
    assert(checkFileIntegrity(root) == true);
#endif
    if (iscntrl(c)) {
      if (c == '\n') {
#ifdef LOGS
        printf("Enter\r\n");
#endif
        cursor = insertNewLineInLineC(cursor);
      }
      else if (c == 9) {
#ifdef LOGS
        printf("Tab\r\n");
#endif
        Char_U8 ch;
        if (TAB_CHAR_USE) {
          ch.t[0] = '\t';
          cursor = insertCharInLineC(cursor, ch);
        }
        else {
          ch.t[0] = ' ';
          for (int i = 0; i < TAB_SIZE; i++) {
            cursor = insertCharInLineC(cursor, ch);
          }
        }
      }
      else {
#ifdef LOGS
        printf("Unsupported Char loaded from file : '%d'.\r\n", c);
#endif
        // exit(0);
      }
    }
    else {
      Char_U8 ch = readChar_U8FromCharArrayWithFirst(chs + index - 1, c);
      index += sizeChar_U8(ch) - 1;
#ifdef LOGS
      printChar_U8(stdout, ch);
      printf("\r\n");
#endif
      cursor = insertCharInLineC(cursor, ch);
    }
  }


  return cursor;
}


Cursor byteCursorToCursor(Cursor cursor, int row, int byte_column) {
  row += 1;
  cursor = tryToReachAbsPosition(cursor, row, 0);

  int byte_count = 0;
  while (byte_count < byte_column) {
    Cursor old_cur = cursor;
    cursor = moveRight(cursor);
    if (old_cur.file_id.absolute_row != cursor.file_id.absolute_row || areCursorEqual(old_cur, cursor)) {
      return old_cur;
    }
    byte_count += sizeChar_U8(getCharAtCursor(cursor));
  }
  return cursor;
}

////// -------------- SELECTION MANAGEMENT --------------


bool isCursorDisabled(Cursor cursor) {
  return cursor.file_id.absolute_row == -1;
}

int charBetween2Curso(Cursor cur1, Cursor cur2) {
  int count = 0;
  while (isCursorPreviousThanOther(cur1, cur2)) {
    cur1 = moveRight(cur1);
    count++;
  }
  return count;
}

Cursor disableCursor(Cursor cursor) {
  cursor.file_id.absolute_row = -1;
  return cursor;
}

void setSelectCursorOn(Cursor cursor, Cursor* select_cursor) {
  if (isCursorDisabled(*select_cursor) == true) {
    *select_cursor = cursor;
  }
}

void setSelectCursorOff(Cursor* cursor, Cursor* select_cursor, SELECT_OFF_STYLE style) {
  if (isCursorDisabled(*select_cursor) == true)
    return;


  if (style == SELECT_OFF_RIGHT && isCursorPreviousThanOther(*cursor, *select_cursor)) {
    Cursor tmp = *cursor;
    *cursor = *select_cursor;
    *select_cursor = tmp;
  }
  else if (style == SELECT_OFF_LEFT && isCursorPreviousThanOther(*select_cursor, *cursor)) {
    Cursor tmp = *cursor;
    *cursor = *select_cursor;
    *select_cursor = tmp;
  }

  *select_cursor = disableCursor(*select_cursor);
}


void selectWord(Cursor* cursor, Cursor* select_cursor) {
  if (cursor->line_id.absolute_column != 0 && isAWordLetter(getCharForLineIdentifier(cursor->line_id)) == true) {
    *cursor = moveToPreviousWord(*cursor);
  }
  setSelectCursorOn(*cursor, select_cursor);
  *cursor = moveToNextWord(*cursor);
}

void selectLine(Cursor* cursor, Cursor* select_cursor) {
  Cursor tmp = cursorOf(
    cursor->file_id,
    moduloLineIdentifierR(getLineForFileIdentifier(cursor->file_id), 0)
  );
  select_cursor->file_id = tryToReachAbsRow(cursor->file_id, cursor->file_id.absolute_row + 1);
  if (select_cursor->file_id.absolute_row == cursor->file_id.absolute_row) {
    tmp = moveLeft(tmp);
    select_cursor->line_id = getLastLineNode(getLineForFileIdentifier(cursor->file_id));
  }
  else {
    select_cursor->line_id = moduloLineIdentifierR(getLineForFileIdentifier(select_cursor->file_id), 0);
  }

  *cursor = tmp;
}

void deleteSelection(Cursor* cursor, Cursor* select_cursor) {
  if (isCursorDisabled(*select_cursor) == true) {
    return;
  }

  if (isCursorPreviousThanOther(*select_cursor, *cursor)) {
    Cursor tmp = *select_cursor;
    *select_cursor = *cursor;
    *cursor = tmp;
  }

  assert(isCursorPreviousThanOther(*cursor, *select_cursor));

  *cursor = bulkDelete(*cursor, *select_cursor);

  *select_cursor = disableCursor(*select_cursor);
}

void deleteSelectionWithHist(History** history_p, Cursor* cursor, Cursor* select_cursor) {
  saveAction(history_p, createDeleteAction(*cursor, *select_cursor));
  deleteSelection(cursor, select_cursor);
}


char* dumpSelection(Cursor cur1, Cursor cur2) {
  char* dump = NULL;

  Cursor it = cur1;
  int byte_between_2_cursor = 0;
  while (isCursorStrictPreviousThanOther(it, cur2)) {
    Cursor tmp = it;
    it = moveRight(it);

    if (hasElementBeforeLine(it.line_id) == true) {
      byte_between_2_cursor += sizeChar_U8(getCharForLineIdentifier(it.line_id));
    }
    else {
      assert(tmp.file_id.absolute_row != it.file_id.absolute_row);
      byte_between_2_cursor++; // Add one byte alloc to save '\n' char.
    }
  }

  if (byte_between_2_cursor == 0) {
    dump = malloc(1 * sizeof(char));
    dump[0] = '\0';
    return dump;
  }

  assert(byte_between_2_cursor != 0);


  dump = malloc(byte_between_2_cursor * sizeof(char) + 1 /*for EOF char*/);

  it = cur1;
  int index = 0;
  while (isCursorStrictPreviousThanOther(it, cur2)) {
    it = moveRight(it);

    if (hasElementBeforeLine(it.line_id) == true) {
      Char_U8 ch = getCharForLineIdentifier(it.line_id);
      int ch_size = sizeChar_U8(ch);
      for (int i = 0; i < ch_size; i++) {
        dump[index] = ch.t[i];
        index++;
      }
    }
    else {
      dump[index] = '\n';
      index++;
    }
  }

  dump[index] = '\0';

  return dump;
}
