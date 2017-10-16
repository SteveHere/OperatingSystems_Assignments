CC = gcc
CFLAGS = -Wall -pedantic -ansi 
TARGETS = assignment

# rules

# default

# targets

all: $(TARGETS)

clean:
	rm -fr $(TARGETS) *~ *.o