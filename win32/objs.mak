# Make include file for Scintilla on Windows
# Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
# The License.txt file describes the conditions under which this software may be distributed.
# This file is included by both the Borland and Microsoft makefiles to define how to build
# all the objects.
# The variables DIR_O, CC, INCLUDEDIRS, and CXXFLAGS must be defined before inclusion.

$(DIR_O)\AutoComplete.obj: ..\src\AutoComplete.cxx ..\include\Platform.h ..\src\AutoComplete.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx

$(DIR_O)\CallTip.obj: ..\src\CallTip.cxx ..\include\Platform.h ..\src\CallTip.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx

$(DIR_O)\CellBuffer.obj: ..\src\CellBuffer.cxx ..\include\Platform.h ..\include\Scintilla.h ..\src\CellBuffer.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx

$(DIR_O)\ContractionState.obj: ..\src\ContractionState.cxx ..\include\Platform.h ..\src\ContractionState.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx

$(DIR_O)\Document.obj: ..\src\Document.cxx ..\include\Platform.h ..\include\Scintilla.h ..\src\CellBuffer.h \
 ..\src\Document.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx
 
$(DIR_O)\DocumentAccessor.obj: ..\src\DocumentAccessor.cxx ..\include\Platform.h ..\include\PropSet.h ..\include\Accessor.h ..\src\DocumentAccessor.h ..\include\Scintilla.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx

$(DIR_O)\Editor.obj: ..\src\Editor.cxx ..\include\Platform.h ..\include\Scintilla.h ..\src\ContractionState.h \
 ..\src\CellBuffer.h ..\src\KeyMap.h ..\src\Indicator.h ..\src\LineMarker.h ..\src\Style.h ..\src\ViewStyle.h \
 ..\src\Document.h ..\src\Editor.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx
 
$(DIR_O)\Indicator.obj: ..\src\Indicator.cxx ..\include\Platform.h ..\include\Scintilla.h ..\src\Indicator.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx

$(DIR_O)\KeyMap.obj: ..\src\KeyMap.cxx ..\include\Platform.h ..\include\Scintilla.h ..\src\KeyMap.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx

$(DIR_O)\KeyWords.obj: ..\src\KeyWords.cxx ..\include\Platform.h ..\include\PropSet.h ..\include\Accessor.h ..\include\KeyWords.h \
 ..\include\Scintilla.h ..\include\SciLexer.h 
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx
 
$(DIR_O)\LexCPP.obj: ..\src\LexCPP.cxx ..\include\Platform.h ..\include\PropSet.h ..\include\Accessor.h ..\include\KeyWords.h \
 ..\include\Scintilla.h ..\include\SciLexer.h 
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx
 
$(DIR_O)\LexHTML.obj: ..\src\LexHTML.cxx ..\include\Platform.h ..\include\PropSet.h ..\include\Accessor.h ..\include\KeyWords.h \
 ..\include\Scintilla.h ..\include\SciLexer.h 
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx
 
$(DIR_O)\LexLua.obj: ..\src\LexLua.cxx ..\include\Platform.h ..\include\PropSet.h ..\include\Accessor.h ..\include\KeyWords.h \
 ..\include\Scintilla.h ..\include\SciLexer.h 
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx
 
$(DIR_O)\LexOthers.obj: ..\src\LexOthers.cxx ..\include\Platform.h ..\include\PropSet.h ..\include\Accessor.h ..\include\KeyWords.h \
 ..\include\Scintilla.h ..\include\SciLexer.h 
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx
 
$(DIR_O)\LexPerl.obj: ..\src\LexPerl.cxx ..\include\Platform.h ..\include\PropSet.h ..\include\Accessor.h ..\include\KeyWords.h \
 ..\include\Scintilla.h ..\include\SciLexer.h 
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx
 
$(DIR_O)\LexPython.obj: ..\src\LexPython.cxx ..\include\Platform.h ..\include\PropSet.h ..\include\Accessor.h ..\include\KeyWords.h \
 ..\include\Scintilla.h ..\include\SciLexer.h 
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx
 
$(DIR_O)\LexSQL.obj: ..\src\LexSQL.cxx ..\include\Platform.h ..\include\PropSet.h ..\include\Accessor.h ..\include\KeyWords.h \
 ..\include\Scintilla.h ..\include\SciLexer.h 
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx
 
$(DIR_O)\LexVB.obj: ..\src\LexVB.cxx ..\include\Platform.h ..\include\PropSet.h ..\include\Accessor.h ..\include\KeyWords.h \
 ..\include\Scintilla.h ..\include\SciLexer.h 
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx
 
