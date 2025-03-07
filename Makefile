CC=clang
CFLAGS=-g -O3 # -fsanitize=address # -lncurses # -Wall -Wextra -Werror -gdwarf-4
#CFLAGS=-g -fsanitize=address # -lncurses # -Wall -Wextra -Werror -gdwarf-4

executable= al # lsp_test #test_line.o test_file.o  test_line test_file  # utils/debug.o

MODULES= \
	src/data-management/utf_8_extractor.o \
	src/data-management/file_structure.o \
	src/data-management/file_management.o \
	src/utils/tools.o \
	src/io_management/io_manager.o \
	src/utils/key_management.o \
	src/utils/clipboard_manager.o \
	src/io_management/viewport_history.o \
	src/data-management/state_control.o \
	src/terminal/term_handler.o \
	src/io_management/io_explorer.o \
	src/advanced/lsp/lsp_client.o \
	src/advanced/tree-sitter/tree_manager.o \
	src/advanced/tree-sitter/tree_query.o \
	src/advanced/theme.o \
	src/terminal/highlight.o \
	src/terminal/click_handler.o \
	src/config/config.o \
	src/io_management/workspace_settings.o \
	src/advanced/lsp/lsp_handler.o

LIBS_MODULES= \
	lib/cJSON/cJSON.o \
	\
	lib/tree-sitter/lib/src/lib.o \
	lib/tree-sitter-c/target/debug/libtree_sitter_c.rlib \
	lib/tree-sitter-python/target/debug/libtree_sitter_python.rlib \
	lib/tree-sitter-java/target/debug/libtree_sitter_java.rlib \
	lib/tree-sitter-cpp/target/debug/libtree_sitter_cpp.rlib \
	lib/tree-sitter-c-sharp/target/debug/libtree_sitter_c_sharp.rlib \
	lib/tree-sitter-make/target/debug/libtree_sitter_make.rlib \
	lib/tree-sitter-css/target/debug/libtree_sitter_css.rlib \
	lib/tree-sitter-dart/target/debug/libtree_sitter_dart.rlib \
	lib/tree-sitter-go/target/debug/libtree_sitter_go.rlib \
	lib/tree-sitter-javascript/target/debug/libtree_sitter_javascript.rlib \
	lib/tree-sitter-json/target/debug/libtree_sitter_json.rlib \
	lib/tree-sitter-bash/target/debug/libtree_sitter_bash.rlib \
	lib/tree-sitter-markdown/target/debug/libtree_sitter_md.rlib \
	lib/tree-sitter-query/target/debug/libtree_sitter_query.rlib \
	lib/tree-sitter-vhdl/target/debug/libtree_sitter_vhdl.rlib \
	lib/tree-sitter-lua/target/debug/libtree_sitter_lua.rlib

ALL_MODULES= $(MODULES) $(LIBS_MODULES)

all: $(MODULES) $(executable)


lib/tree-sitter-markdown/tree-sitter-markdown/libtree-sitter-markdown.a:
	cd lib/tree-sitter-markdown/tree-sitter-markdown/ && tree-sitter generate && $(MAKE)

lib/tree-sitter-markdown/tree-sitter-markdown-inline/libtree-sitter-markdown-inline.a:
	cd lib/tree-sitter-markdown/tree-sitter-markdown-inline/ && tree-sitter generate && $(MAKE)

%.rlib:
	cd  $(shell echo $@ | cut -d/ -f1-2) && cargo build

%.o : %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

test_line: src/test_line.c $(ALL_MODULES)
	$(CC) $(CFLAGS) $^ -o $@ -lncursesw

test_file: src/test_file.c $(ALL_MODULES)
	$(CC) $(CFLAGS) $^ -o $@ -lncursesw

al: src/main.c $(ALL_MODULES)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@ -lncursesw #utils/debug.o

lsp_test: src/lsp_test.c $(ALL_MODULES)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@ -lncursesw

clean:
	rm -rf *.o | rm -rf $(executable) $(MODULES)

clean_all:
	rm -rf *.o | rm -rf $(executable) $(ALL_MODULES)
