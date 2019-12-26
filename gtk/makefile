# Make file for Scintilla on Linux, macOS, or Windows
# @file makefile
# Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
# The License.txt file describes the conditions under which this software may be distributed.
# This makefile assumes GCC 9.0+ is used and changes will be needed to use other compilers.
# Clang 9.0+ can be used with CLANG=1 on command line.
# Builds for GTK+ 2 and 3. GTK 3 requires GTK3=1 on command line.
# Also works with ming32-make on Windows.

.SUFFIXES: .cxx .c .o .h .a .list

srcdir ?= .

ifdef CLANG
CXX = clang++
CXXWARNFLAGS = -Wall -pedantic -Wno-deprecated-register
CC = clang
# Can choose aspect to sanitize: address and undefined can simply change SANITIZE but for
# thread also need to create Position Independent Executable -> search online documentation
SANITIZE = address
#SANITIZE = undefined
else
CXXWARNFLAGS = -Wall -pedantic
endif
ARFLAGS = rc
RANLIB ?= ranlib

ifdef GTK3
GTKVERSION=gtk+-3.0
else
GTKVERSION=gtk+-2.0
endif

# Environment variable windir always defined on Win32

# Enable Position Independent Code except on Windows where it is the default so the flag produces a warning
ifndef windir
PICFLAGS = -fPIC
endif

ifdef windir
CC = gcc
DEL = del /q
COMPLIB=$(srcdir)\..\bin\scintilla.a
else
DEL = rm -f
COMPLIB=$(srcdir)/../bin/scintilla.a
endif

vpath %.h $(srcdir) $(srcdir)/../src $(srcdir)/../include $(srcdir)/../lexlib
vpath %.c $(srcdir)
vpath %.cxx $(srcdir) $(srcdir)/../src $(srcdir)/../lexlib $(srcdir)/../lexers

INCLUDEDIRS=-I $(srcdir)/../include -I $(srcdir)/../src -I $(srcdir)/../lexlib
CXXBASEFLAGS=$(CXXWARNFLAGS) $(PICFLAGS) -DGTK -DSCI_LEXER $(INCLUDEDIRS)

ifdef NO_CXX11_REGEX
REFLAGS=-DNO_CXX11_REGEX
endif

ifdef DEBUG
ifdef CLANG
CTFLAGS=-DDEBUG -g -fsanitize=$(SANITIZE) $(CXXBASEFLAGS)
else
CTFLAGS=-DDEBUG -g $(CXXBASEFLAGS)
endif
else
CTFLAGS=-DNDEBUG -Os $(CXXBASEFLAGS)
endif

CXXTFLAGS:=--std=gnu++17 $(CTFLAGS) $(REFLAGS)

CONFIGFLAGS:=$(shell pkg-config --cflags $(GTKVERSION))
MARSHALLER=scintilla-marshal.o

all: $(COMPLIB)

clean:
	$(DEL) *.o $(COMPLIB) *.plist

.cxx.o:
	$(CXX) $(CONFIGFLAGS) $(CXXTFLAGS) $(CXXFLAGS) -c $<
.c.o:
	$(CC) $(CONFIGFLAGS) $(CTFLAGS) $(CFLAGS) -w -c $<

GLIB_GENMARSHAL = glib-genmarshal
GLIB_GENMARSHAL_FLAGS = --prefix=scintilla_marshal

.list.h:
	$(GLIB_GENMARSHAL) --header $(GLIB_GENMARSHAL_FLAGS) $< > $@
.list.c:
	$(GLIB_GENMARSHAL) --body $(GLIB_GENMARSHAL_FLAGS) $< > $@

LEXOBJS:=$(addsuffix .o,$(basename $(sort $(notdir $(wildcard $(srcdir)/../lexers/Lex*.cxx)))))

analyze:
	clang --analyze $(CONFIGFLAGS) $(CXXTFLAGS) $(CXXFLAGS) $(srcdir)/*.cxx $(srcdir)/../src/*.cxx $(srcdir)/../lexlib/*.cxx $(srcdir)/../lexers/*.cxx

depend deps.mak:
	python DepGen.py

# Required for base Scintilla
SRC_OBJS = \
	AutoComplete.o \
	CallTip.o \
	CaseConvert.o \
	CaseFolder.o \
	CellBuffer.o \
	CharacterCategory.o \
	CharacterSet.o \
	CharClassify.o \
	ContractionState.o \
	DBCS.o \
	Decoration.o \
	Document.o \
	EditModel.o \
	Editor.o \
	EditView.o \
	Indicator.o \
	KeyMap.o \
	LineMarker.o \
	MarginView.o \
	PerLine.o \
	PositionCache.o \
	RESearch.o \
	RunStyles.o \
	Selection.o \
	Style.o \
	UniConversion.o \
	UniqueString.o \
	ViewStyle.o \
	XPM.o

# Required by lexers
LEXLIB_OBJS = \
	Accessor.o \
	Catalogue.o \
	DefaultLexer.o \
	ExternalLexer.o \
	LexerBase.o \
	LexerModule.o \
	LexerSimple.o \
	PropSetSimple.o \
	StyleContext.o \
	WordList.o

GTK_OBJS = \
	ScintillaBase.o \
	PlatGTK.o \
	ScintillaGTK.o \
	ScintillaGTKAccessible.o

$(COMPLIB): $(SRC_OBJS) $(LEXLIB_OBJS) $(GTK_OBJS) $(MARSHALLER) $(LEXOBJS)
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

# Automatically generate header dependencies with "make deps"
include deps.mak
