#ifndef TREE_MANAGER_H
#define TREE_MANAGER_H
#include <regex.h>

#include "../../lib/tree-sitter/lib/include/tree_sitter/api.h"


typedef enum {
  SYMBOL = 's',
  FIELD = 'f',
  REGEX = 'r',
  GROUP = 'g',
  MATCH = 'm'
} PATH_TYPE;

struct TreePath_ {
  PATH_TYPE type;
  regex_t* reg;
  char* name;
  struct TreePath_* next;
};

typedef struct TreePath_ TreePath;


const TSLanguage* tree_sitter_c(void);

const TSLanguage* tree_sitter_python(void);

int lengthTreePath(TreePath* tree_path);

void destroyTreePath(TreePath* path);

void printTreePath(TreePath* path);

// Return null if not matching, return a pointer on a group if match found.
char* isTreePathMatchingQuery(char* last_node_text, TreePath tree_path[], int tree_path_length, TreePath* query);


// TODO implement match for color group using SCM parse.
void treeForEachNode(TSNode root_node, TreePath* path_symbol, int offset, void (*func)(TSNode node, TreePath tree_path[], int tree_path_length, long* args), void* args);

void treeForEachNodeSized(int y_offset, int height, TSNode root_node, TreePath* path_symbol, int offset,
                          void (*func)(TSNode node, TreePath tree_path[], int tree_path_length, long* args), void* args);


#endif //TREE_MANAGER_H
