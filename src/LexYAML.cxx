// Scintilla source code edit control
/** @file LexYAML.cxx
 ** Lexer for YAML.
 **/
// Copyright 2003- by Sean O'Dell <sean@celsoft.com>
// Release under the same license as Scintilla/SciTE.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "PropSet.h"
#include "Accessor.h"
#include "KeyWords.h"
#include "Scintilla.h"
#include "SciLexer.h"

static const char * const yamlWordListDesc[] = {
	"Keywords",
	0
};

static inline bool AtEOL(Accessor &styler, unsigned int i) {
	return (styler[i] == '\n') ||
		((styler[i] == '\r') && (styler.SafeGetCharAt(i + 1) != '\n'));
}

static void ColouriseYAMLLine(
    char *lineBuffer,
    unsigned int lengthLine,
    unsigned int startLine,
    unsigned int endPos,
    WordList &keywords,
    Accessor &styler) {

	unsigned int i = 0;
	bool bInQuotes = false;
	unsigned int startValue, endValue, valueLen;
	char scalar[256];
	if (lineBuffer[0] == '#') {	// Comment
		styler.ColourTo(endPos, SCE_YAML_COMMENT);
		return;
	}
	if (strncmp(lineBuffer, "---", 3) == 0) {	// Document marker
		styler.ColourTo(endPos, SCE_YAML_DOCUMENT);
		return;
	}
	// Skip initial spaces
	while ((i < lengthLine) && isspacechar(lineBuffer[i])) {
		i++;
	}
	while (i < lengthLine) {
		if (lineBuffer[i] == '\'' || lineBuffer[i] == '\"') {
			bInQuotes = !bInQuotes;
		} else if (lineBuffer[i] == ':' && !bInQuotes) {
			styler.ColourTo(startLine + i, SCE_YAML_IDENTIFIER);
			// Non-folding scalar
			startValue = i + 1;
			while ((startValue < lengthLine) && isspacechar(lineBuffer[startValue])) {
				startValue++;
			}
			endValue = lengthLine - 1;
			while ((endValue >= startValue) && isspacechar(lineBuffer[endValue])) {
				endValue--;
			}
			valueLen = endValue - startValue + 1;
			if (endValue < startValue || valueLen > sizeof(scalar)) {
				break;
			}
			strncpy(scalar,  &lineBuffer[startValue], valueLen);
			scalar[valueLen] = '\0';
			if (scalar[0] == '&' || scalar[0] == '*') {
				styler.ColourTo(startLine + endValue, SCE_YAML_REFERENCE);
			}
			else if (keywords.InList(scalar)) { // Convertible value (true/false, etc.)
				styler.ColourTo(startLine + endValue, SCE_YAML_KEYWORD);
			} else {
				startValue = 0;
				while (startValue < valueLen) {
					if (!isdigit(scalar[startValue]) && scalar[startValue] != '-' && scalar[startValue] != '.' && scalar[startValue] != ',') {
						break;
					}
					startValue++;
				}
				if (startValue >= valueLen) {
					styler.ColourTo(startLine + endValue, SCE_YAML_NUMBER);
				}
			}
			break; // the rest of the line is coloured the default
		}
		i++;
	}
	styler.ColourTo(endPos, SCE_YAML_DEFAULT);
}

static void ColouriseYAMLDoc(unsigned int startPos, int length, int, WordList *keywordLists[], Accessor &styler) {
	char lineBuffer[1024];
	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	unsigned int linePos = 0;
	unsigned int startLine = startPos;
	for (unsigned int i = startPos; i < startPos + length; i++) {
		lineBuffer[linePos++] = styler[i];
		if (AtEOL(styler, i) || (linePos >= sizeof(lineBuffer) - 1)) {
			// End of line (or of line buffer) met, colourise it
			lineBuffer[linePos] = '\0';
			ColouriseYAMLLine(lineBuffer, linePos, startLine, i, *keywordLists[0], styler);
			linePos = 0;
			startLine = i + 1;
		}
	}
	if (linePos > 0) {	// Last line does not have ending characters
		ColouriseYAMLLine(lineBuffer, linePos, startLine, startPos + length - 1, *keywordLists[0], styler);
	}
}

static int IdentifierLevelYAMLLine(
    char *lineBuffer,
    unsigned int lengthLine) {

	unsigned int i = 0;
	bool bInQuotes = false;
	int level;
	if (lineBuffer[0] == '#') {	// Comment
		return 0xFFFFFFFF;
	}
	if (strncmp(lineBuffer, "---", 3) == 0) {	// Document marker
		return 0;
	}
	// Count initial spaces and '-' character
	while ((i < lengthLine) && (isspacechar(lineBuffer[i]) || lineBuffer[i] == '-')) {
		i++;
	}
	level = i;
	while (i < lengthLine) {
		if (lineBuffer[i] == '\'' || lineBuffer[i] == '\"') {
			bInQuotes = !bInQuotes;
		} else if (lineBuffer[i] == ':' && !bInQuotes) {
			return level;
		}
		i++;
	}
	return -level; // Scalar
}

static void FoldYAMLDoc(unsigned int startPos, int length, int, WordList *[], Accessor &styler) {
	char lineBuffer[1024];
	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	unsigned int linePos = 0;
	int currentLevel;
	int lastLevel = 0xFFFFFFFF;
	unsigned int lineCurrent = 0;
	for (unsigned int i = startPos; i < startPos + length; i++) {
		lineBuffer[linePos++] = styler[i];
		if (AtEOL(styler, i) || (linePos >= sizeof(lineBuffer) - 1)) {
			// End of line (or of line buffer) met, colourise it
			lineBuffer[linePos] = '\0';
			currentLevel = IdentifierLevelYAMLLine(lineBuffer, linePos);
	 		if (currentLevel != static_cast<int>(0xFFFFFFFF)) {
				if (abs(currentLevel) > abs(lastLevel) && lastLevel >= 0) { // indented higher than last, and last was an identifier line
					styler.SetLevel(lineCurrent - 1, SC_FOLDLEVELHEADERFLAG);
				}
				lastLevel = currentLevel;
			}
			linePos = 0;
			lineCurrent++;
		}
	}
	if (linePos > 0) {	// Last line does not have ending characters
		currentLevel = IdentifierLevelYAMLLine(lineBuffer, linePos);
		if (currentLevel != static_cast<int>(0xFFFFFFFF)) {
			if (abs(currentLevel) > abs(lastLevel) && lastLevel >= 0) {
				styler.SetLevel(lineCurrent - 1, SC_FOLDLEVELHEADERFLAG);
			}
		}
	}
}

LexerModule lmYAML(SCLEX_YAML, ColouriseYAMLDoc, "yaml", FoldYAMLDoc, yamlWordListDesc);
