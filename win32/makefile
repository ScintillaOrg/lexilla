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
	$(CC) $(CXXFLAGS) -c $< -o $@

ALL:	$(COMPONENT) $(LEXCOMPONENT) ScintillaWinS.o WindowAccessor.o

clean:
	del /q *.exe *.o *.obj *.dll *.res *.map

LEXOBJS	= LexAda.o LexAVE.o LexConf.o LexCPP.o LexHTML.o LexLua.o \
	LexOthers.o LexPascal.o LexPerl.o LexPython.o LexSQL.o LexVB.o

SOBJS	= ScintillaWin.o ScintillaBase.o Editor.o Document.o \
	ContractionState.o CellBuffer.o CallTip.o \
	ScintRes.o PlatWin.o KeyMap.o Indicator.o LineMarker.o RESearch.o \
	Style.o ViewStyle.o AutoComplete.o UniConversion.o
$(COMPONENT): $(SOBJS)
	$(DLLWRAP) --target i386-mingw32 -o $@ $(SOBJS) $(LDFLAGS) -s --relocatable

LOBJS	= ScintillaWinL.o ScintillaBaseL.o Editor.o Document.o \
	ContractionState.o CellBuffer.o CallTip.o \
	ScintRes.o PlatWin.o KeyMap.o Indicator.o LineMarker.o RESearch.o \
	Style.o ViewStyle.o AutoComplete.o UniConversion.o KeyWords.o \
	DocumentAccessor.o PropSet.o $(LEXOBJS)
$(LEXCOMPONENT): $(LOBJS)
	$(DLLWRAP) --target i386-mingw32 -o $@ $(LOBJS) $(LDFLAGS) -s --relocatable

# All lexers depend on this set of headers
LEX_HEADERS=Platform.h PropSet.h SString.h Accessor.h \
 KeyWords.h Scintilla.h WinDefs.h SciLexer.h

AutoComplete.o: AutoComplete.cxx Platform.h AutoComplete.h
CallTip.o: CallTip.cxx Platform.h CallTip.h
CellBuffer.o: CellBuffer.cxx Platform.h Scintilla.h CellBuffer.h
ContractionState.o: ContractionState.cxx Platform.h ContractionState.h
Document.o: Document.cxx Platform.h Scintilla.h RESearch.h CellBuffer.h \
 Document.h
DocumentAccessor.o: DocumentAccessor.cxx Platform.h PropSet.h \
 SString.h Accessor.h DocumentAccessor.h Scintilla.h
Editor.o: Editor.cxx Platform.h Scintilla.h ContractionState.h \
 CellBuffer.h KeyMap.h Indicator.h LineMarker.h Style.h ViewStyle.h \
 Document.h Editor.h
Indicator.o: Indicator.cxx Platform.h Scintilla.h Indicator.h
KeyMap.o: KeyMap.cxx Platform.h Scintilla.h KeyMap.h
LexAda.o: LexAVE.cxx $(LEX_HEADERS)
LexAVE.o: LexAVE.cxx $(LEX_HEADERS)
LexConf.o: LexConf.cxx $(LEX_HEADERS)
LexCPP.o: LexCPP.cxx $(LEX_HEADERS)
LexHTML.o: LexHTML.cxx $(LEX_HEADERS)
LexLua.o: LexLua.cxx $(LEX_HEADERS)
LexOthers.o: LexOthers.cxx $(LEX_HEADERS)
LexPascal.o: LexPascal.cxx $(LEX_HEADERS)
LexPerl.o: LexPerl.cxx $(LEX_HEADERS)
LexPython.o: LexPython.cxx $(LEX_HEADERS)
LexSQL.o: LexSQL.cxx $(LEX_HEADERS)
LexVB.o: LexVB.cxx $(LEX_HEADERS)
KeyWords.o: KeyWords.cxx Platform.h PropSet.h SString.h Accessor.h KeyWords.h \
 Scintilla.h SciLexer.h
LineMarker.o: LineMarker.cxx Platform.h Scintilla.h LineMarker.h
PlatWin.o: PlatWin.cxx Platform.h PlatformRes.h UniConversion.h
PropSet.o: PropSet.cxx Platform.h PropSet.h SString.h
RESearch.o: RESearch.cxx RESearch.h
ScintillaBase.o: ScintillaBase.cxx Platform.h Scintilla.h \
 ContractionState.h CellBuffer.h CallTip.h KeyMap.h Indicator.h \
 LineMarker.h Style.h ViewStyle.h AutoComplete.h Document.h Editor.h \
 ScintillaBase.h
ScintillaBaseL.o: ScintillaBase.cxx Platform.h Scintilla.h SciLexer.h \
 ContractionState.h CellBuffer.h CallTip.h KeyMap.h Indicator.h \
 LineMarker.h Style.h AutoComplete.h ViewStyle.h Document.h Editor.h \
 ScintillaBase.h PropSet.h SString.h Accessor.h DocumentAccessor.h KeyWords.h
ScintillaWin.o: ScintillaWin.cxx Platform.h Scintilla.h \
 ContractionState.h CellBuffer.h CallTip.h KeyMap.h Indicator.h \
 LineMarker.h Style.h AutoComplete.h ViewStyle.h Document.h Editor.h \
 ScintillaBase.h UniConversion.h
ScintillaWinL.o: ScintillaWin.cxx Platform.h Scintilla.h SciLexer.h \
 ContractionState.h CellBuffer.h CallTip.h KeyMap.h Indicator.h \
 LineMarker.h Style.h AutoComplete.h ViewStyle.h Document.h Editor.h \
 ScintillaBase.h PropSet.h SString.h Accessor.h KeyWords.h UniConversion.h
ScintillaWinS.o: ScintillaWin.cxx Platform.h Scintilla.h \
 ContractionState.h CellBuffer.h CallTip.h KeyMap.h Indicator.h \
 LineMarker.h Style.h AutoComplete.h ViewStyle.h Document.h Editor.h \
 ScintillaBase.h UniConversion.h
Style.o: Style.cxx Platform.h Style.h
UniConversion.o: UniConversion.cxx UniConversion.h
ViewStyle.o: ViewStyle.cxx Platform.h Scintilla.h Indicator.h \
 LineMarker.h Style.h ViewStyle.h
WindowAccessor.o: DocumentAccessor.cxx Platform.h PropSet.h SString.h Accessor.h WindowAccessor.h Scintilla.h

ScintillaBaseL.o:
	$(CC) $(CXXFLAGS) -D SCI_LEXER -c $< -o $@

ScintillaWinS.o:
	$(CC) $(CXXFLAGS) -D STATIC_BUILD -c $< -o $@

ScintillaWinL.o:
	$(CC) $(CXXFLAGS) -D SCI_LEXER -c $< -o $@

ScintRes.o:	ScintRes.rc PlatformRes.h
	windres ScintRes.rc $@

