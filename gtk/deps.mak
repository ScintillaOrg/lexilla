PlatGTK.o: PlatGTK.cxx ../include/Platform.h ../include/Scintilla.h \
  ../include/ScintillaWidget.h ../src/UniConversion.h ../src/XPM.h
ScintillaGTK.o: ScintillaGTK.cxx ../include/Platform.h \
  ../include/Scintilla.h ../include/ScintillaWidget.h \
  ../include/SciLexer.h ../include/PropSet.h ../include/SString.h \
  ../include/Accessor.h ../include/KeyWords.h ../src/ContractionState.h \
  ../src/SVector.h ../src/CellBuffer.h ../src/CallTip.h ../src/KeyMap.h \
  ../src/Indicator.h ../src/XPM.h ../src/LineMarker.h ../src/Style.h \
  ../src/AutoComplete.h ../src/ViewStyle.h ../src/Document.h \
  ../src/Editor.h ../src/ScintillaBase.h
AutoComplete.o: ../src/AutoComplete.cxx ../include/Platform.h \
  ../include/PropSet.h ../include/SString.h ../src/AutoComplete.h
CallTip.o: ../src/CallTip.cxx ../include/Platform.h \
  ../include/Scintilla.h ../src/CallTip.h
CellBuffer.o: ../src/CellBuffer.cxx ../include/Platform.h \
  ../include/Scintilla.h ../src/SVector.h ../src/CellBuffer.h
ContractionState.o: ../src/ContractionState.cxx ../include/Platform.h \
  ../src/ContractionState.h
DocumentAccessor.o: ../src/DocumentAccessor.cxx ../include/Platform.h \
  ../include/PropSet.h ../include/SString.h ../src/SVector.h \
  ../include/Accessor.h ../src/DocumentAccessor.h ../src/CellBuffer.h \
  ../include/Scintilla.h ../src/Document.h
Document.o: ../src/Document.cxx ../include/Platform.h \
  ../include/Scintilla.h ../src/SVector.h ../src/CellBuffer.h \
  ../src/Document.h ../src/RESearch.h
Editor.o: ../src/Editor.cxx ../include/Platform.h ../include/Scintilla.h \
  ../src/ContractionState.h ../src/SVector.h ../src/CellBuffer.h \
  ../src/KeyMap.h ../src/Indicator.h ../src/XPM.h ../src/LineMarker.h \
  ../src/Style.h ../src/ViewStyle.h ../src/Document.h ../src/Editor.h
ExternalLexer.o: ../src/ExternalLexer.cxx ../include/Platform.h \
 ../include/Scintilla.h ../include/SciLexer.h ../include/PropSet.h \
 ../include/Accessor.h ../src/DocumentAccessor.h ../include/KeyWords.h \
 ../src/ExternalLexer.h
Indicator.o: ../src/Indicator.cxx ../include/Platform.h \
  ../include/Scintilla.h ../src/Indicator.h
KeyMap.o: ../src/KeyMap.cxx ../include/Platform.h ../include/Scintilla.h \
  ../src/KeyMap.h
KeyWords.o: ../src/KeyWords.cxx ../include/Platform.h \
  ../include/PropSet.h ../include/SString.h ../include/Accessor.h \
  ../include/KeyWords.h ../include/Scintilla.h ../include/SciLexer.h
LexAda.o: ../src/LexAda.cxx ../include/Platform.h ../include/Accessor.h \
  ../src/StyleContext.h ../include/PropSet.h ../include/SString.h \
  ../include/KeyWords.h ../include/SciLexer.h
LexAsm.o: ../src/LexAsm.cxx ../include/Platform.h ../include/PropSet.h \
  ../include/SString.h ../include/Accessor.h ../src/StyleContext.h \
  ../include/KeyWords.h ../include/Scintilla.h ../include/SciLexer.h
LexAVE.o: ../src/LexAVE.cxx ../include/Platform.h ../include/PropSet.h \
  ../include/SString.h ../include/Accessor.h ../src/StyleContext.h \
  ../include/KeyWords.h ../include/Scintilla.h ../include/SciLexer.h
LexBaan.o: ../src/LexBaan.cxx ../include/Platform.h ../include/PropSet.h \
  ../include/SString.h ../include/Accessor.h ../src/StyleContext.h \
  ../include/KeyWords.h ../include/Scintilla.h ../include/SciLexer.h
LexBullant.o: ../src/LexBullant.cxx ../include/Platform.h \
  ../include/PropSet.h ../include/SString.h ../include/Accessor.h \
  ../include/KeyWords.h ../include/Scintilla.h ../include/SciLexer.h
LexConf.o: ../src/LexConf.cxx ../include/Platform.h ../include/PropSet.h \
  ../include/SString.h ../include/Accessor.h ../include/KeyWords.h \
  ../include/Scintilla.h ../include/SciLexer.h
LexCPP.o: ../src/LexCPP.cxx ../include/Platform.h ../include/PropSet.h \
  ../include/SString.h ../include/Accessor.h ../src/StyleContext.h \
  ../include/KeyWords.h ../include/Scintilla.h ../include/SciLexer.h
LexCrontab.o: ../src/LexCrontab.cxx ../include/Platform.h \
  ../include/PropSet.h ../include/SString.h ../include/Accessor.h \
  ../include/KeyWords.h ../include/Scintilla.h ../include/SciLexer.h
LexCSS.o: ../src/LexCSS.cxx ../include/Platform.h ../include/PropSet.h \
  ../include/SString.h ../include/Accessor.h ../src/StyleContext.h \
  ../include/KeyWords.h ../include/Scintilla.h ../include/SciLexer.h
