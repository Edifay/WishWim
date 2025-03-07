#include "tree_manager.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <bits/time.h>
#include <linux/limits.h>

#include "../../terminal/highlight.h"
#include "../../utils/constants.h"
#include "../../../lib/tree-sitter/lib/include/tree_sitter/api.h"
#include "../../utils/tools.h"
#include "../tree-sitter/tree_query.h"


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
    ts_query_delete(list->list[i].queries);
    ts_query_cursor_delete(list->list[i].cursor);
    destroyThemeList(&list->list[i].theme_list);
  }
  free(list->list);
  list->list = NULL;
  list->size = 0;
}

ParserContainer* getParserForLanguage(ParserList* list, char* language) {
  for (int i = 0; i < list->size; i++) {
    if (strcmp(list->list[i].lang_id, language) == 0) {
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

TSQuery* loadQueries(const ParserContainer* container, char path[PATH_MAX], uint32_t* error_offset, TSQueryError* error_type) {
  long length;
  char* source = loadFullFile(path, &length);
  TSQuery* queries = ts_query_new(container->lang, source, length, error_offset, error_type);
  free(source);
  return queries;
}

void getTSLanguageFromString(const TSLanguage** lang, char* language) {
  // ADD_NEW_LANGUAGE
  if (strcmp(language, "c") == 0) {
    *lang = tree_sitter_c();
  }
  else if (strcmp(language, "python") == 0) {
    *lang = tree_sitter_python();
  }
  else if (strcmp(language, "markdown") == 0) {
    *lang = tree_sitter_markdown();
  }
  else if (strcmp(language, "java") == 0) {
    *lang = tree_sitter_java();
  }
  else if (strcmp(language, "cpp") == 0) {
    *lang = tree_sitter_cpp();
  }
  else if (strcmp(language, "c-sharp") == 0) {
    *lang = tree_sitter_c_sharp();
  }
  else if (strcmp(language, "make") == 0) {
    *lang = tree_sitter_make();
  }
  else if (strcmp(language, "css") == 0) {
    *lang = tree_sitter_css();
  }
  else if (strcmp(language, "dart") == 0) {
    *lang = tree_sitter_dart();
  }
  else if (strcmp(language, "go") == 0) {
    *lang = tree_sitter_go();
  }
  else if (strcmp(language, "javascript") == 0) {
    *lang = tree_sitter_javascript();
  }
  else if (strcmp(language, "json") == 0) {
    *lang = tree_sitter_json();
  }
  else if (strcmp(language, "bash") == 0) {
    *lang = tree_sitter_bash();
  }
  else if (strcmp(language, "query") == 0) {
    *lang = tree_sitter_query();
  }
  else if (strcmp(language, "vhdl") == 0) {
    *lang = tree_sitter_vhdl();
  }
  else if (strcmp(language, "lua") == 0) {
    *lang = tree_sitter_lua();
  }
}

bool hasTSLanguageImplementation(char* language) {
  // ADD_NEW_LANGUAGE
  return strcmp(language, "c") == 0 ||
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
         strcmp(language, "query") == 0 ||
         strcmp(language, "lua") == 0 ||
         strcmp(language, "vhdl") == 0;
}


bool loadNewParser(ParserContainer* container, char* language) {
  if (!hasTSLanguageImplementation(language)) {
    return false;
  }
  // Set file name
  strcpy(container->lang_id, language);

  // Getting TSLanguage
  getTSLanguageFromString(&container->lang, language);

  // Fetching the folder where to load theme and queries.
  char* load_path = cJSON_GetStringValue(cJSON_GetObjectItem(config, "default_path"));

  char path[PATH_MAX];
  // Loading Theme
  sprintf(path, "%s/theme", load_path);

  bool load_result = getThemeFromFile(path, &container->theme_list);
  if (load_result == false) {
    ts_language_delete(container->lang);
    fprintf(stderr, "Unable to load theme from file '%s'. You can edit path in config file.\n", path);
    return false;
  }

  // Queries
  sprintf(path, "%s/queries/highlights-%s.scm", load_path, container->lang_id);

  uint32_t error_offset;
  TSQueryError error_type;
  container->queries = loadQueries(container, path, &error_offset, &error_type);
  if (container->queries == NULL) {
    ts_language_delete(container->lang);
    destroyThemeList(&container->theme_list);
    fprintf(stderr, "Unable to load queries from file '%s'. You can edit path in config file.\n", path);
    printQueryLoadError(error_offset, error_type);
    return false;
  }

  // Allocating a TSQueryCursor
  container->cursor = ts_query_cursor_new();

  // Post processing
  initColorsForTheme(container->theme_list, &color_index, &color_pair);

  container->parser = ts_parser_new();
  ts_parser_set_language(container->parser, container->lang);
  return true;
}


void setFileHighlightDatas(FileHighlightDatas* data, IO_FileID io_file) {
  bool did_lang_was_found = getLanguageStringIDForFile(data->lang_id, io_file);

  ParserContainer* parser = NULL;
  if (did_lang_was_found == true) {
    parser = getParserForLanguage(&parsers, data->lang_id);
  }

  data->is_active = parser != NULL;
  data->tree = NULL;
}


PayloadStateChange getPayloadStateChange(FileHighlightDatas* highlight_datas) {
  PayloadStateChange payload;
  payload.highlight_datas = highlight_datas;
  return payload;
}

void onStateChangeTS(Action action, long* payload_p) {
  PayloadStateChange payload = *(PayloadStateChange *)payload_p;

  if (payload.highlight_datas->is_active == false) {
    return;
  }

  TSInputEdit edit;
  switch (action.action) {
    case INSERT:
      // system("echo \"=== INSERT ===\" >> tree_logs.txt");
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
      // system("echo \"=== DELETE ===\" >> tree_logs.txt");
      assert(action.byte_start != -1);
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
      // system("echo \"=== DELETE_ONE ===\" >> tree_logs.txt");
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

  FILE *f = fopen("tree_logs.txt", "a");
  fprintf(f, obj_text);
  fprintf(f,"\n");
  fclose(f);


  free(obj_text);
  cJSON_Delete(obj);*/

  ts_tree_edit(payload.highlight_datas->tree, &edit);
}

char read_buffer[CHAR_CHUNK_SIZE_TSINPUT * 4];

const char* internalReaderForTree(void* payload, uint32_t byte_index, TSPoint position, uint32_t* bytes_read) {
  PayloadInternalReader* values = payload;
  // fprintf(stderr, "READ FROM READER\n");
  *bytes_read = readNu8CharAtPosition(&values->cursor, position.row, position.column, read_buffer, CHAR_CHUNK_SIZE_TSINPUT);
  return read_buffer;
}


void parse_tree(FileNode** root, History** history_frame, FileHighlightDatas* highlight_data, History** old_history_frame) {
  if (highlight_data->is_active == false)
    return;

  ParserContainer* parser = getParserForLanguage(&parsers, highlight_data->lang_id);

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

  // fprintf(stderr, "parse() took %f seconds to execute \n", time_taken);

  *old_history_frame = *history_frame;
}
