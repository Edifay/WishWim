#ifndef STATE_CONTROL_H
#define STATE_CONTROL_H

#include "file_structure.h"
#include "../utils/tools.h"
#include "../io_management/io_manager.h"

#define TIME_CONSIDER_UNIQUE_UNDO 200 /*MS*/

#define FILE_STATE_PATH "/tmp/al/state_control/"

void createTmpDir();

typedef enum {
  INSERT = 'i',
  DELETE = 'd',
  DELETE_ONE = 'D',
  ACTION_NONE = 'n'
} ACTION_TYPE;


typedef struct {
  ACTION_TYPE action;
  char unique_ch;
  char* ch;
  time_val time;
  Cursor cur;
  Cursor cur_end;
} Action;

struct History_ {
  Action action;

  struct History_* next;
  struct History_* prev;
};

typedef struct History_ History;

void initHistory(History* history);

Cursor undo(History** history_p, Cursor cursor, void (*forEachUndo)(History** history_frame, History** old_history_frame, long* payload), long* payload);

Cursor redo(History** history_p, Cursor cursor, void (*forEachRedo)(History** history_frame, History** old_history_frame, long* payload), long* payload);

void saveAction(History** history, Action action);

Cursor doReverseAction(Action* action_p, Cursor cursor);

Action createDeleteAction(Cursor cur1, Cursor cur2);

Action createInsertAction(Cursor cur1, Cursor cur2);

void destroyAction(Action action);

void destroyEndOfHistory(History* history);

void saveCurrentStateControl(History root, History* current_state, char* fileName);

void loadCurrentStateControl(History* root, History** current_state, IO_FileID io_file);


#endif //STATE_CONTROL_H
