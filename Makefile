CC=clang
CFLAGS=-g -fsanitize=address# -lncurses # -Wall -Wextra -Werror -gdwarf-4
LDFLAGS +=-fsanitize=address

executable=main.o test_line.o test_file.o al test_line test_file
modules=data-structure/utf_8_extractor.o data-structure/file_structure.o utils/tools.o io_management/file_manager.o

all: $(modules) $(executable)

test_file.o: test_file.c data-structure/utf_8_extractor.h data-structure/file_structure.h
	$(CC) $(CFLAGS) -c $< -o $@

test_line.o: test_line.c data-structure/utf_8_extractor.h data-structure/file_structure.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.c %.h data-structure/utf_8_extractor.h data-structure/file_structure.h
	$(CC) $(CFLAGS) -c $< -o $@

test_line: test_line.o $(modules)
	$(CC) $(CFLAGS) $^ -o $@

test_file: test_file.o $(modules)
	$(CC) $(CFLAGS) $^ -o $@

al: main.o $(modules)
	$(CC) $(CFLAGS) $^ -o $@ -lncurses

clean:
	rm -rf *.o | rm -rf $(executable) $(modules)