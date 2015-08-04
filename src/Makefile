APPNAME=miniweath

CC=gcc
CFLAGS=-c -Wall -Os -flto
LDFLAGS=-s

SOURCES:=miniweath.c jsmn/jsmn.c
OBJECTS:=$(SOURCES:.c=.o)

all: jsmn $(APPNAME)

jsmn:
	hg clone https://bitbucket.org/zserge/jsmn jsmn

$(APPNAME): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf *.o $(APPNAME) ./jsmn