#include "tree_manager.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <bits/time.h>
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
  if (
    // ADD_NEW_LANGUAGE
    strcmp(language, "c") == 0 ||
    strcmp(language, "python") == 0 ||
    strcmp(language, "markdown") == 0 ||
    strcmp(language, "java") == 0 ||
    strcmp(language, "cpp") == 0 ||
    strcmp(language, "c-sharp") == 0 ||
    strcmp(language, "make") == 0 ||
    strcmp(language, "css") == 0 ||
    strcmp(language, "dart") == 0 ||
    strcmp(language, "go") == 0 ||
    strcmp(language, "javascript") == 0 ||
    strcmp(language, "json") == 0 ||
    strcmp(language, "bash") == 0 ||
    strcmp(language, "query") == 0
  ) {
    strcpy(container->lang_name, language);

    char* load_path = cJSON_GetStringValue(cJSON_GetObjectItem(config, "default_path"));

    char path[PATH_MAX];
    // Theme
    sprintf(path, "%s/theme", load_path);

    bool res = getThemeFromFile(path, &container->theme_list);
    if (res == false) {
      fprintf(stderr, "Unable to load theme from file '%s'. You can edit path in config file.\n", path);
      return false;
    }

    // Queries
    sprintf(path, "%s/queries/highlights-%s.scm", load_path, container->lang_name);
    res = parseSCMFile(&container->highlight_queries, path);
    if (res == false) {
      destroyThemeList(&container->theme_list);
      fprintf(stderr, "Unable to load queries from file '%s'. You can edit path in config file.\n", path);
      return false;
    }

    // Post processing
    initColorsForTheme(container->theme_list, &color_index, &color_pair);
    sortTreePathSeqByDecreasingSize(&container->highlight_queries);

    // parsers
    // ADD_NEW_LANGUAGE
    if (strcmp(language, "c") == 0) {
      container->lang = tree_sitter_c();
    }
    else if (strcmp(language, "python") == 0) {
      container->lang = tree_sitter_python();
    }
    else if (strcmp(language, "markdown") == 0) {
      container->lang = tree_sitter_markdown();
    }
    else if (strcmp(language, "java") == 0) {
      container->lang = tree_sitter_java();
    }
    else if (strcmp(language, "cpp") == 0) {
      container->lang = tree_sitter_cpp();
    }
    else if (strcmp(language, "c-sharp") == 0) {
      container->lang = tree_sitter_c_sharp();
    }
    else if (strcmp(language, "make") == 0) {
      container->lang = tree_sitter_make();
    }
    else if (strcmp(language, "css") == 0) {
      container->lang = tree_sitter_css();
    }
    else if (strcmp(language, "dart") == 0) {
      container->lang = tree_sitter_dart();
    }
    else if (strcmp(language, "go") == 0) {
      container->lang = tree_sitter_go();
    }
    else if (strcmp(language, "javascript") == 0) {
      container->lang = tree_sitter_javascript();
    }
    else if (strcmp(language, "json") == 0) {
      container->lang = tree_sitter_json();
    }
    else if (strcmp(language, "bash") == 0) {
      container->lang = tree_sitter_bash();
    }
    else if (strcmp(language, "query") == 0) {
      container->lang = tree_sitter_query();
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

  // for (int i = 0; i < offset; i++) {
  // fprintf(stderr, " ");
  // }
#ifdef PARSE_PRINT
  fprintf(stderr, "( %s [%d, %d] -> [%d, %d] | [%d] -> [%d] )",
          name, ts_node_start_point(root_node).row, ts_node_start_point(root_node).column,
          ts_node_end_point(root_node).row, ts_node_end_point(root_node).column,
          ts_node_start_byte(root_node), ts_node_end_byte(root_node)
  );
#endif
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


void setFileHighlightDatas(FileHighlightDatas* data, IO_FileID io_file) {
  bool did_lang_was_found = getLanguageForFile(data->lang_name, io_file);

  ParserContainer* parser = NULL;
  if (did_lang_was_found == true) {
    parser = getParserForLanguage(&parsers, data->lang_name);
  }

  if (parser != NULL) {
    data->is_active = true;
  }

  data->tree = NULL;
  data->tmp_file_dump = NULL;
}


PayloadStateChange getPayloadStateChange(FileHighlightDatas* highlight_datas) {
  PayloadStateChange payload;
  payload.highlight_datas = highlight_datas;
  return payload;
}

void onStateChangeTS(Action action, long* payload_p) {
  PayloadStateChange payload = *(PayloadStateChange *)payload_p;

  TSInputEdit edit;
  switch (action.action) {
    case INSERT:
      system("echo \"=== INSERT ===\" >> tree_logs.txt");
      assert(action.byte_start != -1);
      assert(action.byte_end != -1);
      edit.start_byte = action.byte_start;
      edit.start_point.row = action.cur.file_id.absolute_row - 1;
      edit.start_point.column = action.cur.line_id.absolute_column;

      edit.old_end_byte = action.byte_start;
      edit.old_end_point.row = edit.start_point.row;
      edit.old_end_point.column = edit.start_point.column;

      edit.new_end_byte = action.byte_end;
      edit.new_end_point.row = action.cur_end.file_id.absolute_row - 1;
      edit.new_end_point.column = action.cur_end.line_id.absolute_column;
    // To force the match with previous node.
      break;
    case DELETE:
      system("echo \"=== DELETE ===\" >> tree_logs.txt");
      assert(action.byte_start != -1);
      fprintf(stderr, "DELETE BYTE_START : %u\n", action.byte_start);
      edit.start_byte = action.byte_start;
      edit.start_point.row = action.cur.file_id.absolute_row - 1;
      edit.start_point.column = action.cur.line_id.absolute_column;

      edit.old_end_byte = action.byte_end;

    // TODO may optimize
    // CALCULATE ROW AND COLUMN POINT
      char* ch = action.ch;
      int current_row = edit.start_point.row;
      int current_column = edit.start_point.column;

      int current_ch_index = 0;
      while (current_ch_index < action.byte_end - action.byte_start) {
        if (TAB_CHAR_USE == false) {
          assert(ch[current_ch_index] != '\t');
        }
        if (ch[current_ch_index] == '\n') {
          current_row++;
          current_column = 0;
        }
        else {
          Char_U8 tmp_ch = readChar_U8FromCharArray(ch + current_ch_index);
          current_ch_index += sizeChar_U8(tmp_ch) - 1;
          current_column++;
        }
        current_ch_index++;
      }

      edit.old_end_point.row = current_row;
      edit.old_end_point.column = current_column;


      edit.new_end_byte = action.byte_start;
      edit.new_end_point.row = edit.start_point.row;
      edit.new_end_point.column = edit.start_point.column;
    // To force the match with previous node.
      break;
    case DELETE_ONE:
      system("echo \"=== DELETE_ONE ===\" >> tree_logs.txt");
      assert(action.byte_start != -1);
      edit.start_byte = action.byte_start;
      edit.start_point.row = action.cur.file_id.absolute_row - 1;
      edit.start_point.column = action.cur.line_id.absolute_column;

      edit.old_end_byte = action.byte_start + 1;
      edit.old_end_point.row = edit.start_point.row;
      edit.old_end_point.column = edit.start_point.column;
      if (action.unique_ch == '\n') {
        edit.old_end_point.row++;
        edit.old_end_point.column = 0;
      }
      else {
        edit.old_end_point.column++;
      }

      edit.new_end_byte = action.byte_start;
      edit.new_end_point.row = edit.start_point.row;
      edit.new_end_point.column = edit.start_point.column;
    // To force the match with previous node.

      break;
    default:
      assert(action.action == ACTION_NONE);
      return;
  }

  /*
  // PRINT TO JSON EDITs
  cJSON* obj = cJSON_CreateObject();
  cJSON_AddNumberToObject(obj, "start_byte", edit.start_byte);
  cJSON_AddNumberToObject(obj, "start_point.row", edit.start_point.row);
  cJSON_AddNumberToObject(obj, "start_point.column", edit.start_point.column);

  cJSON_AddNumberToObject(obj, "old_end_byte", edit.old_end_byte);
  cJSON_AddNumberToObject(obj, "old_end_point.row", edit.old_end_point.row);
  cJSON_AddNumberToObject(obj, "old_end_point.column", edit.old_end_point.column);

  cJSON_AddNumberToObject(obj, "new_end_byte", edit.new_end_byte);
  cJSON_AddNumberToObject(obj, "new_end_point.row", edit.new_end_point.row);
  cJSON_AddNumberToObject(obj, "new_end_point.column", edit.new_end_point.column);

  char* obj_text = cJSON_Print(obj);

  // FILE *f = fopen("tree_logs.txt", "a");
  // fprintf(f, obj_text);
  // fprintf(f,"\n");
  // fclose(f);


  fprintf(stderr, obj_text);
  fprintf(stderr, "\n");


  free(obj_text);
  cJSON_Delete(obj);*/

  ts_tree_edit(payload.highlight_datas->tree, &edit);
}

char read_buffer[CHAR_CHUNK_SIZE_TSINPUT * 4];
const char* internalReaderForTree(void* payload, uint32_t byte_index, TSPoint position, uint32_t* bytes_read) {
  PayloadInternalReader* values = payload;
  *bytes_read = readNBytesAtPosition(&values->cursor, position.row, position.column, read_buffer, CHAR_CHUNK_SIZE_TSINPUT);
  return read_buffer;
}


void parse_tree(FileNode** root, History** history_frame, FileHighlightDatas* highlight_data, History** old_history_frame) {
  if (highlight_data->is_active == false)
    return;

  ParserContainer* parser = getParserForLanguage(&parsers, highlight_data->lang_name);

  Cursor cursor_root = moduloCursorR(*root, 1, 0);


  PayloadInternalReader reader_payload;
  reader_payload.file = NULL;
  reader_payload.size = 0;
  reader_payload.root = *root;
  reader_payload.cursor = cursor_root;

  TSInput input;
  input.encoding = TSInputEncodingUTF8;
  input.read = internalReaderForTree;
  input.payload = &reader_payload;

  clock_t t;
  t = clock();

  TSTree* old_tree = highlight_data->tree;
  highlight_data->tree = ts_parser_parse(
    parser->parser,
    highlight_data->tree,
    input
  );
  ts_tree_delete(old_tree);


  t = clock() - t;
  double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds

  fprintf(stderr, "fun() took %f seconds to execute \n", time_taken);

  *old_history_frame = *history_frame;
}
