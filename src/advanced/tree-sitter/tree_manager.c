#include "tree_manager.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>

#include "scm_parser.h"
#include "../../terminal/highlight.h"
#include "../../utils/constants.h"
#include "../../../lib/tree-sitter/lib/include/tree_sitter/api.h"


void initParserList(ParserList* list) {
  list->size = 0;
  list->list = NULL;
}

void addParserToParserList(ParserList* list, ParserContainer new_parser) {
  list->size++;
  list->list = realloc(list->list, list->size * sizeof(ParserContainer));
  list->list[list->size - 1] = new_parser;
}

void destroyParserList(ParserList* list) {
  for (int i = 0; i < list->size; i++) {
    ts_parser_delete(list->list[i].parser);
    ts_language_delete(list->list[i].lang);
    destroyTreePathSeq(&list->list[i].highlight_queries);
    destroyThemeList(&list->list[i].theme_list);
  }
  free(list->list);
  list->list = NULL;
  list->size = 0;
}

ParserContainer* getParserForLanguage(ParserList* list, char* language) {
  for (int i = 0; i < list->size; i++) {
    if (strcmp(list->list[i].lang_name, language) == 0) {
      return list->list + i;
    }
  }

  ParserContainer new_parser;
  bool result = loadNewParser(&new_parser, language);
  if (result == false) {
    return NULL;
  }

  addParserToParserList(list, new_parser);

  return list->list + list->size - 1;
}

bool loadNewParser(ParserContainer* container, char* language) {
  if (strcmp(language, "c") == 0 || strcmp(language, "python") == 0) {
    strcpy(container->lang_name, language);

    char* load_path = cJSON_GetStringValue(cJSON_GetObjectItem(config, "default_path"));

    char path[PATH_MAX];
    // Theme
    sprintf(path, "%s/theme", load_path);

    bool res = getThemeFromFile(path, &container->theme_list);
    if (res == false) {
      printf("Unable to load theme from file '%s'. You can edit path in config file.\n", path);
      return false;
    }

    // Queries
    sprintf(path, "%s/queries/highlights-%s.scm", load_path, container->lang_name);
    res = parseSCMFile(&container->highlight_queries, path);
    if (res == false) {
      destroyThemeList(&container->theme_list);
      printf("Unable to load queries from file '%s'. You can edit path in config file.\n", path);
      return false;
    }

    // Post processing
    initColorsForTheme(container->theme_list, &color_index, &color_pair);
    sortTreePathSeqByDecreasingSize(&container->highlight_queries);

    // parsers
    if (strcmp(language, "c") == 0) {
      container->lang = tree_sitter_c();
    }
    else if (strcmp(language, "python") == 0) {
      container->lang = tree_sitter_python();
    }
    container->parser = ts_parser_new();
    ts_parser_set_language(container->parser, container->lang);
    return true;
  }
  return false;
}


int lengthTreePath(TreePath* tree_path) {
  int count = 0;
  while (tree_path != NULL) {
    count++;
    tree_path = tree_path->next;
  }
  return count;
}

void destroyTreePath(TreePath* path) {
  if (path == NULL)
    return;

  free(path->name);
  if (path->reg != NULL) {
    regfree(path->reg);
  }
  free(path->reg);

  path = path->next;
  while (path != NULL) {
    TreePath* tmp = path;
    path = path->next;
    free(tmp->name);
    if (tmp->reg != NULL) {
      regfree(tmp->reg);
    }
    free(tmp->reg);
    free(tmp);
  }
}

void printTreePath(TreePath* path) {
  while (path != NULL) {
    switch (path->type) {
      case SYMBOL:
        printf("(%s)", path->name);
        break;
      case FIELD:
        printf("%s:", path->name);
        break;
      case REGEX:
        printf("Reg:\"%s\"", path->name);
        break;
      case GROUP:
        printf("@%s", path->name);
        break;
      case MATCH:
        printf("\"%s\"", path->name);
        break;
    }
    printf(" ");
    path = path->next;
  }
}


