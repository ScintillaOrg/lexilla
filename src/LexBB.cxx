// Scintilla source code edit control
/** @file LexBB.cxx
 ** Lexer for BlitzBasic.
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

// Currently, this only supports the old (2D only) BlitzBasic. But I believe relatively
// few changes would be needed to support the newer versions (Blitz3D, BlitzPlus,
// BlitzMax, ...) - but I don't have them. Mail me (elias <at> users <dot> sf <dot> net)
// for any bugs.

// Folding works for Function/End Function and Type/End Type only.

// The generated styles are:
// val SCE_BB_DEFAULT=0
// val SCE_BB_COMMENT=1
// val SCE_BB_NUMBER=2
// val SCE_BB_STRING=3
// val SCE_BB_OPERATOR=4
// val SCE_BB_IDENTIFIER=5
// val SCE_BB_KEYWORD=6
// val SCE_BB_KEYWORD2=7
// val SCE_BB_KEYWORD3=8
// val SCE_BB_KEYWORD4=9
// val SCE_BB_ERROR=10

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

static bool IsOperator(int c) {
	return isoperator(static_cast<char>(c)) || c == '#' || c == '$' || c == '\\';
}

static bool IsIdentifier(int c) {
	return isalnum(c) || c == '_' || c == '.';
}

static bool IsDigit(int c) {
	return isdigit(c) || c == '.';
}

static void ColouriseBBDoc(unsigned int startPos, int length, int initStyle,
                           WordList *keywordlists[], Accessor &styler) {
	styler.StartAt(startPos);

	StyleContext sc(startPos, length, initStyle, styler);

	// Can't use sc.More() here else we miss the last character
	for (; ; sc.Forward()) {
		if (sc.state == SCE_BB_IDENTIFIER) {
			if (!IsIdentifier(sc.ch)) {
				char s[100];
				sc.GetCurrentLowered(s, sizeof(s));
				for (int i = 0; i < 4; i++)
				{
					if (keywordlists[i]->InList(s)) {
						sc.ChangeState(SCE_BB_KEYWORD + i);
						sc.SetState(SCE_BB_IDENTIFIER);
					}
				}
				sc.SetState(SCE_BB_DEFAULT);
			}
		} else if (sc.state == SCE_BB_OPERATOR) {
			if (!IsOperator(sc.ch))
				sc.SetState(SCE_BB_DEFAULT);
		} else if (sc.state == SCE_BB_NUMBER) {
			if (!IsDigit(sc.ch)) {
				sc.SetState(SCE_BB_DEFAULT);
			}
		} else if (sc.state == SCE_BB_STRING) {
			if (sc.ch == '"') {
				sc.ForwardSetState(SCE_BB_DEFAULT);
			}
			if (sc.atLineEnd) {
				sc.ChangeState(SCE_BB_ERROR);
				sc.SetState(SCE_BB_DEFAULT);
			}
		} else if (sc.state == SCE_BB_COMMENT) {
			if (sc.atLineEnd) {
				sc.SetState(SCE_BB_DEFAULT);
			}
		}

		if (sc.state == SCE_BB_DEFAULT || sc.state == SCE_BB_ERROR) {
			if (sc.Match(';')) {
				sc.SetState(SCE_BB_COMMENT);
			} else if (sc.Match('"')) {
				sc.SetState(SCE_BB_STRING);
			} else if (IsDigit(sc.ch)) {
				sc.SetState(SCE_BB_NUMBER);
			} else if (IsOperator(sc.ch)) {
				sc.SetState(SCE_BB_OPERATOR);
			} else if (isalnum(sc.ch) || sc.Match('_')) {
				sc.SetState(SCE_BB_IDENTIFIER);
			} else if (!isspace(sc.ch)) {
				sc.SetState(SCE_BB_ERROR);
			}
		}
		if (!sc.More())
			break;
	}
	sc.Complete();
}

static void FoldBBDoc(unsigned int startPos, int length, int,
	WordList *[], Accessor &styler) {
	int line = styler.GetLine(startPos);
	int level = styler.LevelAt(line);
	int go = 0;
	int endPos = startPos + length;
	char word[256];
	int wordlen = 0;
	int i;
	for (i = startPos; i < endPos; i++) {
		int c = styler.SafeGetCharAt(i);
		if (wordlen) {
			word[wordlen] = static_cast<char>(tolower(c));
			if (isspace(c)) {
				word[wordlen] = '\0';
				if (!strcmp(word, "function") ||
					!strcmp(word, "type")) {
					level |= SC_FOLDLEVELHEADERFLAG;
					go = 1;
				}
				if (!strcmp(word, "end function") ||
					!strcmp(word, "end type")) {
					go = -1;
				}
				// Treat any whitespace as single blank.
				if (!isspace (word[wordlen - 1])) {
					word[wordlen] = ' ';
					wordlen++;
				}
			} else {
				wordlen++;
			}
		} else {
			if (!isspace(c))
				word[wordlen++] = static_cast<char>(tolower(c));
		}
		if (c == '\n') {
			if (wordlen == 0)
				level |= SC_FOLDLEVELWHITEFLAG;
			if (level != styler.LevelAt(line))
				styler.SetLevel(line, level);
			wordlen = 0;
			level &= ~SC_FOLDLEVELHEADERFLAG;
			level &= ~SC_FOLDLEVELWHITEFLAG;
			level += go;
			go = 0;
			line++;
		}
	}
}

static const char * const bbWordListDesc[] = {
	"Blitzbasic Keywords",
	"user1",
	"user2",
	"user3",
	0
};

LexerModule lmBB(SCLEX_BB, ColouriseBBDoc, "bb", FoldBBDoc, bbWordListDesc);

