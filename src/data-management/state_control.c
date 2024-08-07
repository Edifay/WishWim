#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "state_control.h"
#include "file_management.h"
#include "../io_management/viewport_history.h"


void initHistory(History* history) {
  history->action.action = ACTION_NONE;
  history->action.time = 0;
  history->prev = NULL;
  history->next = NULL;
}


Cursor undo(History** history_p, Cursor cursor, void (*forEachUndo)(History** history_frame, History** old_history_frame, long* payload), long* payload) {
  History* history = *history_p;

  // If hisotory is at root return and do nothing. Cannot undo nothing ;).
  if (history->action.action == ACTION_NONE) {
    return cursor;
  }

  time_val canceled_time_action = history->action.time;
  cursor = doReverseAction(&history->action, cursor);
  history->action.time = canceled_time_action;

  History* old_history = *history_p;
  *history_p = history->prev;

  if (forEachUndo != NULL) {
    forEachUndo(history_p, &old_history, payload);
  }

  if (diff2Time(history->action.time, history->prev->action.time) < TIME_CONSIDER_UNIQUE_UNDO) {
    return undo(history_p, cursor, forEachUndo, payload);
  }

  return cursor;
}

Cursor redo(History** history_p, Cursor cursor, void (*forEachRedo)(History** history_frame, History** old_history_frame, long* payload), long* payload) {
  History* history = *history_p;

  // If hisotory is at root return and do nothing. Cannot undo nothing ;).
  if (history->next == NULL) {
    return cursor;
  }

  History* old_history = *history_p;
  *history_p = history->next;
  history = *history_p;

  time_val canceled_time_action = history->action.time;

  cursor = doReverseAction(&history->action, cursor);
  history->action.time = canceled_time_action;

  if (forEachRedo != NULL) {
    forEachRedo(history_p, &old_history, payload);
  }

  if (history->next != NULL && diff2Time(history->action.time, history->next->action.time) < TIME_CONSIDER_UNIQUE_UNDO) {
    return redo(history_p, cursor, forEachRedo, payload);
  }

  return cursor;
}


void saveAction(History** history_p, Action action) {
  History* history = *history_p;
  if (action.action == ACTION_NONE) {
    return;
  }

  destroyEndOfHistory(history);
  assert(history->next == NULL);

  assert(history != NULL);
  history->next = malloc(sizeof(History));

  assert(history->next != NULL);
  history->next->prev = history;
  history->next->next = NULL;
  history->next->action = action;

  history = history->next;

  *history_p = history;
}

Cursor doReverseAction(Action* action_p, Cursor cursor) {
  Action action = *action_p;
  Cursor tmp;
  Cursor tmp_end;
  ACTION_TYPE type = action.action;
  switch (type) {
    case DELETE_ONE:
      tmp.file_id = tryToReachAbsRow(cursor.file_id, action.cur.file_id.absolute_row);
      tmp.line_id = moduloLineIdentifierR(getLineForFileIdentifier(tmp.file_id), action.cur.line_id.absolute_column);
      if (action.unique_ch == '\n') {
        cursor = insertNewLineInLineC(tmp);
        destroyAction(action);
        *action_p = createInsertAction(tmp, cursor);
        return cursor;
      }
      cursor = insertCharInLineC(tmp, readChar_U8FromInput(action.unique_ch));
      destroyAction(action);
      *action_p = createInsertAction(tmp, cursor);
      return cursor;
    case DELETE:
      tmp.file_id = tryToReachAbsRow(cursor.file_id, action.cur.file_id.absolute_row);
      tmp.line_id = moduloLineIdentifierR(getLineForFileIdentifier(tmp.file_id), action.cur.line_id.absolute_column);
      cursor = insertCharArrayAtCursor(tmp, action.ch);
      destroyAction(action);
      *action_p = createInsertAction(tmp, cursor);
      return cursor;
    case INSERT:
      tmp.file_id = tryToReachAbsRow(cursor.file_id, action.cur.file_id.absolute_row);
      tmp.line_id = moduloLineIdentifierR(getLineForFileIdentifier(tmp.file_id), action.cur.line_id.absolute_column);
      tmp_end.file_id = tryToReachAbsRow(cursor.file_id, action.cur_end.file_id.absolute_row);
      tmp_end.line_id = moduloLineIdentifierR(getLineForFileIdentifier(tmp_end.file_id), action.cur_end.line_id.absolute_column);

      destroyAction(action);
      *action_p = createDeleteAction(tmp, tmp_end);
      deleteSelection(&tmp, &tmp_end);
      return tmp;
    case ACTION_NONE:
      return cursor;
    default:
      assert(false);
  }
}

