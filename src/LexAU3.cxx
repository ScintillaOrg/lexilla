// Scintilla source code edit control
// @file LexAU3.cxx
// Lexer for AutoIt3  http://www.hiddensoft.com/autoit3
// by Jos van der Zande, jvdzande@yahoo.com 
//
// Changes:
//
// Copyright for Scintilla: 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.
// Scintilla source code edit control

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

#include "Platform.h"

#include "PropSet.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "KeyWords.h"
#include "Scintilla.h"
#include "SciLexer.h"

static bool IsAU3Comment(Accessor &styler, int pos, int len) {
	return len>0 && styler[pos]==';';
}

static inline bool IsTypeCharacter(const int ch)
{
    return ch == '$';
}
static inline bool IsAWordChar(const int ch)
{
    return (ch < 0x80) && (isalnum(ch) || ch == '.' || ch == '_' || ch == '-');
}

static inline bool IsAWordStart(const int ch)
{
    return (ch < 0x80) && (isalnum(ch) || ch == '_' || ch == '@' || ch == '#' || ch == '{' || ch == '+' || ch == '!' || ch == '#' || ch == '^');
}

static void ColouriseAU3Doc(unsigned int startPos, 
							int length, int initStyle,
							WordList *keywordlists[],
							Accessor &styler) {

    WordList &keywords = *keywordlists[0];
    WordList &keywords2 = *keywordlists[1];
    WordList &keywords3 = *keywordlists[2];
    WordList &keywords4 = *keywordlists[3];
    styler.StartAt(startPos);

    StyleContext sc(startPos, length, initStyle, styler);
	char si,sk;
	si=0;
	sk=0;
    for (; sc.More(); sc.Forward()) {
		char s[100];
		sc.GetCurrentLowered(s, sizeof(s));
		switch (sc.state)
        {
            case SCE_AU3_COMMENTBLOCK:
            {
                if (sc.ch == '#') {sc.SetState(SCE_AU3_DEFAULT);}
				break;
			}
            case SCE_AU3_COMMENT:
            {
                if (sc.atLineEnd) {sc.SetState(SCE_AU3_DEFAULT);}
                break;
            }
            case SCE_AU3_OPERATOR:
            {
                sc.SetState(SCE_AU3_DEFAULT);
                break;
            }
            case SCE_AU3_KEYWORD:
            {
                if (!IsAWordChar(sc.ch))
                {
                    if (!IsTypeCharacter(sc.ch))
                    {
						if (strcmp(s, "#cs")==0 || strcmp(s, "#comments_start")==0)
						{
							sc.ChangeState(SCE_AU3_COMMENTBLOCK);
							sc.SetState(SCE_AU3_COMMENTBLOCK);
						}
						else if (strcmp(s, "#ce")==0 || strcmp(s, "#comments_end")==0) 
						{
							sc.ChangeState(SCE_AU3_COMMENTBLOCK);
							sc.SetState(SCE_AU3_DEFAULT);
						}
						else if (keywords.InList(s)) {
							sc.ChangeState(SCE_AU3_KEYWORD);
							sc.SetState(SCE_AU3_DEFAULT);
							//sc.SetState(SCE_AU3_KEYWORD);
						}
						else if (keywords2.InList(s)) {
							sc.ChangeState(SCE_AU3_FUNCTION);
							sc.SetState(SCE_AU3_DEFAULT);
							//sc.SetState(SCE_AU3_FUNCTION);
						}
						else if (keywords3.InList(s)) {
							sc.ChangeState(SCE_AU3_MACRO);
							sc.SetState(SCE_AU3_DEFAULT);
						}
						else if (!IsAWordChar(sc.ch)) {
							sc.ChangeState(SCE_AU3_DEFAULT);
							sc.SetState(SCE_AU3_DEFAULT);
						}
					}
				}	
                if (sc.atLineEnd) {sc.SetState(SCE_AU3_DEFAULT);}
                break;
            }
            case SCE_AU3_NUMBER:
            {
                if (!IsAWordChar(sc.ch)) {sc.SetState(SCE_AU3_DEFAULT);}
                break;
            }
            case SCE_AU3_VARIABLE:
            {
                if (!IsAWordChar(sc.ch)) {sc.SetState(SCE_AU3_DEFAULT);}
                break;
            }
            case SCE_AU3_STRING:
            {
				sk = 0;
				// check for " in and single qouted string
	            if (si == 1){
					if (sc.ch == '\"'){sc.ForwardSetState(SCE_AU3_DEFAULT);}}
				// check for ' in and double qouted string
                if (si == 2){
					if (sc.ch == '\''){sc.ForwardSetState(SCE_AU3_DEFAULT);}}
                if (sc.atLineEnd) {sc.SetState(SCE_AU3_DEFAULT);}
				// find Sendkeys in an STRING
				if (sc.ch == '{') {sc.SetState(SCE_AU3_SENT);}
				if (sc.ch == '+' && sc.chNext == '{') {sc.SetState(SCE_AU3_SENT);}
				if (sc.ch == '!' && sc.chNext == '{') {sc.SetState(SCE_AU3_SENT);}
				if (sc.ch == '^' && sc.chNext == '{') {sc.SetState(SCE_AU3_SENT);}
				if (sc.ch == '#' && sc.chNext == '{') {sc.SetState(SCE_AU3_SENT);}
				break;
            }
            
            case SCE_AU3_SENT:
            {
				// Sent key string ended 
				if (sk == 1) 
				{
					// set color to SENTKEY when valid sentkey .. else set to comment to show its wrong
					if (keywords4.InList(s)) 
					{
						sc.ChangeState(SCE_AU3_SENT);
					}
					else
					{
						sc.ChangeState(SCE_AU3_STRING);
					}
					sc.SetState(SCE_AU3_STRING);
					sk=0;
				}
				// check if next portion is again a sentkey
				if (sc.atLineEnd) {sc.SetState(SCE_AU3_DEFAULT);}
				if (sc.ch == '{') {sc.SetState(SCE_AU3_SENT);}
				if (sc.ch == '+' && sc.chNext == '{') {sc.SetState(SCE_AU3_SENT);}
				if (sc.ch == '!' && sc.chNext == '{') {sc.SetState(SCE_AU3_SENT);}
				if (sc.ch == '^' && sc.chNext == '{') {sc.SetState(SCE_AU3_SENT);}
				if (sc.ch == '#' && sc.chNext == '{') {sc.SetState(SCE_AU3_SENT);}
				// check to see if the string ended...
				// check for " in and single qouted string
	            if (si == 1){
					if (sc.ch == '\"'){sc.ForwardSetState(SCE_AU3_DEFAULT);}}
				// check for ' in and double qouted string
                if (si == 2){
					if (sc.ch == '\''){sc.ForwardSetState(SCE_AU3_DEFAULT);}}
				break;
            }
        }  //switch (sc.state)

        // Determine if a new state should be entered:
        if (sc.state == SCE_AU3_SENT)
        {
			if (sc.ch == '}' && sc.chNext != '}') 
			{
				sk = 1;
			}			
		}
		if (sc.state == SCE_AU3_DEFAULT)
        {
            if (sc.ch == ';') {sc.SetState(SCE_AU3_COMMENT);}
            else if (sc.ch == '#') {sc.SetState(SCE_AU3_KEYWORD);}
            else if (IsAWordStart(sc.ch)) {sc.SetState(SCE_AU3_KEYWORD);}
            else if (sc.ch == '@') {sc.SetState(SCE_AU3_KEYWORD);}
            else if (sc.ch == '\"') {
				sc.SetState(SCE_AU3_STRING);
				si = 1;	}
            else if (sc.ch == '\'') {
				sc.SetState(SCE_AU3_STRING);
				si = 2;	}
            else if (sc.ch == '$') {sc.SetState(SCE_AU3_VARIABLE);}
            else if (IsADigit(sc.ch) || (sc.ch == '.' && IsADigit(sc.chNext))) {sc.SetState(SCE_AU3_NUMBER);}
            //else if (IsAWordStart(sc.ch)) {sc.SetState(SCE_AU3_KEYWORD);}
            else if (isoperator(static_cast<char>(sc.ch)) || (sc.ch == '\\')) {sc.SetState(SCE_AU3_OPERATOR);}
			else if (sc.atLineEnd) {sc.SetState(SCE_AU3_DEFAULT);}
        }
    }      //for (; sc.More(); sc.Forward())
    sc.Complete();
}

