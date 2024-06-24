#include "scm_parser.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void initTreePathSeq(TreePathSeq* seq) {
  assert(seq != NULL);
  seq->next = NULL;
  seq->value = NULL;
}


void destroyTreePathSeq(TreePathSeq* seq) {
  if (seq == NULL) {
    return;
  }
  destroyTreePath(seq->value);
  free(seq->value);
  seq = seq->next;
  while (seq != NULL) {
    TreePathSeq* tmp = seq;
    seq = seq->next;
    destroyTreePath(tmp->value);
    free(tmp->value);
    free(tmp);
  }
}

int lengthTreePathSeq(TreePathSeq* seq) {
  int count = 0;
  while (seq != NULL) {
    count++;
    seq = seq->next;
  }
  return count;
}

TreePathSeq* pushAfterNewTreePathSeqNode(TreePathSeq* seq) {
  TreePathSeq* new_val = malloc(sizeof(TreePathSeq));
  initTreePathSeq(new_val);
  seq->next = new_val;
  return new_val;
}


void pushSymbol(TreePathSeq* seq, char* name) {
  assert(seq != NULL);
  TreePath* tmp = seq->value;
  seq->value = malloc(sizeof(TreePath));
  seq->value->name = malloc(strlen(name) * sizeof(char) + 1);
  strcpy(seq->value->name, name);
  seq->value->type = SYMBOL;
  seq->value->next = tmp;
  seq->value->reg = NULL;
}

void pushField(TreePathSeq* seq, char* name) {
  assert(seq != NULL);
  TreePath* tmp = seq->value;
  seq->value = malloc(sizeof(TreePath));
  seq->value->name = malloc(strlen(name) * sizeof(char) + 1);
  strcpy(seq->value->name, name);
  seq->value->type = FIELD;
  seq->value->next = tmp;
  seq->value->reg = NULL;
}

void pushMatch(TreePathSeq* seq, char* name) {
  assert(seq != NULL);
  TreePath* tmp = seq->value;
  seq->value = malloc(sizeof(TreePath));
  seq->value->name = malloc(strlen(name) * sizeof(char) + 1);
  strcpy(seq->value->name, name);
  seq->value->type = MATCH;
  seq->value->next = tmp;
  seq->value->reg = NULL;
}


void pushGroup(TreePathSeq* seq, char* name) {
  assert(seq != NULL);
  TreePath* tmp = seq->value;
  seq->value = malloc(sizeof(TreePath));
  seq->value->name = malloc(strlen(name) * sizeof(char) + 1);
  strcpy(seq->value->name, name);
  seq->value->type = GROUP;
  seq->value->next = tmp;
  seq->value->reg = NULL;
}

void pushRegex(TreePathSeq* seq, char* name) {
  assert(seq != NULL);
  TreePath* tmp = seq->value;
  seq->value = malloc(sizeof(TreePath));
  seq->value->name = malloc(strlen(name) * sizeof(char) + 1);
  strcpy(seq->value->name, name);
  seq->value->type = REGEX;
  seq->value->next = tmp;
  seq->value->reg = malloc(sizeof(regex_t));
  int comp_res = regcomp(seq->value->reg, name, 0);
  if (comp_res) {
    printf("Error compiling regex %s\n.", name);
    exit(0);
  }
}