Action createDeleteAction(Cursor cur1, Cursor cur2) {
  if (isCursorDisabled(cur1) || isCursorDisabled(cur2)) {
    Action action;
    action.action = ACTION_NONE;
    action.time = 0;
    return action;
  }

  if (isCursorPreviousThanOther(cur2, cur1)) {
    Cursor tmp = cur1;
    cur1 = cur2;
    cur2 = tmp;
  }

  Action action;
  action.time = timeInMilliseconds();
  action.cur = cur1;
  // Not used.
  action.cur_end = disableCursor(cur1);

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
    action.action = ACTION_NONE;
    action.time = 0;
    return action;
  }

  assert(byte_between_2_cursor != 0);

  // If there is only 1 element to save we don't use malloc. We use unique_ch.
  if (byte_between_2_cursor == 1) {
    action.action = DELETE_ONE;
    action.ch = NULL;
    if (hasElementBeforeLine(it.line_id) == true) {
      action.unique_ch = getCharForLineIdentifier(it.line_id).t[0];
    }
    else {
      action.unique_ch = '\n';
    }
    return action;
  }


  action.action = DELETE;
  action.unique_ch = '\0';
  action.ch = malloc(byte_between_2_cursor * sizeof(char) + 1 /*for EOF char*/);
  assert(action.ch != NULL);

  it = cur1;
  int index = 0;
  while (isCursorStrictPreviousThanOther(it, cur2)) {
    it = moveRight(it);

    if (hasElementBeforeLine(it.line_id) == true) {
      Char_U8 ch = getCharForLineIdentifier(it.line_id);
      int ch_size = sizeChar_U8(ch);
      for (int i = 0; i < ch_size; i++) {
        action.ch[index] = ch.t[i];
        index++;
      }
    }
    else {
      action.ch[index] = '\n';
      index++;
    }
  }

  action.ch[index] = '\0';

  return action;
}

Action createInsertAction(Cursor cur1, Cursor cur2) {
  Action action;
  action.action = INSERT;
  // Know that cur is the first cursor can be useful.
  if (isCursorPreviousThanOther(cur2, cur1) == true) {
    action.cur = cur2;
    action.cur_end = cur1;
  }
  else {
    action.cur = cur1;
    action.cur_end = cur2;
  }
  action.time = timeInMilliseconds();
  action.unique_ch = '\0';
  action.ch = NULL;
  return action;
}

void destroyAction(Action action) {
  assert(action.action != ACTION_NONE);
  free(action.ch);
}

void destroyEndOfHistory(History* history) {
  assert(history != NULL);
  History* tmp = history;

  history = history->next;
  while (history != NULL) {
    History* tmp2 = history;
    history = history->next;

    assert(tmp2->action.action != ACTION_NONE);
    destroyAction(tmp2->action);
    free(tmp2);
  }

  tmp->next = NULL;
}


void createTmpDir() {
  char command[20 + strlen(FILE_STATE_PATH)];
  sprintf(command, "mkdir %s -p",FILE_STATE_PATH);
  system(command);
}

