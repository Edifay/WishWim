#include "tree_query.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>


void printQueryLoadError(uint32_t error_offset, TSQueryError error_type) {
  switch (error_type) {
    case TSQueryErrorCapture:
      fprintf(stderr, "ERROR : Query Capture Error\n");
      break;
    case TSQueryErrorField:
      fprintf(stderr, "ERROR : Query Field Error\n");
      break;
    case TSQueryErrorLanguage:
      fprintf(stderr, "ERROR : Query Language Error\n");
      break;
    case TSQueryErrorNone:
      fprintf(stderr, "ERROR : No error.\n");
      break;
    case TSQueryErrorStructure:
      fprintf(stderr, "ERROR : Query Structure Error\n");
      break;
    case TSQueryErrorSyntax:
      fprintf(stderr, "ERROR : Query Syntax Error\n");
      break;
    case TSQueryErrorNodeType:
      fprintf(stderr, "ERROR : Query NodeType Error\n");
      break;
  }
  fprintf(stderr, "Syntax error at %d byte_offset.", error_offset);
}

bool eq(ProcessPredicatePayload payload, bool any, bool not) {
  String predicate_capture_name;
  predicates_consumeCapture(payload.stream, payload.query, &predicate_capture_name);

  TSQueryPredicateStepType last_node_type = predicates_peekType(payload.stream);
  if (last_node_type != TSQueryPredicateStepTypeString) {
    fprintf(stderr, "Predicates type '#eq? @capture_1 @capture_2' are not supported !\n");
    return false;
  }

  String string_to_match;
  predicates_consumeString(payload.stream, payload.query, &string_to_match);

  for (int i = 0; i < payload.qmatch.capture_count; i++) {
    String match_capture_name = getCaptureString(payload.query, payload.qmatch.captures[i].index);
    if (areStringEquals(predicate_capture_name, match_capture_name)) {
      TSNode current_node = payload.qmatch.captures[i].node;
      if (isStringEqualToNodeContent(payload.tmp, string_to_match, current_node) == (any != not)) {
        return any;
      }
    }
  }

  return !any;
}

bool any_of(ProcessPredicatePayload payload) {
  String predicate_capture_name;
  predicates_consumeCapture(payload.stream, payload.query, &predicate_capture_name);

  bool found = false;
  while (predicates_peekType(payload.stream) != TSQueryPredicateStepTypeDone) {
    String string_to_match;
    predicates_consumeString(payload.stream, payload.query, &string_to_match);

    if (found == true)
      continue;

    for (int i = 0; i < payload.qmatch.capture_count; i++) {
      String match_capture_name = getCaptureString(payload.query, payload.qmatch.captures[i].index);
      if (areStringEquals(predicate_capture_name, match_capture_name)) {
        TSNode current_node = payload.qmatch.captures[i].node;
        if (isStringEqualToNodeContent(payload.tmp, string_to_match, current_node) == true) {
          found = true;
          break;
        }
      }
    }
  }

  return found;
}

void getOrCreateRegex(ProcessPredicatePayload payload, String regex_pattern_string, uint32_t regex_id, regex_t* regex) {
  bool exist = getRegexForRegexId(&payload.regex_map, regex_id, regex);
  if (exist == false) {
    addRegexPatternToRegexMap(&payload.regex_map, regex_pattern_string.content, regex_id);
    exist = getRegexForRegexId(&payload.regex_map, regex_id, regex);
    assert(exist == true);
  }
}

bool match(ProcessPredicatePayload payload, bool any, bool not) {
  String predicate_capture_name;
  predicates_consumeCapture(payload.stream, payload.query, &predicate_capture_name);

  String regex_pattern_string;
  uint32_t regex_id = payload.stream->predicates[payload.stream->index].value_id;
  predicates_consumeString(payload.stream, payload.query, &regex_pattern_string);

  regex_t regex;
  getOrCreateRegex(payload, regex_pattern_string, regex_id, &regex);

  for (int i = 0; i < payload.qmatch.capture_count; i++) {
    String match_capture_name = getCaptureString(payload.query, payload.qmatch.captures[i].index);
    if (areStringEquals(predicate_capture_name, match_capture_name)) {
      TSNode current_node = payload.qmatch.captures[i].node;
      if (isRegexMatchingToNodeContent(payload.tmp, regex, current_node) == (any != not)) {
        regfree(&regex);
        return any;
      }
    }
  }

  return !any;
}


