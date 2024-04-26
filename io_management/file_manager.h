#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "../data-structure/file_structure.h"

Cursor initWrittableFileFromFile(char* fileName);

bool loadFile(Cursor cursor, char* fileName);

void saveFile(FileNode* file, char* fileName);

#endif //FILE_MANAGER_H
