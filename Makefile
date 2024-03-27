CC=clang
CFLAGS=-g -fsanitize=address# -Wall -Wextra -Werror -gdwarf-4
LDFLAGS += -fsanitize=address

executable=al
modules=data-structure/utf_8_extractor.o data-structure/file_structure.o

all: $(executable)

%.o : %.c %.h
	$(CC) -c $< -o $@

al: main.o $(modules)
	$(CC) $^ -o $@ $(LDFLAGS)
	
clean:
	rm -rf *.o | rm -rf $(executable) $(modules)