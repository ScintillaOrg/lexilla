# Make file for Scintilla on Linux or compatible OS
# Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
# The License.txt file describes the conditions under which this software may be distributed.
# This makefile assumes GCC 2.95.2 is used and changes will be needed to use other compilers.
# GNU make does not like \r\n line endings so should be saved to CVS in binary form.

.SUFFIXES: .cxx .o .h .a
CC = g++
AR = ar

COMPLIB=../bin/scintilla.a

vpath %.h ../src ../include
vpath %.cxx ../src

#CXXFLAGS= -g -DGTK -DSCI_LEXER -Wwrite-strings
INCLUDEDIRS=-I ../include -I ../src
CXXFLAGS= -DGTK -DSCI_LEXER

.cxx.o:
	$(CC) `gtk-config --cflags` $(INCLUDEDIRS) $(CXXFLAGS) -c $< -o $@

LEXOBJS	=  LexCPP.o LexHTML.o LexOthers.o LexPerl.o LexPython.o LexSQL.o LexVB.o

# The LEXOBJS have to be treated specially as the functions in them are not called from external code

all: $(COMPLIB) $(LEXOBJS)

$(COMPLIB): Accessor.o KeyWords.o Document.o CallTip.o \
	ScintillaBase.o ContractionState.o Editor.o PropSet.o PlatGTK.o \
	KeyMap.o LineMarker.o ScintillaGTK.o CellBuffer.o ViewStyle.o \
	Style.o Indicator.o AutoComplete.o
	$(AR) rc $@ $^

Accessor.o: Accessor.cxx Platform.h PropSet.h Accessor.h Scintilla.h \
 WinDefs.h
AutoComplete.o: AutoComplete.cxx Platform.h AutoComplete.h
CallTip.o: CallTip.cxx Platform.h CallTip.h
CellBuffer.o: CellBuffer.cxx Platform.h Scintilla.h WinDefs.h \
 CellBuffer.h
ContractionState.o: ContractionState.cxx Platform.h ContractionState.h
Document.o: Document.cxx Platform.h Scintilla.h WinDefs.h CellBuffer.h \
 Document.h
Editor.o: Editor.cxx Platform.h Scintilla.h WinDefs.h \
 ContractionState.h CellBuffer.h KeyMap.h Indicator.h LineMarker.h \
 Style.h ViewStyle.h Document.h Editor.h
Indicator.o: Indicator.cxx Platform.h Scintilla.h WinDefs.h \
 Indicator.h
KeyMap.o: KeyMap.cxx Platform.h Scintilla.h WinDefs.h KeyMap.h
KeyWords.o: KeyWords.cxx Platform.h PropSet.h Accessor.h KeyWords.h \
 Scintilla.h WinDefs.h SciLexer.h
LexCPP.o: LexCPP.cxx Platform.h PropSet.h Accessor.h KeyWords.h \
 Scintilla.h WinDefs.h SciLexer.h
LexHTML.o: LexHTML.cxx Platform.h PropSet.h Accessor.h KeyWords.h \
 Scintilla.h WinDefs.h SciLexer.h
LexOthers.o: LexOthers.cxx Platform.h PropSet.h Accessor.h KeyWords.h \
 Scintilla.h WinDefs.h SciLexer.h
LexPerl.o: LexPerl.cxx Platform.h PropSet.h Accessor.h KeyWords.h \
 Scintilla.h WinDefs.h SciLexer.h
LexPython.o: LexPython.cxx Platform.h PropSet.h Accessor.h KeyWords.h \
 Scintilla.h WinDefs.h SciLexer.h
LexSQL.o: LexSQL.cxx Platform.h PropSet.h Accessor.h KeyWords.h \
 Scintilla.h WinDefs.h SciLexer.h
LexVB.o: LexVB.cxx Platform.h PropSet.h Accessor.h KeyWords.h \
 Scintilla.h WinDefs.h SciLexer.h
LineMarker.o: LineMarker.cxx Platform.h Scintilla.h WinDefs.h \
 LineMarker.h
PlatGTK.o: PlatGTK.cxx Platform.h Scintilla.h WinDefs.h
PropSet.o: PropSet.cxx Platform.h PropSet.h
SciTEBase.o: SciTEBase.cxx Platform.h WinDefs.h SciTE.h PropSet.h \
 Accessor.h KeyWords.h Scintilla.h SciLexer.h SciTEBase.h
SciTEGTK.o: SciTEGTK.cxx Platform.h WinDefs.h SciTE.h PropSet.h \
 Accessor.h KeyWords.h Scintilla.h SciTEBase.h
ScintillaBase.o: ScintillaBase.cxx Platform.h Scintilla.h WinDefs.h \
 SciLexer.h PropSet.h Accessor.h KeyWords.h ContractionState.h \
 CellBuffer.h CallTip.h KeyMap.h Indicator.h LineMarker.h Style.h \
 ViewStyle.h AutoComplete.h Document.h Editor.h ScintillaBase.h
ScintillaGTK.o: ScintillaGTK.cxx Platform.h Scintilla.h WinDefs.h \
 SciLexer.h PropSet.h Accessor.h KeyWords.h ContractionState.h \
 CellBuffer.h CallTip.h KeyMap.h Indicator.h LineMarker.h Style.h \
 AutoComplete.h ViewStyle.h Document.h Editor.h ScintillaBase.h
Style.o: Style.cxx Platform.h Style.h
ViewStyle.o: ViewStyle.cxx Platform.h Scintilla.h WinDefs.h \
 Indicator.h LineMarker.h Style.h ViewStyle.h

clean: 
	rm -f *.o SciTE