LexEiffel.o: ../src/LexEiffel.cxx ../include/Platform.h \
  ../include/PropSet.h ../include/SString.h ../include/Accessor.h \
  ../src/StyleContext.h ../include/KeyWords.h ../include/Scintilla.h \
  ../include/SciLexer.h
LexFortran.o: ../src/LexFortran.cxx ../include/Platform.h \
  ../include/PropSet.h ../include/SString.h ../include/Accessor.h \
  ../src/StyleContext.h ../include/KeyWords.h ../include/Scintilla.h \
  ../include/SciLexer.h
LexHTML.o: ../src/LexHTML.cxx ../include/Platform.h ../include/PropSet.h \
  ../include/SString.h ../include/Accessor.h ../src/StyleContext.h \
  ../include/KeyWords.h ../include/Scintilla.h ../include/SciLexer.h
LexLisp.o: ../src/LexLisp.cxx ../include/Platform.h ../include/PropSet.h \
  ../include/SString.h ../include/Accessor.h ../include/KeyWords.h \
  ../include/Scintilla.h ../include/SciLexer.h
LexLua.o: ../src/LexLua.cxx ../include/Platform.h ../include/PropSet.h \
  ../include/SString.h ../include/Accessor.h ../src/StyleContext.h \
  ../include/KeyWords.h ../include/Scintilla.h ../include/SciLexer.h
LexMatlab.o: ../src/LexMatlab.cxx ../include/Platform.h \
  ../include/PropSet.h ../include/SString.h ../include/Accessor.h \
  ../src/StyleContext.h ../include/KeyWords.h ../include/Scintilla.h \
  ../include/SciLexer.h
LexOthers.o: ../src/LexOthers.cxx ../include/Platform.h \
  ../include/PropSet.h ../include/SString.h ../include/Accessor.h \
  ../include/KeyWords.h ../include/Scintilla.h ../include/SciLexer.h
LexPascal.o: ../src/LexPascal.cxx ../include/Platform.h \
  ../include/PropSet.h ../include/SString.h ../include/Accessor.h \
  ../include/KeyWords.h ../include/Scintilla.h ../include/SciLexer.h \
  ../src/StyleContext.h
LexPerl.o: ../src/LexPerl.cxx ../include/Platform.h ../include/PropSet.h \
  ../include/SString.h ../include/Accessor.h ../include/KeyWords.h \
  ../include/Scintilla.h ../include/SciLexer.h
LexPython.o: ../src/LexPython.cxx ../include/Platform.h \
  ../include/PropSet.h ../include/SString.h ../include/Accessor.h \
  ../src/StyleContext.h ../include/KeyWords.h ../include/Scintilla.h \
  ../include/SciLexer.h
LexRuby.o: ../src/LexRuby.cxx ../include/Platform.h ../include/PropSet.h \
  ../include/SString.h ../include/Accessor.h ../include/KeyWords.h \
  ../include/Scintilla.h ../include/SciLexer.h
LexSQL.o: ../src/LexSQL.cxx ../include/Platform.h ../include/PropSet.h \
  ../include/SString.h ../include/Accessor.h ../include/KeyWords.h \
  ../include/Scintilla.h ../include/SciLexer.h
LexVB.o: ../src/LexVB.cxx ../include/Platform.h ../include/PropSet.h \
  ../include/SString.h ../include/Accessor.h ../src/StyleContext.h \
  ../include/KeyWords.h ../include/Scintilla.h ../include/SciLexer.h
LineMarker.o: ../src/LineMarker.cxx ../include/Platform.h \
  ../include/Scintilla.h ../src/XPM.h ../src/LineMarker.h
PropSet.o: ../src/PropSet.cxx ../include/Platform.h ../include/PropSet.h \
  ../include/SString.h
RESearch.o: ../src/RESearch.cxx ../src/RESearch.h
ScintillaBase.o: ../src/ScintillaBase.cxx ../include/Platform.h \
  ../include/Scintilla.h ../include/PropSet.h ../include/SString.h \
  ../include/SciLexer.h ../include/Accessor.h ../src/DocumentAccessor.h \
  ../include/KeyWords.h ../src/ContractionState.h ../src/SVector.h \
  ../src/CellBuffer.h ../src/CallTip.h ../src/KeyMap.h ../src/Indicator.h \
  ../src/XPM.h ../src/LineMarker.h ../src/Style.h ../src/ViewStyle.h \
  ../src/AutoComplete.h ../src/Document.h ../src/Editor.h \
  ../src/ScintillaBase.h
StyleContext.o: ../src/StyleContext.cxx ../include/Platform.h \
  ../include/PropSet.h ../include/SString.h ../include/Accessor.h \
  ../src/StyleContext.h
Style.o: ../src/Style.cxx ../include/Platform.h ../include/Scintilla.h \
  ../src/Style.h
UniConversion.o: ../src/UniConversion.cxx ../src/UniConversion.h
ViewStyle.o: ../src/ViewStyle.cxx ../include/Platform.h \
  ../include/Scintilla.h ../src/Indicator.h ../src/XPM.h \
  ../src/LineMarker.h ../src/Style.h ../src/ViewStyle.h
WindowAccessor.o: ../src/WindowAccessor.cxx ../include/Platform.h \
  ../include/PropSet.h ../include/SString.h ../include/Accessor.h \
  ../include/WindowAccessor.h ../include/Scintilla.h
XPM.o: ../src/XPM.cxx ../include/Platform.h ../src/XPM.h
