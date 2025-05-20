# ChatGPT created this Makefile. 

CC=gcc
CFLAGS=-Wall -Wextra -Wno-unused-variable -std=c99
SRC=main.c db.c sqlite3.c
OBJ=$(SRC:.c=.o)
EXEC=main

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-del /q *.o *.exe 2>nul || rm -f *.o $(EXEC)
