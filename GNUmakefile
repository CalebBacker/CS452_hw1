# Rules shared by all assignments.
# A subdirectory's GNUmakefile sets prog=<name>, includes this file,
# and then adds whatever targets are peculiar to it.
#
#   make          build ./try, the test driver
#   make test     build and run the test suite
#   make valgrind run the test suite under valgrind
#   make clean    remove everything built

CC     = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -g -fPIC

# Shared libraries are named differently, and located differently at
# run time, on Linux (onyx) and macOS.
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
  SOFLAGS = -dynamiclib -Wl,-install_name,@rpath/lib$(prog).so
else
  SOFLAGS = -shared
endif

.PHONY: all test valgrind clean

all: try

lib$(prog).so: $(prog).o
	$(CC) $(SOFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

test: try
	./try

valgrind: try
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
	         --error-exitcode=1 ./try

clean:
	rm -f *.o lib$(prog).so try
