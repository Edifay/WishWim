CC=clang
CFLAGS=-g -fsanitize=address# -Wall -Wextra -Werror -gdwarf-4
LDFLAGS += -fsanitize=address #-lncurses

executable=main.o test_line.o al test_line
modules=data-structure/utf_8_extractor.o data-structure/file_structure.o utils/tools.o

all: $(modules) $(executable)

main.o: main.c data-structure/utf_8_extractor.h data-structure/file_structure.h
	$(CC) -c $< -o $@ $(LDFLAGS)

test_line.o: test_line.c data-structure/utf_8_extractor.h data-structure/file_structure.h
	$(CC) -c $< -o $@ $(LDFLAGS)
    
%.o : %.c %.h data-structure/utf_8_extractor.h data-structure/file_structure.h
	$(CC) -c $< -o $@ $(LDFLAGS)

test_line: test_line.o $(modules)
	$(CC) $^ -o $@ $(LDFLAGS)

al: main.o $(modules)
	$(CC) $^ -o $@ $(LDFLAGS)
	
clean:
	rm -rf *.o | rm -rf $(executable) $(modules)