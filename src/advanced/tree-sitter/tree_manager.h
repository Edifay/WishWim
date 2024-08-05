#ifndef TREE_MANAGER_H
#define TREE_MANAGER_H
#include <regex.h>

#include "../../../lib/tree-sitter/lib/include/tree_sitter/api.h"
#include "../theme.h"
#include "../../io_management/io_manager.h"
#include "../../data-management/state_control.h"


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

typedef struct {
  History* history_frame;
  int byte_start;
  int byte_end;
} ActionImprovedWithBytes;


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
  char* tmp_file_dump;
} FileHighlightDatas;

const TSLanguage* tree_sitter_c(void);

const TSLanguage* tree_sitter_python(void);

const TSLanguage* tree_sitter_java(void);

const TSLanguage* tree_sitter_cpp(void);

const TSLanguage* tree_sitter_c_sharp(void);

const TSLanguage* tree_sitter_markdown(void);

const TSLanguage* tree_sitter_markdown_inline(void);

const TSLanguage* tree_sitter_make(void);

const TSLanguage* tree_sitter_css(void);

const TSLanguage* tree_sitter_dart(void);

const TSLanguage* tree_sitter_go(void);

const TSLanguage* tree_sitter_javascript(void);

const TSLanguage* tree_sitter_json(void);

const TSLanguage* tree_sitter_bash(void);

const TSLanguage* tree_sitter_query(void);


void initParserList(ParserList* list);

void destroyParserList(ParserList* list);

void addParserToParserList(ParserList* list, ParserContainer new_parser);

ParserContainer* getParserForLanguage(ParserList* list, char* language);

bool loadNewParser(ParserContainer* container, char* language);


////// ------------------- TREE UTILS -------------------

int lengthTreePath(TreePath* tree_path);

void destroyTreePath(TreePath* path);

void printTreePath(TreePath* path);

// Return null if not matching, return a pointer on a group if match found.
char* isTreePathMatchingQuery(char* last_node_text, TreePath tree_path[], int tree_path_length, TreePath* query);


void treeForEachNode(TSNode root_node, TreePath* path_symbol, int offset, void (*func)(TSNode node, TreePath tree_path[], int tree_path_length, long* args), void* args);

void treeForEachNodeSized(int y_offset, int x_offset, int height, int width, TSNode root_node, TreePath* path_symbol, int offset,
                          void (*func)(TSNode node, TreePath tree_path[], int tree_path_length, long* args), void* args);

void detectLanguage(FileHighlightDatas* data, IO_FileID io_file);


void edit_tree(FileHighlightDatas* highlight_data, FileNode** root, char** tmp_file_dump, int* n_bytes, History** history_frame, History* old_history_frame);


void edit_and_parse_tree(FileNode** root, History** history_frame, FileHighlightDatas* highlight_data, History** old_history_frame);


long* get_payload_edit_and_parse_tree(FileNode*** root, FileHighlightDatas** highlight_data);

void edit_and_parse_tree_from_payload(History** history_frame, History* *old_history_frame, long* payload);


#endif //TREE_MANAGER_H
