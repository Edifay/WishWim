#ifndef CLIPBOARD_MANAGER_H
#define CLIPBOARD_MANAGER_H

#include "../data-management/file_structure.h"


// If changed go to clipboard_manager.c and change here too.
#define CLIPBOARD_PATH "/tmp/al/clipboard"

bool saveToClipBoard(Cursor begin, Cursor end);

Cursor loadFromClipBoard(Cursor cursor);

#endif //CLIPBOARD_MANAGER_H
