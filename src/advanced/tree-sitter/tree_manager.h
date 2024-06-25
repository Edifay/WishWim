#ifndef TREE_MANAGER_H
#define TREE_MANAGER_H
#include <regex.h>

#include "../../../lib/tree-sitter/lib/include/tree_sitter/api.h"
#include "../theme.h"


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

struct TreePathSeq_ {
  TreePath* value;
  struct TreePathSeq_* next;
};

typedef struct TreePathSeq_ TreePathSeq;


////// ------------------- TREE HANDLER -------------------

// TODO implement a parser abstraction.
typedef struct {
  char lang_name[100];
  const TSLanguage* lang;
  TSParser* parser;
  TreePathSeq highlight_queries;
  HighlightThemeList theme_list;
} ParserContainer;

typedef struct {
  ParserContainer* list;
  int size;
} ParserList;

typedef struct {
  char lang_name[100];
  bool is_active;
  TSTree* tree;
} FileHighlightDatas;

const TSLanguage* tree_sitter_c(void);

const TSLanguage* tree_sitter_python(void);

void initParserList(ParserList* list);

void destroyParserList(ParserList *list);

void addParserToParserList(ParserList* list, ParserContainer new_parser);

ParserContainer* getParserForLanguage(ParserList* list, char* language);

bool loadNewParser(ParserContainer* container, char* language);


////// ------------------- TREE UTILS -------------------

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