//
//
static void FoldAU3Doc(unsigned int startPos, int length, int, WordList *[], Accessor &styler)
{
		int endPos = startPos + length;

	// Backtrack to previous line in case need to fix its fold status
	int lineCurrent = styler.GetLine(startPos);
	if (startPos > 0) {
		if (lineCurrent > 0) {
			lineCurrent--;
			startPos = styler.LineStart(lineCurrent);
		}
	}
	int spaceFlags = 0;
	int indentCurrent = styler.IndentAmount(lineCurrent, &spaceFlags, IsAU3Comment);
	char chNext = styler[startPos];
	for (int i = startPos; i < endPos; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);

		if ((ch == '\r' && chNext != '\n') || (ch == '\n') || (i == endPos)) {
			int lev = indentCurrent;
			int indentNext = styler.IndentAmount(lineCurrent + 1, &spaceFlags, IsAU3Comment);
			if (!(indentCurrent & SC_FOLDLEVELWHITEFLAG)) {
				// Only non whitespace lines can be headers
				if ((indentCurrent & SC_FOLDLEVELNUMBERMASK) < (indentNext & SC_FOLDLEVELNUMBERMASK)) {
					lev |= SC_FOLDLEVELHEADERFLAG;
				} else if (indentNext & SC_FOLDLEVELWHITEFLAG) {
					// Line after is blank so check the next - maybe should continue further?
					int spaceFlags2 = 0;
					int indentNext2 = styler.IndentAmount(lineCurrent + 2, &spaceFlags2, IsAU3Comment);
					if ((indentCurrent & SC_FOLDLEVELNUMBERMASK) < (indentNext2 & SC_FOLDLEVELNUMBERMASK)) {
						lev |= SC_FOLDLEVELHEADERFLAG;
					}
				}
			}
			indentCurrent = indentNext;
			styler.SetLevel(lineCurrent, lev);
			lineCurrent++;
		}
	}

}



//

static const char * const AU3WordLists[] = {
    "#autoit keywords",
    "#autoit functions",
    "#autoit macros",
    "#autoit Sent keys",
    0
};
LexerModule lmAU3(SCLEX_AU3, ColouriseAU3Doc, "au3", FoldAU3Doc , AU3WordLists);