void saveCurrentStateControl(History root, History* current_state, char* fileName) {
  createTmpDir();

  char fileStateControl[strlen(FILE_STATE_PATH) + 60 /*size of the hash*/ + 1];
  sprintf(fileStateControl, "%s%llu", FILE_STATE_PATH, hashFileName(fileName));


  FILE* f = fopen(fileStateControl, "w");
  if (f == NULL) {
    printf("Impossible to save state control, couldn't open file %s.\r\n", fileStateControl);
    return;
  }

  struct stat attr;
  stat(fileName, &attr);
  fprintf(f, "%ld\n", attr.st_mtime);

  History* current = root.next;
  while (current != NULL) {
    char isCurrentState = current == current_state ? 'y' : 'n';
    fprintf(f, "%c %c %lld\n", isCurrentState, current->action.action, current->action.time);

    switch (current->action.action) {
      case DELETE:
        fprintf(f, "%d %d\n", current->action.cur.file_id.absolute_row, current->action.cur.line_id.absolute_column);
        fprintf(f, "%ld\n", strlen(current->action.ch));
        fprintf(f, "%s\n", current->action.ch);
        break;
      case DELETE_ONE:
        fprintf(f, "%d %d\n", current->action.cur.file_id.absolute_row, current->action.cur.line_id.absolute_column);
        fprintf(f, "%c\n", current->action.unique_ch);
        break;
      case INSERT:
        fprintf(f, "%d %d\n", current->action.cur.file_id.absolute_row, current->action.cur.line_id.absolute_column);
        fprintf(f, "%d %d\n", current->action.cur_end.file_id.absolute_row, current->action.cur_end.line_id.absolute_column);
        break;
      case ACTION_NONE:
        break;
      default:
        assert(false);
    }
    current = current->next;
  }


  fclose(f);
}

void loadCurrentStateControl(History* root, History** current_state, IO_FileID io_file) {
  *current_state = root;
  initHistory(root);

  if (io_file.status != EXIST) {
    return;
  }

  char fileStateControl[strlen(FILE_STATE_PATH) + 60 /*size of the hash*/ + 1];
  sprintf(fileStateControl, "%s%llu", FILE_STATE_PATH, hashFileName(io_file.path_abs));

  FILE* f = fopen(fileStateControl, "r");
  if (f == NULL) {
    return;
  }

  struct stat attr;
  stat(io_file.path_abs, &attr);

  struct timespec loaded;
  fscanf(f, "%ld\n", &loaded);

  if (loaded.tv_sec != attr.st_mtim.tv_sec) {
    fclose(f);
    return;
  }

  // TODO may use fread() instead of fscanf().
  History* state_to_return = *current_state;
  char scan_separtor;
  while (true) {
    char isCurrentState;
    if (fscanf(f, "%c", &isCurrentState) == EOF)
      break;

    Action action;
    action.ch = NULL;
    char tmp_ch_action_read;
    fscanf(f, "%c%c%c%lld%c", &scan_separtor, &tmp_ch_action_read, &scan_separtor, &action.time, &scan_separtor);
    action.action = tmp_ch_action_read;

    switch (action.action) {
      case DELETE:
        fscanf(f, "%d%c%d%c", &action.cur.file_id.absolute_row, &scan_separtor, &action.cur.line_id.absolute_column, &scan_separtor);
        long str_len;
        fscanf(f, "%ld%c", &str_len, &scan_separtor);
        action.ch = malloc(str_len + 1);
        assert(action.ch != NULL);
        for (int i = 0; i < str_len; i++) {
          fscanf(f, "%c", action.ch + i);
        }
        action.ch[str_len] = '\0';
      // printf("%ld\r\n", strlen(action.ch));
      // printf("%s\r\n", action.ch);
      // exit(0);
        fscanf(f, "%c", &scan_separtor);
        break;
      case DELETE_ONE:
        fscanf(f, "%d%c%d%c", &action.cur.file_id.absolute_row, &scan_separtor, &action.cur.line_id.absolute_column, &scan_separtor);
        fscanf(f, "%c%c", &action.unique_ch, &scan_separtor);
        break;
      case INSERT:
        fscanf(f, "%d%c%d%c", &action.cur.file_id.absolute_row, &scan_separtor, &action.cur.line_id.absolute_column, &scan_separtor);
        fscanf(f, "%d%c%d%c", &action.cur_end.file_id.absolute_row, &scan_separtor, &action.cur_end.line_id.absolute_column, &scan_separtor);
        break;
      case ACTION_NONE:
        break;
      default:
        printf("Current state : %c\r\n", isCurrentState);
        printf("HERE %c\r\n", action.action);
        assert(false);
    }

    saveAction(current_state, action);
    if (isCurrentState == 'y') {
      state_to_return = *current_state;
    }
  }

  *current_state = state_to_return;
}