// TODO test this
char* isTreePathMatchingQuery(char* last_node_text, TreePath tree_path[], int tree_path_length, TreePath* query) {
  if (query == NULL) {
    return NULL;
  }

  // Regex is supported only for the last cell.
  if (query->type == REGEX) {
    if (last_node_text == NULL) {
      // Unable to check regex.
      return NULL;
    }
    int regex_result = regexec(query->reg, last_node_text, 0, NULL, 0);
    if (regex_result != 0) {
      // regex do not match.
      return NULL;
    }
    query = query->next;
  }

  // We will assume that a match sequence is a simple element made of 1 match and 1 group.
  int i = tree_path_length - 1;
  char* group_name = NULL;
  while (i >= 0) {
    // Query is empty mean it work.
    if (query == NULL) {
      if (group_name == NULL) {
        printf("Match without any group found.\n");
      }
      return group_name;
    }

    TreePath current_cell = tree_path[i];

    // printf("Checking for : cell : {name:'%s', type:'%c'} | Query : {name:'%s', type:'%c'}\n", current_cell.name, current_cell.type, query->name, query->type);

    // If the query type is GROUP just save it end go to the next
    if (query->type == GROUP) {
      if (group_name == NULL) {
        // Last group found is keep.
        group_name = query->name;
      }
      query = query->next;
      continue;
    }

    if (query->type == REGEX) {
      printf("Regex in midle of a query is not supported.\n");
      return NULL;
    }

    if (query->type == MATCH) {
      if (strcmp(last_node_text, query->name) != 0) {
        return NULL;
      }
      query = query->next;
      assert(query == NULL);
      continue;
    }

    // Field are not always specified in query so skip field in current_path if the field is not specified.
    if (current_cell.type == FIELD && query->type != FIELD) {
      i--;
      continue;
    }

    // Type do not.
    if (current_cell.type != query->type) {
      // Path don't match.
      return NULL;
    }

    // HERE query.type == current_cell.type
    assert(query->type == FIELD || query->type == SYMBOL);
    if (strcmp(current_cell.name, query->name) != 0) {
      return NULL;
    }

    query = query->next;
    i--;
  }

  return group_name;
}


void treeForEachNode(TSNode root_node, TreePath* path_symbol, int offset, void (*func)(TSNode node, TreePath tree_path[], int tree_path_length, long* args), void* args) {
  const char* name = ts_node_type(root_node);
  path_symbol[offset].type = SYMBOL;
  path_symbol[offset].name = name;
  path_symbol[offset].next = NULL;
  path_symbol[offset].reg = NULL;

  if (func != NULL) {
    func(root_node, path_symbol, offset + 1, args);
  }

  for (int i = 0; i < ts_node_child_count(root_node); i++) {
    TSNode current_child = ts_node_child(root_node, i);
    const char* field = ts_node_field_name_for_child(root_node, i);


    if (field != NULL) {
      offset++;
      path_symbol[offset].type = FIELD;
      path_symbol[offset].name = ts_node_field_name_for_child(root_node, i);
      path_symbol[offset].next = NULL;
      path_symbol[offset].reg = NULL;
    }
    treeForEachNode(current_child, path_symbol, offset + 1, func, args);
    if (field != NULL) {
      offset--;
    }
  }
}

// Put offset to 0 if you call by your own this function.
void treeForEachNodeSized(int y_offset, int x_offset, int height, int width, TSNode root_node, TreePath* path_symbol, int offset,
                          void (*func)(TSNode node, TreePath tree_path[], int tree_path_length, long* args), void* args) {
  const char* name = ts_node_type(root_node);
  path_symbol[offset].type = SYMBOL;
  path_symbol[offset].name = name;
  path_symbol[offset].next = NULL;
  path_symbol[offset].reg = NULL;

  if (func != NULL) {
    func(root_node, path_symbol, offset + 1, args);
  }

  for (int i = 0; i < ts_node_child_count(root_node); i++) {
    TSNode current_child = ts_node_child(root_node, i);
    TSPoint begin = ts_node_start_point(current_child);
    TSPoint end = ts_node_end_point(current_child);
    // printf("Height %d\n", y_offset);
    if (y_offset + height < begin.row || end.row < y_offset - 1) {
      continue;
    }
    if (begin.row == end.row && (x_offset + width < begin.column || end.column < x_offset - 1)) {
      continue;
    }
    const char* field = ts_node_field_name_for_child(root_node, i);
    if (field != NULL) {
      offset++;
      path_symbol[offset].type = FIELD;
      path_symbol[offset].name = ts_node_field_name_for_child(root_node, i);
      path_symbol[offset].next = NULL;
      path_symbol[offset].reg = NULL;
    }
    treeForEachNodeSized(y_offset, x_offset, height, width, current_child, path_symbol, offset + 1, func, args);
    if (field != NULL) {
      offset--;
    }
  }
}


void detectLanguage(FileHighlightDatas* data, IO_FileID io_file) {
  char* dot = strrchr(io_file.path_args, '.');
  if (dot != NULL)
    strncpy(data->lang_name, dot + 1, 99);

  if (strcmp(data->lang_name, "h") == 0 || strcmp(data->lang_name, "c") == 0) {
    strcpy(data->lang_name, "c");
  }
  else if (strcmp(data->lang_name, "py") == 0) {
    strcpy(data->lang_name, "python");
  }

  ParserContainer* parser = getParserForLanguage(&parsers, data->lang_name);
  if(parser != NULL) {
    data->is_active = true;
  }
  data->tree = NULL;
}
