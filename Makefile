# cJSON : https://github.com/DaveGamble/cJSON?tab=readme-ov-file#cmake

CC=clang
CFLAGS=-g -fsanitize=address # -lncurses # -Wall -Wextra -Werror -gdwarf-4
LDFLAGS +=-fsanitize=address

executable=data-structure/term_handler.o main.o al lsp_test#test_line.o test_file.o  test_line test_file  # utils/debug.o
modules= \
	 \
	data-structure/utf_8_extractor.o data-structure/file_structure.o data-structure/file_management.o utils/tools.o    \
	io_management/io_manager.o utils/key_management.o utils/clipboard_manager.o io_management/viewport_history.o         \
	data-structure/state_control.o data-structure/term_handler.o io_management/io_explorer.o advanced/lsp/lsp_client.o \
	advanced/tree-sitter/scm_parser.o  advanced/tree-sitter/tree_manager.o\
	lib/tree-sitter/libtree-sitter.a \
	lib/tree-sitter-c/libtree-sitter-c.a \
	lib/tree-sitter-python/libtree-sitter-python.a


LIBS= -I lib/tree-sitter/lib/include -I lib/tree-sitter/lib/src


all: $(modules) $(executable)

test_file.o: test_file.c data-structure/utf_8_extractor.h data-structure/file_structure.h
	$(CC) $(CFLAGS) -c $< -o $@

test_line.o: test_line.c data-structure/utf_8_extractor.h data-structure/file_structure.h
	$(CC) $(CFLAGS) -c $< -o $@

lib/tree-sitter-c/libtree-sitter-c.a:
	cd lib/tree-sitter-c/ && tree-sitter generate && $(MAKE)

lib/tree-sitter-python/libtree-sitter-python.a:
	cd lib/tree-sitter-python/ && tree-sitter generate && $(MAKE)

lib/tree-sitter/libtree-sitter.a:
	cd lib/tree-sitter/ && $(MAKE)


%.o : %.c %.h data-structure/utf_8_extractor.h data-structure/file_structure.h utils/constants.h data-structure/file_management.h
	$(CC) $(CFLAGS) -c $< -o $@

test_line: test_line.o $(modules)
	$(CC) $(CFLAGS) $^ -o $@

test_file: test_file.o $(modules)
	$(CC) $(CFLAGS) $^ -o $@

al: main.o $(modules)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@ -lcjson -lncursesw #utils/debug.o

# cJSON : https://github.com/DaveGamble/cJSON?tab=readme-ov-file#cmake


lsp_test: lsp_test.c $(modules)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@ -lcjson -lncursesw

clean:
	rm -rf *.o | rm -rf $(executable) $(modules)