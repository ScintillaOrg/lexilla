# Make file for Scintilla on Windows
# Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
# The License.txt file describes the conditions under which this software may be distributed.
# This makefile assumes the mingw32 version of GCC 2.95.2 is used and changes will
# be needed to use other compilers.

.SUFFIXES: .cxx
CC = g++
DLLWRAP = dllwrap

COMPONENT = ../bin/Scintilla.dll
LEXCOMPONENT = ../bin/SciLexer.dll

vpath %.h ../src ../include
vpath %.cxx ../src

LDFLAGS = -lkernel32 -luser32 -lgdi32 -limm32 -lole32 -luuid
# Add -MMD to get dependencies
#CXXFLAGS = -g -pg -pedantic -Os -fno-exceptions -fvtable-thunks -fno-rtti
INCLUDEDIRS=-I ../include -I ../src
CXXBASEFLAGS = -W -Wall -pedantic $(INCLUDEDIRS) -Os -fno-exceptions -fvtable-thunks -fno-rtti

ifdef DEBUG
CXXFLAGS=-DDEBUG $(CXXBASEFLAGS)
else
CXXFLAGS=-DNDEBUG $(CXXBASEFLAGS)
endif

.cxx.o:
	$(CC) $(CXXFLAGS) -c $<

ALL:	$(COMPONENT) $(LEXCOMPONENT) ScintillaWinS.o WindowAccessor.o

clean:
	del /q *.exe *.o *.obj *.dll *.res *.map

deps:
	$(CC) -MM $(CXXFLAGS) *.cxx ../src/*.cxx >deps.mak

LEXOBJS=LexAda.o LexAVE.o LexConf.o LexCPP.o LexEiffel.o LexHTML.o \
	LexLisp.o LexLua.o LexOthers.o LexPascal.o LexPerl.o \
	LexPython.o LexRuby.o LexSQL.o LexVB.o

SOBJS	= ScintillaWin.o ScintillaBase.o Editor.o Document.o \
	ContractionState.o CellBuffer.o CallTip.o \
	ScintRes.o PlatWin.o KeyMap.o Indicator.o LineMarker.o RESearch.o \
	Style.o ViewStyle.o AutoComplete.o UniConversion.o PropSet.o
$(COMPONENT): $(SOBJS)
	$(DLLWRAP) --target i386-mingw32 -o $@ $(SOBJS) $(LDFLAGS) -s --relocatable

LOBJS	= ScintillaWinL.o ScintillaBaseL.o Editor.o Document.o \
	ContractionState.o CellBuffer.o CallTip.o \
	ScintRes.o PlatWin.o KeyMap.o Indicator.o LineMarker.o RESearch.o \
	Style.o ViewStyle.o AutoComplete.o UniConversion.o KeyWords.o \
	DocumentAccessor.o PropSet.o ExternalLexer.o StyleContext.o $(LEXOBJS)
$(LEXCOMPONENT): $(LOBJS)
	$(DLLWRAP) --target i386-mingw32 -o $@ $(LOBJS) $(LDFLAGS) -s --relocatable

# Automatically generate dependencies for most files with "make deps"
include deps.mak

# These dependencies are maintained by hand as they do not use the default output name
ScintillaBaseL.o: ScintillaBase.cxx Platform.h Scintilla.h SciLexer.h \
 ContractionState.h CellBuffer.h CallTip.h KeyMap.h Indicator.h \
 LineMarker.h Style.h AutoComplete.h ViewStyle.h Document.h Editor.h \
 ScintillaBase.h PropSet.h SString.h Accessor.h DocumentAccessor.h \
 KeyWords.h ExternalLexer.h
ScintillaWinL.o: ScintillaWin.cxx Platform.h Scintilla.h SciLexer.h \
 ContractionState.h CellBuffer.h CallTip.h KeyMap.h Indicator.h \
 LineMarker.h Style.h AutoComplete.h ViewStyle.h Document.h Editor.h \
 ScintillaBase.h PropSet.h SString.h Accessor.h KeyWords.h \
 ExternalLexer.h UniConversion.h
ScintillaWinS.o: ScintillaWin.cxx Platform.h Scintilla.h \
 ContractionState.h CellBuffer.h CallTip.h KeyMap.h Indicator.h \
 LineMarker.h Style.h AutoComplete.h ViewStyle.h Document.h Editor.h \
 ScintillaBase.h UniConversion.h

ScintillaBaseL.o:
	$(CC) $(CXXFLAGS) -D SCI_LEXER -c $< -o $@

ScintillaWinS.o:
	$(CC) $(CXXFLAGS) -D STATIC_BUILD -c $< -o $@

ScintillaWinL.o:
	$(CC) $(CXXFLAGS) -D SCI_LEXER -c $< -o $@

ScintRes.o:	ScintRes.rc PlatformRes.h
	windres ScintRes.rc $@

