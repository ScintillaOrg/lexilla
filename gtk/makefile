# Make file for Scintilla on Linux or compatible OS
# Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
# The License.txt file describes the conditions under which this software may be distributed.
# This makefile assumes GCC 2.95.2 is used and changes will be needed to use other compilers.
# GNU make does not like \r\n line endings so should be saved to CVS in binary form.

.SUFFIXES: .cxx .o .h .a
CC = g++
AR = ar

COMPLIB=../bin/scintilla.a

vpath %.h ../src ../include
vpath %.cxx ../src

INCLUDEDIRS=-I ../include -I ../src
CXXBASEFLAGS=-W -Wall -DGTK -DSCI_LEXER $(INCLUDEDIRS)

ifdef DEBUG
CXXFLAGS=-DDEBUG -g $(CXXBASEFLAGS)
else
CXXFLAGS=-DNDEBUG $(CXXBASEFLAGS)
endif

.cxx.o:
	$(CC) `gtk-config --cflags` $(CXXFLAGS) -c $<

LEXOBJS=LexAda.o LexAVE.o LexConf.o LexCPP.o LexEiffel.o LexHTML.o \
	LexLisp.o LexLua.o LexOthers.o LexPascal.o LexPerl.o \
	LexPython.o LexRuby.o LexSQL.o LexVB.o

# The LEXOBJS have to be treated specially as the functions in them are not called from external code

all: $(COMPLIB) $(LEXOBJS)

clean:
	rm -f *.o $(COMPLIB)

deps:
	$(CC) -MM `gtk-config --cflags` $(CXXFLAGS) *.cxx ../src/*.cxx >deps.mak

$(COMPLIB): DocumentAccessor.o WindowAccessor.o KeyWords.o StyleContext.o Document.o CallTip.o \
	ScintillaBase.o ContractionState.o Editor.o PropSet.o PlatGTK.o \
	KeyMap.o LineMarker.o ScintillaGTK.o CellBuffer.o ViewStyle.o \
	RESearch.o Style.o Indicator.o AutoComplete.o
	$(AR) rc $@ $^

# Automatically generate header dependencies with "make deps"
include deps.mak
