# cJSON : https://github.com/DaveGamble/cJSON?tab=readme-ov-file#cmake

CC=clang
CFLAGS=-g -O3 #-fsanitize=address # -lncurses # -Wall -Wextra -Werror -gdwarf-4
#LDFLAGS +=-fsanitize=address

executable= al #lsp_test #test_line.o test_file.o  test_line test_file  # utils/debug.o
modules= \
	src/data-management/utf_8_extractor.o src/data-management/file_structure.o src/data-management/file_management.o src/utils/tools.o    \
	src/io_management/io_manager.o src/utils/key_management.o src/utils/clipboard_manager.o src/io_management/viewport_history.o         \
	src/data-management/state_control.o src/terminal/term_handler.o src/io_management/io_explorer.o src/advanced/lsp/lsp_client.o \
	src/advanced/tree-sitter/scm_parser.o  src/advanced/tree-sitter/tree_manager.o src/advanced/theme.o src/terminal/highlight.o\
	src/terminal/click_handler.o src/config/config.o src/io_management/workspace_settings.o \
	\
	lib/tree-sitter/libtree-sitter.a \
	lib/tree-sitter-c/libtree-sitter-c.a \
	lib/tree-sitter-python/libtree-sitter-python.a \
	lib/tree-sitter-java/libtree-sitter-java.a \
	lib/tree-sitter-cpp/libtree-sitter-cpp.a \
	lib/tree-sitter-c-sharp/target/debug/libtree_sitter_c_sharp.rlib \
	lib/tree-sitter-make/target/debug/libtree_sitter_make.rlib \
	lib/tree-sitter-css/target/debug/libtree_sitter_css.rlib \
	lib/tree-sitter-dart/target/debug/libtree_sitter_dart.rlib \
	lib/tree-sitter-go/target/debug/libtree_sitter_go.rlib \
	lib/tree-sitter-javascript/target/debug/libtree_sitter_javascript.rlib \
	lib/tree-sitter-json/target/debug/libtree_sitter_json.rlib \
	lib/tree-sitter-bash/target/debug/libtree_sitter_bash.rlib \
	lib/tree-sitter-markdown/tree-sitter-markdown/libtree-sitter-markdown.a \
	lib/tree-sitter-markdown/tree-sitter-markdown-inline/libtree-sitter-markdown-inline.a \
	lib/tree-sitter-query/target/debug/libtree_sitter_query.rlib \
#	lib/tree-sitter-markdown/target/debug/libtree_sitter_markdown.rlib \
#	lib/tree-sitter-c/src/parser.o \
#	lib/tree-sitter-python/src/parser.o  lib/tree-sitter-python/src/scanner.o\
#	lib/tree-sitter-c-sharp/libtree-sitter-c-sharp.a \



LIBS= -I lib/tree-sitter/lib/include -I lib/tree-sitter/lib/src


all: $(modules) $(executable)

test_file.o: src/test_file.c
	$(CC) $(CFLAGS) -c $< -o $@

test_line.o: src/test_line.c
	$(CC) $(CFLAGS) -c $< -o $@

lib/tree-sitter-c/libtree-sitter-c.a:
	cd lib/tree-sitter-c/ && tree-sitter generate && $(MAKE)

lib/tree-sitter-cpp/libtree-sitter-cpp.a:
	cd lib/tree-sitter-cpp/ && $(MAKE)

lib/tree-sitter-c-sharp/target/debug/libtree_sitter_c_sharp.rlib:
	cd lib/tree-sitter-c-sharp/ && cargo build

lib/tree-sitter-make/target/debug/libtree_sitter_make.rlib:
	cd lib/tree-sitter-make/ && cargo build

lib/tree-sitter-css/target/debug/libtree_sitter_css.rlib:
	cd lib/tree-sitter-css/ && cargo build

lib/tree-sitter-dart/target/debug/libtree_sitter_dart.rlib:
	cd lib/tree-sitter-dart/ && cargo build

lib/tree-sitter-go/target/debug/libtree_sitter_go.rlib:
	cd lib/tree-sitter-go/ && cargo build

lib/tree-sitter-javascript/target/debug/libtree_sitter_javascript.rlib:
	cd lib/tree-sitter-javascript/ && cargo build

lib/tree-sitter-json/target/debug/libtree_sitter_json.rlib:
	cd lib/tree-sitter-json/ && cargo build

lib/tree-sitter-bash/target/debug/libtree_sitter_bash.rlib:
	cd lib/tree-sitter-bash/ && cargo build

lib/tree-sitter-markdown/target/debug/libtree_sitter_markdown.rlib:
	cd lib/tree-sitter-markdown && cargo build

lib/tree-sitter-query/target/debug/libtree_sitter_query.rlib:
	cd lib/tree-sitter-query/ && cargo build

lib/tree-sitter-python/libtree-sitter-python.a:
	cd lib/tree-sitter-python/ && tree-sitter generate && $(MAKE)

lib/tree-sitter-java/libtree-sitter-java.a:
	cd lib/tree-sitter-java/ && tree-sitter generate && $(MAKE)

lib/tree-sitter/libtree-sitter.a:
	cd lib/tree-sitter/ && $(MAKE)

lib/tree-sitter-markdown/tree-sitter-markdown/libtree-sitter-markdown.a:
	cd lib/tree-sitter-markdown/tree-sitter-markdown/ && tree-sitter generate && $(MAKE)

lib/tree-sitter-markdown/tree-sitter-markdown-inline/libtree-sitter-markdown-inline.a:
	cd lib/tree-sitter-markdown/tree-sitter-markdown-inline/ && tree-sitter generate && $(MAKE)

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