void optimizeHistory(History* root, History** history_frame) {
  History* current = root;

  while (current != NULL && current->next != NULL) {
    History* next = current->next;
    if (diff2Time(current->action.time, next->action.time) < TIME_CONSIDER_UNIQUE_UNDO && current != *history_frame) {
      bool is_pos_linked = false;
      switch (current->action.action) {
        case DELETE:
        case DELETE_ONE:
          switch (next->action.action) {
            case DELETE_ONE:
              if (next->action.unique_ch == '\n') {
                if (current->action.cur.line_id.absolute_column == 0 && next->action.cur.line_id.absolute_column == current->action.cur.file_id.absolute_row - 1) {
                  // We may don't want to join when line is removed.
                  is_pos_linked = false;
                }
              }
              else {
                if (current->action.cur.file_id.absolute_row == next->action.cur.file_id.absolute_row
                    && current->action.cur.line_id.absolute_column - 1 == next->action.cur.line_id.absolute_column) {
                  is_pos_linked = true;
                }
              }

              if (is_pos_linked) {
                if (next == *history_frame) {
                  *history_frame = current;
                }

                if (current->action.action == DELETE_ONE) {
                  current->action.action = DELETE;
                  current->action.ch = malloc(3 * sizeof(char));
                  current->action.ch[0] = next->action.unique_ch;
                  current->action.ch[1] = current->action.unique_ch;
                  current->action.ch[2] = '\0';
                }
                else {
                  int old_size = strlen(current->action.ch);
                  current->action.ch = realloc(current->action.ch, (old_size + 2/*new char and null char*/) * sizeof(char));
                  memmove(current->action.ch + 1, current->action.ch, old_size);
                  current->action.ch[0] = next->action.unique_ch;
                  current->action.ch[old_size + 1] = '\0';
                }

                current->action.cur = next->action.cur;
                current->action.time = next->action.time;
                current->next = next->next;
                destroyAction(next->action);
                free(next);
              }
              break;
            default:
              break;
          }
          break;

        case INSERT:
          switch (next->action.action) {
            case INSERT:
              if (next->action.unique_ch == '\n') {
                // TODO change condition
                // if (history->action.cur.line_id.absolute_column == 0 && action.cur.line_id.absolute_column == history->action.cur.file_id.absolute_row - 1) {
                // We may don't want to join when line is removed.
                // is_pos_linked = false;
                // }
              }
              else {
                if (current->action.cur_end.file_id.absolute_row == next->action.cur.file_id.absolute_row && current->action.cur_end.line_id.absolute_column == next->action.cur.
                    line_id.
                    absolute_column) {
                  is_pos_linked = true;
                }
              }

              if (is_pos_linked) {
                if (next == *history_frame) {
                  *history_frame = current;
                }
                current->action.cur_end = next->action.cur_end;
                current->action.time = next->action.time;
                current->next = next->next;
                destroyAction(next->action);
                free(next);
                return;
              }
              break;

            default:
              break;
          }
          break;

        default:
          break;
      }
    }
    current = current->next;
  }
}
