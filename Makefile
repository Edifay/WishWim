CC=clang
CFLAGS=-g -fsanitize=address# -Wall -Wextra -Werror -gdwarf-4
LDFLAGS += -fsanitize=address

executable=al
modules=main.o data-structure/utf_8_extractor.o data-structure/file_structure.o

all: $(modules) $(executable)

main.o: main.c data-structure/utf_8_extractor.h data-structure/file_structure.h
	$(CC) -c $< -o $@ $(LDFLAGS)
    
%.o : %.c %.h data-structure/utf_8_extractor.h data-structure/file_structure.h
	$(CC) -c $< -o $@ $(LDFLAGS)

al: main.o $(modules) 
	$(CC) $^ -o $@ $(LDFLAGS)
	
clean:
	rm -rf *.o | rm -rf $(executable) $(modules)