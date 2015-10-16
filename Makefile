
.PHONY: clean distclean count

# OS type: Linux/Win DJGPP
ifdef OS
   EXE=.exe
else
   EXE=
endif

CFILES   = symbol.cpp error.cpp general.cpp final.cpp opt.cpp
HFILES   = symbol.h error.h general.h
OBJFILES = $(patsubst %.cpp,%.o,$(CFILES))
SRCFILES = $(HFILES) $(CFILES)

CC=g++
CFLAGS2=-Wall -pedantic -g -std=c++0x -Wno-psabi
CFLAGS=-g -Wall

all: pazcal$(EXE)

parser.o : parser.cpp
	$(CC) $(CFLAGS2) -c $<

error.o : error.cpp general.h error.h
	$(CC) $(CFLAGS2) -c $<

general.o : general.cpp general.h error.h
	$(CC) $(CFLAGS2) -c $<

final.o: final.cpp final.h general.h error.h
	$(CC) $(CFLAGS2) -c $<

opt.o: opt.cpp opt.h general.h error.h
	$(CC) $(CFLAGS2) -c $<

symbol.o : symbol.cpp symbol.h general.h error.h
	$(CC) $(CFLAGS2) -c $<

pazcal$(EXE): lexer.o parser.o symbol.o error.o final.o general.o opt.o
	$(CC) $(CFLAGS) -o $@ $^ -lfl

lexer.cpp: lexer.l
	flex -s -o $@ $<

parser.cpp parser.hpp: parser.y symbol.o error.o general.o final.o opt.o
	bison -v -d -o parser.cpp $<

lexer.o: lexer.cpp parser.hpp
	$(CC) $(CFLAGS2) -c $<

clean:
	$(RM) $(OBJFILES) lexer.cpp parser.cpp parser.hpp parser.output *.o *~

distclean: clean
	$(RM) $(EXEFILES) pazcal$(EXE)

count:
	wc -l -c Makefile $(SRCFILES)
