#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H
#include <linux/limits.h>

#include "../data-structure/file_structure.h"

typedef enum {
	NONE,
	DONT_EXIST,
	EXIST,
} FILE_STATUS;


typedef struct{
	FILE_STATUS status;
	char path_abs[PATH_MAX];
	char *path_args;
} IO_FileID;

Cursor initWrittableFileFromFile(char* fileName);

bool loadFile(Cursor cursor, char* fileName);

void saveFile(FileNode* root, IO_FileID *file);

void setupFile(int argc, char** args, IO_FileID *file);

#endif //FILE_MANAGER_H
