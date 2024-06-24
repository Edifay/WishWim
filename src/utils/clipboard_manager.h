#ifndef CLIPBOARD_MANAGER_H
#define CLIPBOARD_MANAGER_H

#include "../data-management/file_structure.h"

bool saveToClipBoard(Cursor begin, Cursor end);

Cursor loadFromClipBoard(Cursor cursor);

#endif //CLIPBOARD_MANAGER_H
