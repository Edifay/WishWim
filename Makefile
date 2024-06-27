# cJSON : https://github.com/DaveGamble/cJSON?tab=readme-ov-file#cmake

CC=clang
CFLAGS=-g  -fsanitize=address # -lncurses # -Wall -Wextra -Werror -gdwarf-4
LDFLAGS +=-fsanitize=address

executable= al #lsp_test #test_line.o test_file.o  test_line test_file  # utils/debug.o
modules= \
	src/data-management/utf_8_extractor.o src/data-management/file_structure.o src/data-management/file_management.o src/utils/tools.o    \
	src/io_management/io_manager.o src/utils/key_management.o src/utils/clipboard_manager.o src/io_management/viewport_history.o         \
	src/data-management/state_control.o src/terminal/term_handler.o src/io_management/io_explorer.o src/advanced/lsp/lsp_client.o \
	src/advanced/tree-sitter/scm_parser.o  src/advanced/tree-sitter/tree_manager.o src/advanced/theme.o src/terminal/highlight.o\
	src/terminal/click_handler.o src/config/config.o \
	\
	lib/tree-sitter/libtree-sitter.a \
	lib/tree-sitter-c/libtree-sitter-c.a \
	lib/tree-sitter-python/libtree-sitter-python.a \
#	lib/tree-sitter-c/src/parser.o \
#	lib/tree-sitter-python/src/parser.o  lib/tree-sitter-python/src/scanner.o\



LIBS= -I lib/tree-sitter/lib/include -I lib/tree-sitter/lib/src


all: $(modules) $(executable)

test_file.o: src/test_file.c
	$(CC) $(CFLAGS) -c $< -o $@

test_line.o: src/test_line.c
	$(CC) $(CFLAGS) -c $< -o $@

lib/tree-sitter-c/libtree-sitter-c.a:
	cd lib/tree-sitter-c/ && tree-sitter generate && $(MAKE)

lib/tree-sitter-python/libtree-sitter-python.a:
	cd lib/tree-sitter-python/ && tree-sitter generate && $(MAKE)

lib/tree-sitter/libtree-sitter.a:
	cd lib/tree-sitter/ && $(MAKE)

%.o : %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

test_line: src/test_line.c $(modules)
	$(CC) $(CFLAGS) $^ -o $@

test_file: src/test_file.c $(modules)
	$(CC) $(CFLAGS) $^ -o $@

# cJSON : https://github.com/DaveGamble/cJSON?tab=readme-ov-file#cmake

al: src/main.c $(modules)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@ -lcjson -lncursesw #utils/debug.o

lsp_test: src/lsp_test.c $(modules)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@ -lcjson -lncursesw

clean:
	rm -rf *.o | rm -rf $(executable) $(modules)