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
CXXBASEFLAGS= -DGTK -DSCI_LEXER -W -Wall

ifdef DEBUG
CXXFLAGS=-DDEBUG $(CXXBASEFLAGS)
else
CXXFLAGS=-DNDEBUG $(CXXBASEFLAGS)
endif

.cxx.o:
	$(CC) `gtk-config --cflags` $(INCLUDEDIRS) $(CXXFLAGS) -c $< -o $@

LEXOBJS	= LexAda.o LexAVE.o LexConf.o LexCPP.o LexHTML.o LexLua.o \
	LexOthers.o LexPascal.o LexPerl.o LexPython.o LexSQL.o LexVB.o

# The LEXOBJS have to be treated specially as the functions in them are not called from external code

all: $(COMPLIB) $(LEXOBJS)

$(COMPLIB): DocumentAccessor.o WindowAccessor.o KeyWords.o Document.o CallTip.o \
	ScintillaBase.o ContractionState.o Editor.o PropSet.o PlatGTK.o \
	KeyMap.o LineMarker.o ScintillaGTK.o CellBuffer.o ViewStyle.o \
	RESearch.o Style.o Indicator.o AutoComplete.o
	$(AR) rc $@ $^

# All lexers depend on this set of headers
LEX_HEADERS=Platform.h PropSet.h SString.h Accessor.h \
 KeyWords.h Scintilla.h WinDefs.h SciLexer.h

AutoComplete.o: AutoComplete.cxx Platform.h AutoComplete.h
CallTip.o: CallTip.cxx Platform.h CallTip.h
CellBuffer.o: CellBuffer.cxx Platform.h Scintilla.h WinDefs.h \
 CellBuffer.h
ContractionState.o: ContractionState.cxx Platform.h ContractionState.h
Document.o: Document.cxx Platform.h Scintilla.h WinDefs.h RESearch.h \
 CellBuffer.h Document.h
DocumentAccessor.o: DocumentAccessor.cxx Platform.h PropSet.h SString.h \
 Accessor.h DocumentAccessor.h Scintilla.h WinDefs.h
Editor.o: Editor.cxx Platform.h Scintilla.h WinDefs.h \
 ContractionState.h CellBuffer.h KeyMap.h Indicator.h LineMarker.h \
 Style.h ViewStyle.h Document.h Editor.h
Indicator.o: Indicator.cxx Platform.h Scintilla.h WinDefs.h \
 Indicator.h
KeyMap.o: KeyMap.cxx Platform.h Scintilla.h WinDefs.h KeyMap.h
KeyWords.o: KeyWords.cxx Platform.h PropSet.h SString.h Accessor.h \
 KeyWords.h Scintilla.h WinDefs.h SciLexer.h
LexAda.o: LexAda.cxx $(LEX_HEADERS)
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
LineMarker.o: LineMarker.cxx Platform.h Scintilla.h WinDefs.h \
 LineMarker.h
PlatGTK.o: PlatGTK.cxx Platform.h Scintilla.h ScintillaWidget.h WinDefs.h
PropSet.o: PropSet.cxx Platform.h PropSet.h SString.h
RESearch.o: RESearch.cxx RESearch.h
SciTEBase.o: SciTEBase.cxx Platform.h WinDefs.h SciTE.h PropSet.h \
 SString.h Accessor.h KeyWords.h Scintilla.h SciLexer.h SciTEBase.h
SciTEGTK.o: SciTEGTK.cxx Platform.h WinDefs.h SciTE.h PropSet.h \
 SString.h Accessor.h KeyWords.h Scintilla.h SciTEBase.h
ScintillaBase.o: ScintillaBase.cxx Platform.h Scintilla.h WinDefs.h \
 SciLexer.h PropSet.h SString.h Accessor.h KeyWords.h ContractionState.h \
 CellBuffer.h CallTip.h KeyMap.h Indicator.h LineMarker.h Style.h \
 ViewStyle.h AutoComplete.h Document.h Editor.h ScintillaBase.h
ScintillaGTK.o: ScintillaGTK.cxx Platform.h Scintilla.h ScintillaWidget.h WinDefs.h \
 SciLexer.h PropSet.h SString.h Accessor.h KeyWords.h ContractionState.h \
 CellBuffer.h CallTip.h KeyMap.h Indicator.h LineMarker.h Style.h \
 AutoComplete.h ViewStyle.h Document.h Editor.h ScintillaBase.h
Style.o: Style.cxx Platform.h Style.h
ViewStyle.o: ViewStyle.cxx Platform.h Scintilla.h WinDefs.h \
 Indicator.h LineMarker.h Style.h ViewStyle.h
WindowAccessor.o: WindowAccessor.cxx Platform.h PropSet.h SString.h \
 Accessor.h WindowAccessor.h Scintilla.h WinDefs.h

clean:
	rm -f *.o SciTE