bool executeCurrentPredicate(ProcessPredicatePayload payload) {
  String str;
  predicates_consumeString(payload.stream, payload.query, &str);

  // No need to check the str.content length. Query parser assert that there is atleast a predicate with 1 letter.
  // Minimal predicate '#a?'.
  if (str.content[str.length - 1] != '?') {
    fprintf(stderr, "Unsupported predicates '?' : '%s'.\n", str.content);
    return false;
  }

  if (strncmp(str.content, "eq", str.length - 1) == 0) {
    return eq(payload, false, false);
  }
  if (strncmp(str.content, "not-eq", str.length - 1) == 0) {
    return eq(payload, false, true);
  }
  if (strncmp(str.content, "any-eq", str.length - 1) == 0) {
    return eq(payload, true, false);
  }
  if (strncmp(str.content, "any-not-eq", str.length - 1) == 0) {
    return eq(payload, true, true);
  }
  if (strncmp(str.content, "any-of", str.length - 1) == 0) {
    return any_of(payload);
  }
  if (strncmp(str.content, "match", str.length - 1) == 0) {
    return match(payload, false, false);
  }
  if (strncmp(str.content, "not-match", str.length - 1) == 0) {
    return match(payload, false, true);
  }
  if (strncmp(str.content, "any-match", str.length - 1) == 0) {
    return match(payload, true, false);
  }
  if (strncmp(str.content, "any-not-match", str.length - 1) == 0) {
    return match(payload, true, true);
  }


  fprintf(stderr, "Unsupported predicates : '%s'.\n", str.content);
  return false;
}


bool arePredicatesMatching(Cursor* tmp, TSQuery* query, TSQueryMatch qmatch, const TSQueryPredicateStep* predicates, uint32_t length, RegexMap regex_map) {
  PredicateStream stream = predicates_stream(predicates, length);
  ProcessPredicatePayload payload;
  payload.qmatch = qmatch;
  payload.query = query;
  payload.stream = &stream;
  payload.tmp = tmp;
  payload.regex_map = regex_map;

  while (predicates_hasNext(&stream)) {
    if (executeCurrentPredicate(payload) == false) {
      return false;
    }
    predicates_consumeEND(&stream);
  }

  return true;
}


bool TSQueryCursorNextMatchWithPredicates(Cursor* tmp, TSQuery* query, TSQueryCursor* qcursor, TSQueryMatch* qmatch, RegexMap regex_map) {
  TSQueryMatch _qmatch;
  while (ts_query_cursor_next_match(qcursor, &_qmatch)) {
    uint32_t length;
    const TSQueryPredicateStep* predicates = ts_query_predicates_for_pattern(query, _qmatch.pattern_index, &length);

    // Pattern don't contain any predicates. We can send it.
    if (length == 0) {
      *qmatch = _qmatch;
      return true;
    }

    // If predicates matching send it.
    if (arePredicatesMatching(tmp, query, _qmatch, predicates, length, regex_map)) {
      *qmatch = _qmatch;
      return true;
    }

    // If predicates don't match, process to the next match.
  }

  return false;
}


String getCaptureString(TSQuery* query, uint32_t index) {
  String str;
  str.content = ts_query_capture_name_for_id(query, index, &str.length);
  return str;
}

bool isStringEqualToNodeContent(Cursor* tmp, String str, TSNode node) {
  uint32_t content_length = ts_node_end_byte(node) - ts_node_start_byte(node);
  char content[content_length + 1];
  content[content_length] = '\0';

  TSPoint start_point = ts_node_start_point(node);
  readNBytesAtPosition(tmp, start_point.row, start_point.column, content, content_length);

  return str.length == content_length && strncmp(content, str.content, content_length) == 0;
}

bool isRegexMatchingToNodeContent(Cursor* tmp, regex_t regex, TSNode node) {
  uint32_t content_length = ts_node_end_byte(node) - ts_node_start_byte(node);
  char content[content_length + 1];
  content[content_length] = '\0';

  TSPoint start_point = ts_node_start_point(node);
  readNBytesAtPosition(tmp, start_point.row, start_point.column, content, content_length);

  regmatch_t reg_matchs[1];
  int match_result = regexec(&regex, content, 1, reg_matchs, 0);

  return match_result == 0;
}


PredicateStream predicates_stream(const TSQueryPredicateStep* self, uint32_t length) {
  PredicateStream stream;
  stream.index = 0;
  stream.length = length;
  stream.predicates = self;
  return stream;
}

TSQueryPredicateStep predicates_peek(const PredicateStream* self) {
  assert(predicates_hasNext(self));
  return self->predicates[self->index];
}

TSQueryPredicateStep predicates_consume(PredicateStream* self) {
  TSQueryPredicateStep step = predicates_peek(self);
  self->index++;
  return step;
}

TSQueryPredicateStepType predicates_peekType(const PredicateStream* self) {
  return predicates_peek(self).type;
}

TSQueryPredicateStep predicates_consumeCapture(PredicateStream* self, TSQuery* query, String* str) {
  TSQueryPredicateStep step = predicates_consume(self);
  assert(step.type == TSQueryPredicateStepTypeCapture);

  str->content = ts_query_capture_name_for_id(query, step.value_id, &str->length);
  return step;
}

TSQueryPredicateStep predicates_consumeString(PredicateStream* self, TSQuery* query, String* str) {
  TSQueryPredicateStep step = predicates_consume(self);
  assert(step.type == TSQueryPredicateStepTypeString);

  str->content = ts_query_string_value_for_id(query, step.value_id, &str->length);
  return step;
}

void predicates_consumeEND(PredicateStream* self) {
  TSQueryPredicateStep step = predicates_consume(self);
  assert(step.type == TSQueryPredicateStepTypeDone);
}

bool predicates_hasNext(const PredicateStream* self) {
  return self->index < self->length;
}
