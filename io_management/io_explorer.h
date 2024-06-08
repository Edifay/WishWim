#ifndef IO_EXPLORER_H
#define IO_EXPLORER_H
#include <stdbool.h>
#include <linux/limits.h>


// TODO implement theses shits
typedef struct {
  char path[PATH_MAX];
  int size;
} ExplorerFile;

struct _ExplorerFolder {
  char path[PATH_MAX];
  struct _ExplorerFolder* folders;
  ExplorerFile* files;
  int folder_count;
  int file_count;
  bool discovered;
  bool open;
};

typedef struct _ExplorerFolder ExplorerFolder;

void initFolder(char* path, ExplorerFolder* folder);

void discoverFolder(ExplorerFolder* folder);

void destroyFolder(ExplorerFolder* folder);

#endif //IO_EXPLORER_H