$(DIR_O)\LineMarker.obj: ..\src\LineMarker.cxx ..\include\Platform.h ..\include\Scintilla.h ..\src\LineMarker.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx

$(DIR_O)\PlatWin.obj: PlatWin.cxx ..\include\Platform.h PlatformRes.h ..\src\UniConversion.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ $(@B).cxx

$(DIR_O)\PropSet.obj: ..\src\PropSet.cxx ..\include\Platform.h ..\include\PropSet.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx

$(DIR_O)\ScintillaBase.obj: ..\src\ScintillaBase.cxx ..\include\Platform.h ..\include\Scintilla.h \
 ..\src\ContractionState.h ..\src\CellBuffer.h ..\src\CallTip.h ..\src\KeyMap.h ..\src\Indicator.h \
 ..\src\LineMarker.h ..\src\Style.h ..\src\ViewStyle.h ..\src\AutoComplete.h ..\src\Document.h ..\src\Editor.h \
 ..\src\ScintillaBase.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\ScintillaBase.cxx
	
$(DIR_O)\ScintillaBaseL.obj: ..\src\ScintillaBase.cxx ..\include\Platform.h ..\include\Scintilla.h ..\include\SciLexer.h \
 ..\src\ContractionState.h ..\src\CellBuffer.h ..\src\CallTip.h ..\src\KeyMap.h ..\src\Indicator.h \
 ..\src\LineMarker.h ..\src\Style.h ..\src\AutoComplete.h ..\src\ViewStyle.h ..\src\Document.h ..\src\Editor.h \
 ..\src\ScintillaBase.h ..\include\PropSet.h ..\include\Accessor.h ..\src\DocumentAccessor.h ..\include\KeyWords.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -DSCI_LEXER -c $(NAMEFLAG)$@ ..\src\ScintillaBase.cxx

$(DIR_O)\ScintillaWin.obj: ScintillaWin.cxx ..\include\Platform.h ..\include\Scintilla.h \
 ..\src\ContractionState.h ..\src\CellBuffer.h ..\src\CallTip.h ..\src\KeyMap.h ..\src\Indicator.h \
 ..\src\LineMarker.h ..\src\Style.h ..\src\AutoComplete.h ..\src\ViewStyle.h ..\src\Document.h ..\src\Editor.h \
 ..\src\ScintillaBase.h ..\src\UniConversion.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ $(@B).cxx

$(DIR_O)\ScintillaWinL.obj: ScintillaWin.cxx ..\include\Platform.h ..\include\Scintilla.h ..\include\SciLexer.h \
 ..\src\ContractionState.h ..\src\CellBuffer.h ..\src\CallTip.h ..\src\KeyMap.h ..\src\Indicator.h \
 ..\src\LineMarker.h ..\src\Style.h ..\src\AutoComplete.h ..\src\ViewStyle.h ..\src\Document.h ..\src\Editor.h \
 ..\src\ScintillaBase.h ..\include\PropSet.h ..\include\Accessor.h ..\include\KeyWords.h ..\src\UniConversion.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -DSCI_LEXER -c $(NAMEFLAG)$@ ScintillaWin.cxx

$(DIR_O)\ScintillaWinS.obj: ScintillaWin.cxx ..\include\Platform.h ..\include\Scintilla.h \
 ..\src\ContractionState.h ..\src\CellBuffer.h ..\src\CallTip.h ..\src\KeyMap.h ..\src\Indicator.h \
 ..\src\LineMarker.h ..\src\Style.h ..\src\AutoComplete.h ..\src\ViewStyle.h ..\src\Document.h ..\src\Editor.h \
 ..\src\ScintillaBase.h ..\src\UniConversion.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -DSTATIC_BUILD -c $(NAMEFLAG)$@ ScintillaWin.cxx

$(DIR_O)\Style.obj: ..\src\Style.cxx ..\include\Platform.h ..\src\Style.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx

$(DIR_O)\ViewStyle.obj: ..\src\ViewStyle.cxx ..\include\Platform.h ..\include\Scintilla.h ..\src\Indicator.h \
 ..\src\LineMarker.h ..\src\Style.h ..\src\ViewStyle.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx

$(DIR_O)\UniConversion.obj: ..\src\UniConversion.cxx ..\src\UniConversion.h
 	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx

$(DIR_O)\WindowAccessor.obj: ..\src\WindowAccessor.cxx ..\include\Platform.h ..\include\PropSet.h ..\include\Accessor.h ..\include\WindowAccessor.h ..\include\Scintilla.h
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ ..\src\$(@B).cxx
