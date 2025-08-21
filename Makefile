CC=gcc
CFLAGS=-c -Wall -DRS232 -Wno-unused-function
LDFLAGS=-s
SOURCES=main.c hex_bin.c cmd.c page.c io.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=./bin/prg_stm32

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *.o .obj/*
	
.PHONY: all clean

