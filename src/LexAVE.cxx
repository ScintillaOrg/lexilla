// SciTE - Scintilla based Text Editor
// LexAVE.cxx - lexer for Avenue
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h> 
#include <string.h> 
#include <ctype.h> 
#include <stdio.h> 
#include <stdarg.h> 

#include "Platform.h"

#include "PropSet.h"
#include "Accessor.h"
#include "KeyWords.h"
#include "Scintilla.h"
#include "SciLexer.h"

/*
static bool isOKBeforeRE(char ch) {
	return (ch == '(') || (ch == '=') || (ch == ',');
}
*/
static void ColouriseAveDoc(unsigned int startPos, int length, int initStyle, WordList *keywordlists[], 
	Accessor &styler) {
	
	WordList &keywords = *keywordlists[0];
	
	styler.StartAt(startPos);
	
	bool fold = styler.GetPropertyInt("fold");
//	bool stylingWithinPreprocessor = styler.GetPropertyInt("styling.within.preprocessor");
	int lineCurrent = styler.GetLine(startPos);
	int levelPrev = styler.LevelAt(lineCurrent) & SC_FOLDLEVELNUMBERMASK;
	int levelCurrent = levelPrev;

	int state = initStyle;
	if (state == SCE_C_STRINGEOL)	// Does not leak onto next line
		state = SCE_C_DEFAULT;
	char chPrev = ' ';
	char chNext = styler[startPos];
//	char chPrevNonWhite = ' ';
	unsigned int lengthDoc = startPos + length;
	int visibleChars = 0;
	styler.StartSegment(startPos);

	for (unsigned int i = startPos; i < lengthDoc; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		if ((ch == '\r' && chNext != '\n') || (ch == '\n')) {
			// Trigger on CR only (Mac style) or either on LF from CR+LF (Dos/Win) or on LF alone (Unix)
			// Avoid triggering two times on Dos/Win
			// End of line
			if (state == SCE_C_STRINGEOL) {
				styler.ColourTo(i, state);
				state = SCE_C_DEFAULT;
			}
			if (fold) {
				int lev = levelPrev;
				if (visibleChars == 0)
					lev |= SC_FOLDLEVELWHITEFLAG;
				if ((levelCurrent > levelPrev) && (visibleChars > 0))
					lev |= SC_FOLDLEVELHEADERFLAG;
				styler.SetLevel(lineCurrent, lev);
				lineCurrent++;
				levelPrev = levelCurrent;
			}
			visibleChars = 0;
		}
		if (!isspace(ch))
			visibleChars++;
		if (styler.IsLeadByte(ch)) {
			chNext = styler.SafeGetCharAt(i + 2);
			chPrev = ' ';
			i += 1;
			continue;
		}

		if (state == SCE_C_DEFAULT) {
			if (iswordstart(ch) || (ch == '.')) {
				styler.ColourTo(i-1, state);
				state = SCE_C_IDENTIFIER;
			} else if (ch == '\'') {
				styler.ColourTo(i-1, state);
				state = SCE_C_COMMENTLINE;
			} else if (ch == '\"') {
				styler.ColourTo(i-1, state);
				state = SCE_C_STRING;
			} else if (isoperator(ch) ) {
				styler.ColourTo(i-1, state);
				styler.ColourTo(i, SCE_C_OPERATOR);
			}
		} 
	 	else if (state == SCE_C_IDENTIFIER) {
			if (!iswordchar(ch) || ch == '.' ) {
				char s[100];
				unsigned int start = styler.GetStartSegment();
				unsigned int end = i - 1;
				for (unsigned int ii = 0; ii < end - start + 1 && ii < 30; ii++) 	{
					s[ii] = styler[start + ii];
					s[ii + 1] = '\0';
				}
				char chAttr = SCE_C_IDENTIFIER;
				if (isdigit(s[0]) || (s[0] == '.'))
					chAttr = SCE_C_NUMBER;
				else {
					if (keywords.InList(s)) 
					{
						chAttr = SCE_C_WORD;
						if (strcmp(s, "for") == 0)	levelCurrent +=1;
						if (strcmp(s, "if") == 0)	levelCurrent +=1;
						//if (strcmp(s, "elseif") == 0)	levelCurrent +=1;
						if (strcmp(s, "while") == 0)	levelCurrent +=1;
						if (strcmp(s, "end") == 0)	{
							levelCurrent -=1;
						}
					}
				}
				styler.ColourTo(end, chAttr);
				state = SCE_C_DEFAULT;
				
				if (ch == '\'') {
					state = SCE_C_COMMENTLINE;
				} else if (ch == '\"') {
					state = SCE_C_STRING;
				} else if (isoperator(ch)) {
					styler.ColourTo(i, SCE_C_OPERATOR);
				}
			}
		} 
	  	else if (state == SCE_C_COMMENTLINE) {
			if (ch == '\r' || ch == '\n') {
				styler.ColourTo(i-1, state);
				state = SCE_C_DEFAULT;
			}
		}
		else if (state == SCE_C_STRING) {
			if (ch == '\"') {
				if (chNext == '\"') {
					i++;
					ch = chNext;
					chNext = styler.SafeGetCharAt(i + 1);
				} else
				{
					styler.ColourTo(i, state);
					state = SCE_C_DEFAULT;
				}
			} else if (chNext == '\r' || chNext == '\n') {
				styler.ColourTo(i-1, SCE_C_STRINGEOL);
				state = SCE_C_STRINGEOL;
			}
		} 
	}
	styler.ColourTo(lengthDoc - 1, state);

	// Fill in the real level of the next line, keeping the current flags as they will be filled in later
	if (fold) {
		int flagsNext = styler.LevelAt(lineCurrent) & ~SC_FOLDLEVELNUMBERMASK;
		//styler.SetLevel(lineCurrent, levelCurrent | flagsNext);
		styler.SetLevel(lineCurrent, levelPrev | flagsNext);
		
	}
}

LexerModule lmAVE(SCLEX_AVE, ColouriseAveDoc);