bool parseSCMFile(TreePathSeq* seq, char* file_name) {
  FILE* f = fopen(file_name, "r");
  if (f == NULL) {
    printf("Unable to parse %s file.\n\r", file_name);
    return false;
  }

  TreePathSeq* current_seq_node = seq;

  char c;
  int scan_res = fscanf(f, " %c", &c);
  int depth = 0;
  int index;
  char name[MAX_LENGTH_SYMBOL];
  if (scan_res != 1) {
    // File is empty
    return true;
  }
  while (1) {
    // Parser main loop
    // printf("Main loop %c\n", c);
    switch (c) {
      case ';': // Comment in file.
        while (c != '\n' || scan_res != 1) {
          // Skip the current line.
          scan_res = fscanf(f, "%c", &c);
        }
        break;

      case '(': // Opening a query

        if (current_seq_node->value != NULL) {
          // Begin a new query
          current_seq_node = pushAfterNewTreePathSeqNode(current_seq_node);
          assert(current_seq_node != NULL);
        }

        while (1) {
          // Query internal loop
          // printf("Query internal loop %c depth %d\n", c, depth);


          switch (c) {
            case '(': // Going deeper.
              depth++;
              scan_res = fscanf(f, "%c", &c);
              break;
            case ')': // Going upper.
              depth--;
              scan_res = fscanf(f, "%c", &c);
              break;
            case '@': // Add a group to the current query.
              index = 0;
            // read group name.
              while ((scan_res = fscanf(f, "%c", &c)) == 1 && c != '\n' && c != '\t' && c != ' ' && c != ')' && c != '(' && c != '@' && c != '#' && c != '"') {
                assert(index < MAX_LENGTH_SYMBOL);
                name[index] = c;
                index++;
              }
              name[index] = '\0';
              pushGroup(current_seq_node, name);
              break;
            case '#':
              while ((scan_res = fscanf(f, "%c", &c)) == 1 && c != '"'); // skip until reach the regex.
              if (scan_res != 1) {
                printf("Syntax error %s, in match.\n", file_name);
                return false;
              }
              scan_res = fscanf(f, "%c", &c); // Skip the '"' char.

            // Read the regex string.
              index = 0;
              while ((scan_res = fscanf(f, "%c", &c)) == 1 && c != '"') {
                assert(index < MAX_LENGTH_SYMBOL);
                name[index] = c;
                index++;
              }
              name[index] = '\0';
            // Push regex.
              pushRegex(current_seq_node, name);
              scan_res = fscanf(f, "%c", &c); // Skip the end '"' char.
              break;
            case '\n':
            case '\t':
            case ' ':
              while ((scan_res = fscanf(f, "%c", &c)) == 1 && (c == '\n' || c == '\t' || c == ' ')); // Skip empty chars.
              if (scan_res != 1) {
                // End of file.
                break;
              }
              break;
            default:
              // Read a field or a symbol.
              index = 1;
              name[0] = c;
            // Read name of the symbol/field.
              while ((scan_res = fscanf(f, "%c", &c)) == 1 && c != '\n' && c != '\t' && c != ' ' && c != ')' && c != '(' && c != '@' && c != '#' && c != '"') {
                assert(index < MAX_LENGTH_SYMBOL);
                name[index] = c;
                index++;
              }
              name[index] = '\0';
              if (scan_res != 1) {
                // If end is reached.
                printf("Syntax error %s.\n", file_name);
                exit(0);
              }
            // If it end with ':' it's a field.
              if (index >= 1 && name[index - 1] == ':') {
                name[index - 1] = '\0';
                pushField(current_seq_node, name);
              }
              else {
                pushSymbol(current_seq_node, name);
              }
              break;
          }
          // Breaking for char at depth == 0. (new query is detected)
          if ((c == '(' || c == '"' || c == '[' || c == ';') && depth == 0) {
            break;
          }
          // If end of file is reached.
          if (scan_res < 1) {
            break;
          }
        }
        break;
      case '[': // Openening a querie list
        TreePathSeq* intern_list = malloc(sizeof(TreePathSeq));
        initTreePathSeq(intern_list);
        scan_res = fscanf(f, "%c", &c);
        if (scan_res != 1) {
          printf("Syntax error, list not closed in file %s.\n", file_name);
          return false;
        }
        TreePathSeq* current_intern_list = intern_list;
        while (c != ']' && scan_res >= 1) {
          // Internal list loop
          switch (c) {
            case '"': // Match type
              if (current_intern_list->value != NULL) {
                // Begin of a new query
                current_intern_list = pushAfterNewTreePathSeqNode(current_intern_list);
                assert(current_intern_list != NULL);
              }
            // Getting the match string.
              index = 0;
              while ((scan_res = fscanf(f, "%c", &c)) == 1 && c != '"') {
                assert(index < MAX_LENGTH_SYMBOL);
                name[index] = c;
                index++;
              }
              name[index] = '\0';

            // Push the match.
              pushMatch(current_intern_list, name);
              if (c != '"') {
                // If the string doesn't end with a '"' the string is not closed.
                printf("Syntax error in file %s.\n", file_name);
                exit(0);
                return false;
              }
              scan_res = fscanf(f, "%c", &c);
              break;

            case '(': // Match type
              if (current_intern_list->value != NULL) {
                // Begin of a new query
                current_intern_list = pushAfterNewTreePathSeqNode(current_intern_list);
                assert(current_intern_list != NULL);
              }
            // Getting the match string.
              index = 0;
              while ((scan_res = fscanf(f, "%c", &c)) == 1 && c != ')') {
                assert(index < MAX_LENGTH_SYMBOL);
                name[index] = c;
                index++;
              }
              name[index] = '\0';
            // Push the match.
              pushSymbol(current_intern_list, name);
              if (c != ')') {
                // If the string doesn't end with a '"' the string is not closed.
                printf("Syntax error in file %s.\n", file_name);
                exit(0);
                return false;
              }
              scan_res = fscanf(f, "%c", &c);
              break;


            case '\n':
            case '\t':
            case ' ':
              // skip blanc
              while ((scan_res = fscanf(f, "%c", &c)) == 1 && (c == '\n' || c == '\t' || c == ' '));
              if (scan_res != 1) {
                // end of file reached.
                break;
              }
              break;
            default:
              printf("Syntax error ! %s\n", file_name);
              break;
          }
        }
        if (scan_res != 1) {
          printf("Syntax error, group not given for last array. %s.\n", file_name);
          return false;
        }

      // skip blank
        while ((scan_res = fscanf(f, "%c", &c)) == 1 && (c == '\n' || c == '\t' || c == ' '));
        if (scan_res != 1) {
          // end of file reached.
          break;
        }

        if (c != '@') {
          printf("Syntax error, need to give a group for the list. %s.\n", file_name);
          return false;
        }

      // Getting the group name
        index = 0;
        while ((scan_res = fscanf(f, "%c", &c)) == 1 && c != '\n' && c != '\t' && c != ' ' && c != ')' && c != '(' && c != '@' && c != '#' && c != '"') {
          assert(index < MAX_LENGTH_SYMBOL);
          name[index] = c;
          index++;
        }
        name[index] = '\0';
      // Push the group for the current match query
        current_intern_list = intern_list;
        while (1) {
          pushGroup(current_intern_list, name);
          if (current_intern_list->next == NULL) // Used to keep the last node of the intern list.
            break;
          current_intern_list = current_intern_list->next;
        }

      // append the current intern list to the main list
        current_seq_node->next = intern_list;
        current_seq_node = current_intern_list;
        break;


      case '"': // Match type
        if (current_seq_node->value != NULL) {
          // Begin of a new query
          current_seq_node = pushAfterNewTreePathSeqNode(current_seq_node);
          assert(current_seq_node != NULL);
        }
      // Getting the match string.
        index = 0;
        while ((scan_res = fscanf(f, "%c", &c)) == 1 && c != '"') {
          assert(index < MAX_LENGTH_SYMBOL);
          name[index] = c;
          index++;
        }
        name[index] = '\0';
      // Push the match.
        pushMatch(current_seq_node, name);
        if (c != '"') {
          // If the string doesn't end with a '"' the string is not closed.
          printf("Syntax error in file %s.\n", file_name);
          exit(0);
          return false;
        }
        scan_res = fscanf(f, "%c", &c);
        while ((scan_res = fscanf(f, "%c", &c)) == 1 && (c == '\n' || c == '\t' || c == ' ')) {
        }
        if (scan_res != 1) {
          printf("Syntax error %s. Didn't find id for \"%s\"\n", file_name, name);
          exit(0);
          return false;
        }
      // Assert that a group is given for the current match.
        if (c != '@') {
          printf("Syntax error in file %s.\n", file_name);
          exit(0);
          return false;
        }
      // Getting the group name
        index = 0;
        while ((scan_res = fscanf(f, "%c", &c)) == 1 && c != '\n' && c != '\t' && c != ' ' && c != ')' && c != '(' && c != '@' && c != '#' && c != '"') {
          assert(index < MAX_LENGTH_SYMBOL);
          name[index] = c;
          index++;
        }
        name[index] = '\0';
      // Push the group for the current match query
        pushGroup(current_seq_node, name);
        break;
      case '\n':
      case '\t':
      case ' ':
        // skip blanc
        while ((scan_res = fscanf(f, "%c", &c)) == 1 && (c == '\n' || c == '\t' || c == ' '));
        if (scan_res != 1) {
          // end of file reached.
          break;
        }
        break;
      default:
        printf("Syntax error %s\n", file_name);
        return false;
        break;
    }
    if (scan_res < 1) {
      break;
    }
  }


  fclose(f);
  return true;
}

void printTreePathSeq(TreePathSeq* seq) {
  TreePathSeq* current = seq;
  while (current != NULL) {
    printTreePath(current->value);
    printf("\n\n____________________\n\n");
    current = current->next;
  }
}


// Sorting by swap method.
void sortTreePathSeqByDecreasingSize(TreePathSeq* seq) {
  int length = lengthTreePathSeq(seq);

  for (int i = 0; i < length - 1; i++) {
    TreePathSeq* current_node = seq;
    for (int j = 0; j < length - 1; j++) {
      assert(current_node->next != NULL);
      if (lengthTreePath(current_node->value) < lengthTreePath(current_node->next->value)) {
        // Swap seq.value and seq.next.value
        TreePath* val = current_node->value;
        current_node->value = current_node->next->value;
        current_node->next->value = val;
      }
      current_node = current_node->next;
    }
  }
}
