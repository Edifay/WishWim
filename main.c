#include <assert.h>
#include <ncurses.h>
#include <stdlib.h>

#include "data-structure/file_structure.h"
#include "io_management/file_manager.h"

Cursor createFile(int argc, char** args);

void printFile(Cursor cursor, int screen_x, int screen_y);

int main(int argc, char** args) {
  Cursor cursor = createFile(argc, args);
  FileNode* root = cursor.file_id.file;
  assert(root->prev == NULL);

  int screen_x = 0;
  int screen_y = 0;

  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();

  getch();

  refresh();
  endwin();
  destroyFullFile(root);
  return 0;
}


void printFile(Cursor cursor, int screen_x, int screen_y) {

}



Cursor createFile(int argc, char** args) {
  if (argc >= 2) {
    return initWrittableFileFromFile(args[1]);
  }
  return initNewWrittableFile();
}
