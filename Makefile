CC=clang
CFLAGS=-g -fsanitize=address # -lncurses # -Wall -Wextra -Werror -gdwarf-4
LDFLAGS +=-fsanitize=address

executable=data-structure/term_handler.o main.o al #test_line.o test_file.o  test_line test_file  # utils/debug.o
modules= \
	data-structure/utf_8_extractor.o data-structure/file_structure.o data-structure/file_management.o utils/tools.o    \
	io_management/io_manager.o utils/key_management.o utils/clipboard_manager.o io_management/viewport_history.o         \
	data-structure/state_control.o data-structure/term_handler.o



all: $(modules) $(executable)

test_file.o: test_file.c data-structure/utf_8_extractor.h data-structure/file_structure.h
	$(CC) $(CFLAGS) -c $< -o $@

test_line.o: test_line.c data-structure/utf_8_extractor.h data-structure/file_structure.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.c %.h data-structure/utf_8_extractor.h data-structure/file_structure.h utils/constants.h
	$(CC) $(CFLAGS) -c $< -o $@

test_line: test_line.o $(modules)
	$(CC) $(CFLAGS) $^ -o $@

test_file: test_file.o $(modules)
	$(CC) $(CFLAGS) $^ -o $@

al: main.o $(modules)
	$(CC) $(CFLAGS) $^ -o $@ -lncursesw #utils/debug.o

clean:
	rm -rf *.o | rm -rf $(executable) $(modules)