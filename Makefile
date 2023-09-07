CC := gcc
AR := ar
CFLAGS := -Wall -Wextra -Werror -O2 -ansi -std=gnu99
DEBUG := -DNDEBUG
INSTALL_PATH := /usr/local

LIB = simak65.a
HEADER = simak65.h
OBJ = addrmode.o alu.o bus.o decoder.o exec.o simak65.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(VERSION) $(DEBUG) -I.

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

all: $(LIB)

install:
	cp $(LIB) $(INSTALL_PATH)/lib/
	cp $(HEADER) $(INSTALL_PATH)/include/

clean:
	rm -f *.o $(LIB)

.PHONY: clean
.PHONY: install
