#include "tree_manager.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
void treeForEachNodeSized(int y_offset, int height, TSNode root_node, TreePath* path_symbol, int offset,
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
    const char* field = ts_node_field_name_for_child(root_node, i);
    if (field != NULL) {
      offset++;
      path_symbol[offset].type = FIELD;
      path_symbol[offset].name = ts_node_field_name_for_child(root_node, i);
      path_symbol[offset].next = NULL;
      path_symbol[offset].reg = NULL;
    }
    treeForEachNodeSized(y_offset, height, current_child, path_symbol, offset + 1, func, args);
    if (field != NULL) {
      offset--;
    }
  }
}
