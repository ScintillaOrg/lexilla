// Scintilla source code edit control
/** @file LexTADS3.cxx
 ** Lexer for TADS3.
 **/
/* Copyright 2005 by Michael Cartmell
 * Parts copyright 1998-2002 by Neil Hodgson <neilh@scintilla.org>
 * In particular FoldTADS3Doc is derived from FoldCppDoc
 * The License.txt file describes the conditions under which this software may
 * be distributed.
 */

/*
 * TADS3 is a language designed by Michael J. Roberts for the writing of text
 * based games.  TADS comes from Text Adventure Development System.  It has good
 * support for the processing and outputting of formatted text and much of a
 * TADS program listing consists of strings.
 *
 * TADS has two types of strings, those enclosed in single quotes (') and those
 * enclosed in double quotes (").  These strings have different symantics and
 * can be given different highlighting if desired.
 *
 * There can be embedded within both types of strings html tags
 * ( <tag key=value> ), library directives ( <.directive> ), and message
 * parameters ( {The doctor's/his} ).
 *
 * Double quoted strings can also contain interpolated expressions
 * ( << rug.moved ? ' and a hole in the floor. ' : nil >> ).  These expressions
 * may themselves contain single or double quoted strings, although the double
 * quoted strings may not contain interpolated expressions.
 *
 * These embedded constructs influence the output and formatting and are an
 * important part of a program and require highlighting.
 *
 * Because strings, html tags, library directives, message parameters, and
 * interpolated expressions may span multiple lines it is necessary to have
 * multiple states for a single construct so that the surrounding context can be
 * known.  This is important if scanning starts part way through a source file.
 *
 * States that have a Single quoted string context have _S_ in the name
 * States that have a Double quoted string context have _D_ in the name
 * States that have an interpolated eXpression context have _X_ in the name
 * eg SCE_T3_X_S_MSG_PARAM is a message parameter in a single quoted string
 * that is part of an interpolated expression.
 * "You see << isKnown? '{iobj/him} lying' : 'nothing' >> on the ground. "
 *                       ----------
 *
 * TO DO
 *
 * top level blocks can have one of two forms
 *
 * cave : DarkRoom 'small cave' 'small cave'
 *     "There is barely enough room to stand in this small cave. "
 * ;
 *
 * or
 *
 * cave : DarkRoom {
 *     'small cave'
 *     'small cave'
 *     "There is barely enough room to stand in this small cave. "
 * }
 *
 * currently only the latter form is folded.
 *
 * LINKS
 * http://www.tads.org/
 */

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

static unsigned int endPos;

static inline bool IsEOL(const int ch) {
	return ch == '\r' || ch == '\n';
}

static inline bool IsASpaceOrTab(const int ch) {
	return ch == ' ' || ch == '\t';
}

static inline bool IsATADS3Operator(const int ch) {
	return ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '|'
		|| ch == '!' || ch == ':' || ch == '?' || ch == '@' || ch == ';'
		|| ch == '&' || ch == '<' || ch == '>' || ch == '=';
}

static inline bool IsAWordChar(const int ch) {
	return isalnum(ch) || ch == '_';
}

static inline bool IsAWordStart(const int ch) {
	return isalpha(ch) || ch == '_';
}

static inline bool IsAHexDigit(const int ch) {
	int lch = tolower(ch);
	return isdigit(lch) || lch == 'a' || lch == 'b' || lch == 'c'
		|| lch == 'd' || lch == 'e' || lch == 'f';
}

static inline bool IsABracket(const int ch) {
	return ch == '(' || ch == '{' || ch == '['
		|| ch == ')' || ch == '}' || ch == ']';
}

static inline bool IsAMsgParamChar(int ch) {
	return isalpha(ch) || isspace(ch) || ch == '/' || ch == '\'' || ch == '\\';
}

static inline bool IsAnHTMLChar(int ch) {
	return isalnum(ch) || ch == '-' || ch == '_' || ch == '.';
}

static inline bool IsADirectiveChar(int ch) {
	return isalnum(ch) || isspace(ch) || ch == '-';
}

static inline bool IsANumberStart(StyleContext &sc) {
	return isdigit(sc.ch)
		|| (!isdigit(sc.chPrev) && sc.ch == '.' && isdigit(sc.chNext));
}

inline static void ColouriseTADS3Operator(StyleContext &sc) {
	int initState = sc.state;
	sc.SetState(SCE_T3_OPERATOR);
	sc.ForwardSetState(initState);
}

inline static void ColouriseTADS3Bracket(StyleContext &sc) {
	int initState = sc.state;
	sc.SetState(SCE_T3_BRACKET);
	sc.ForwardSetState(initState);
}

static void ColouriseTADSHTMLString(StyleContext &sc) {
	int initState = sc.state;
	int quoteChar = sc.ch;
	sc.SetState(SCE_T3_HTML_STRING);
	sc.Forward();

	while (sc.More()) {
		if (sc.Match('\\', static_cast<char>(quoteChar))) {
			sc.Forward(2);
		}
		if (sc.ch == quoteChar || IsEOL(sc.ch)) {
			sc.ForwardSetState(initState);
			return;
		}
		sc.Forward();
	}
}

static void ColouriseTADS3HTMLTagStart(StyleContext &sc) {
	sc.SetState(SCE_T3_HTML_TAG);
	sc.Forward();
	if (sc.ch == '/') {
		sc.Forward();
	}
	while (IsAnHTMLChar(sc.ch)) {
		sc.Forward();
	}
}

static void ColouriseTADS3HTMLTag(StyleContext &sc) {
	int initState = sc.state;
	switch (initState) {
		case SCE_T3_S_STRING:
			ColouriseTADS3HTMLTagStart(sc);
			sc.SetState(SCE_T3_S_H_DEFAULT);
			break;
		case SCE_T3_D_STRING:
			ColouriseTADS3HTMLTagStart(sc);
			sc.SetState(SCE_T3_S_H_DEFAULT);
			break;
		case SCE_T3_X_S_STRING:
			ColouriseTADS3HTMLTagStart(sc);
			sc.SetState(SCE_T3_X_S_H_DEFAULT);
			break;
		case SCE_T3_X_D_STRING:
			ColouriseTADS3HTMLTagStart(sc);
			sc.SetState(SCE_T3_X_D_H_DEFAULT);
			break;
		case SCE_T3_S_H_DEFAULT:
			initState = SCE_T3_S_STRING;
			break;
		case SCE_T3_D_H_DEFAULT:
			initState = SCE_T3_D_STRING;
			break;
		case SCE_T3_X_S_H_DEFAULT:
			initState = SCE_T3_X_S_STRING;
			break;
		case SCE_T3_X_D_H_DEFAULT:
			initState = SCE_T3_X_D_STRING;
			break;
	}

	while (sc.More()) {
		if (sc.Match('/', '>')) {
			sc.SetState(SCE_T3_HTML_TAG);
			sc.Forward(2);
			sc.SetState(initState);
			return;
		} else if (sc.ch == '>') {
			sc.SetState(SCE_T3_HTML_TAG);
			sc.ForwardSetState(initState);
			return;
		}
		if (sc.ch == '\'' || sc.ch == '"') {
			ColouriseTADSHTMLString(sc);
		} else if (sc.ch == '=') {
			ColouriseTADS3Operator(sc);
		} else {
			sc.Forward();
		}
	}
}

static void ColouriseTADS3Keyword(StyleContext &sc,
										 WordList *keywordlists[]) {
	static char s[250];
	WordList &keywords = *keywordlists[0];
	WordList &userwords1 = *keywordlists[1];
	WordList &userwords2 = *keywordlists[2];
	int initState = sc.state;
	sc.SetState(SCE_T3_KEYWORD);
	while (sc.More() && (IsAWordChar(sc.ch) || sc.ch == '.')) {
		sc.Forward();
	}
	sc.GetCurrent(s, sizeof(s));
	if (userwords1.InList(s)) {
		sc.ChangeState(SCE_T3_USER1);
	} else if (userwords2.InList(s)) {
		sc.ChangeState(SCE_T3_USER2);
	} else if (keywords.InList(s)) {
		// state already correct
	} else if ( strcmp(s, "is") == 0 || strcmp(s, "not") == 0) {
		// have to find if "in" is next
		int n = 1;
		while (n + sc.currentPos < endPos && IsASpaceOrTab(sc.GetRelative(n)))
			n++;
		if (sc.GetRelative(n) == 'i' && sc.GetRelative(n+1) == 'n') {
			sc.Forward(n+2);
		} else {
			sc.ChangeState(initState);
		}
	} else {
		sc.ChangeState(initState);
	}

	sc.SetState(initState);
}

static void ColouriseTADS3MsgParam(StyleContext &sc) {
	int initState = sc.state;
	switch (initState) {
		case SCE_T3_S_STRING:
			sc.SetState(SCE_T3_S_MSG_PARAM);
			sc.Forward();
			break;
		case SCE_T3_D_STRING:
			sc.SetState(SCE_T3_D_MSG_PARAM);
			sc.Forward();
			break;
		case SCE_T3_X_S_STRING:
			sc.SetState(SCE_T3_X_S_MSG_PARAM);
			sc.Forward();
			break;
		case SCE_T3_X_D_STRING:
			sc.SetState(SCE_T3_X_D_MSG_PARAM);
			sc.Forward();
			break;
		case SCE_T3_S_MSG_PARAM:
			initState = SCE_T3_S_STRING;
			break;
		case SCE_T3_D_MSG_PARAM:
			initState = SCE_T3_D_STRING;
			break;
		case SCE_T3_X_S_MSG_PARAM:
			initState = SCE_T3_X_S_STRING;
			break;
		case SCE_T3_X_D_MSG_PARAM:
			initState = SCE_T3_X_D_STRING;
			break;
	}
	while (sc.More() && IsAMsgParamChar(sc.ch)) {
		if (sc.ch == '\\') {
			if (initState == SCE_T3_D_STRING) {
				break;
			} else if (sc.chNext != '\'') {
				break;
			}
		}
		sc.Forward();
	}
	if (sc.ch == '}' || !sc.More()) {
		sc.ForwardSetState(initState);
	} else {
		sc.ChangeState(initState);
		sc.Forward();
	}
}

static void ColouriseTADS3LibDirective(StyleContext &sc) {
	int initState = sc.state;
	switch (initState) {
		case SCE_T3_S_STRING:
			sc.SetState(SCE_T3_S_LIB_DIRECTIVE);
			sc.Forward(2);
			break;
		case SCE_T3_D_STRING:
			sc.SetState(SCE_T3_D_LIB_DIRECTIVE);
			sc.Forward(2);
			break;
		case SCE_T3_X_S_STRING:
			sc.SetState(SCE_T3_X_S_LIB_DIRECTIVE);
			sc.Forward(2);
			break;
		case SCE_T3_X_D_STRING:
			sc.SetState(SCE_T3_X_D_LIB_DIRECTIVE);
			sc.Forward(2);
			break;
		case SCE_T3_S_LIB_DIRECTIVE:
			initState = SCE_T3_S_STRING;
			break;
		case SCE_T3_D_LIB_DIRECTIVE:
			initState = SCE_T3_D_STRING;
			break;
		case SCE_T3_X_S_LIB_DIRECTIVE:
			initState = SCE_T3_X_S_STRING;
			break;
		case SCE_T3_X_D_LIB_DIRECTIVE:
			initState = SCE_T3_X_D_STRING;
			break;
	}
	while (sc.More() && IsADirectiveChar(sc.ch)) {
		sc.Forward();
	};
	if (sc.ch == '>' || !sc.More()) {
		sc.ForwardSetState(initState);
	} else {
		sc.ChangeState(initState);
		sc.Forward();
	}
}

static void ColouriseTADS3String(StyleContext &sc) {
	int quoteChar = sc.ch;
	int initState = sc.state;
	switch (sc.state) {
		case SCE_T3_DEFAULT:
			if (quoteChar == '"') {
				sc.SetState(SCE_T3_D_STRING);
			} else {
				sc.SetState(SCE_T3_S_STRING);
			}
			sc.Forward();
			break;
		case SCE_T3_X_DEFAULT:
			if (quoteChar == '"') {
				sc.SetState(SCE_T3_X_D_STRING);
			} else {
				sc.SetState(SCE_T3_X_S_STRING);
			}
			sc.Forward();
			break;
		case SCE_T3_S_STRING:
			quoteChar = '\'';
			initState = SCE_T3_DEFAULT;
			break;
		case SCE_T3_D_STRING:
			quoteChar = '"';
			initState = SCE_T3_DEFAULT;
			break;
		case SCE_T3_X_S_STRING:
			quoteChar = '\'';
			initState = SCE_T3_X_DEFAULT;
			break;
		case SCE_T3_X_D_STRING:
			quoteChar = '"';
			initState = SCE_T3_X_DEFAULT;
			break;
	}
	while (sc.More()) {
		if (sc.Match('\\', static_cast<char>(quoteChar))) {
			sc.Forward(2);
		}
		if (sc.ch == quoteChar) {
			sc.ForwardSetState(initState);
			return;
		}
		if (sc.ch == '{') {
			ColouriseTADS3MsgParam(sc);
		} else if (sc.state == SCE_T3_D_STRING && sc.Match('<', '<')) {
			sc.SetState(SCE_T3_X_DEFAULT);
			sc.Forward(2);
			return;
		} else if (sc.Match('<', '.')) {
			ColouriseTADS3LibDirective(sc);
		} else if (sc.ch == '<') {
			ColouriseTADS3HTMLTag(sc);
		} else {
			sc.Forward();
		}
	}
}

static void ColouriseTADS3Comment(StyleContext &sc, const int initState,
								  const int endState) {
	if (sc.state != initState) {
		sc.SetState(initState);
	}
	for (; sc.More(); sc.Forward()) {
		if (sc.Match('*', '/')) {
			sc.Forward(2);
			sc.SetState(endState);
			return;
		}
	}
}

static void ColouriseToEndOfLine(StyleContext &sc, const int initState,
								 const int endState) {
	if (sc.state != initState) {
		sc.SetState(initState);
	}
	for (; sc.More(); sc.Forward()) {
		if (sc.ch == '\\') {
			if (IsEOL(sc.chNext)) {
				sc.Forward();
				if (sc.ch == '\r' && sc.chNext == '\n') {
					sc.Forward();
				}
				continue;
			}
		}
		if (IsEOL(sc.ch)) {
			sc.SetState(endState);
			return;
		}
	}
}

static void ColouriseTADS3Number(StyleContext &sc) {
	int initState = sc.state;
	bool inHexNumber = false;
	bool seenE = false;
	bool seenDot = sc.ch == '.';
	sc.SetState(SCE_T3_NUMBER);
	if (sc.More()) {
		sc.Forward();
	}
	if (sc.chPrev == '0' && tolower(sc.ch) == 'x') {
		inHexNumber = true;
		sc.Forward();
	}
	for (; sc.More(); sc.Forward()) {
		if (inHexNumber) {
			if (!IsAHexDigit(sc.ch)) {
				break;
			}
		} else if (!isdigit(sc.ch)) {
			if (!seenE && tolower(sc.ch) == 'e') {
				seenE = true;
				seenDot = true;
				if (sc.chNext == '+' || sc.chNext == '-') {
					sc.Forward();
				}
			} else if (!seenDot && sc.ch == '.') {
				seenDot = true;
			} else {
				break;
			}
		}
	}
	sc.SetState(initState);
}

static void ColouriseTADS3Doc(unsigned int startPos, int length, int initStyle,
							   WordList *keywordlists[], Accessor &styler) {
	int visibleChars = 0;
	int bracketLevel = 0;
	endPos = startPos + length;
	StyleContext sc(startPos, length, initStyle, styler);

	while (sc.More()) {

		if (IsEOL(sc.ch)) {
			visibleChars = 0;
			sc.Forward();
			continue;
		}

		switch(sc.state) {
			case SCE_T3_PREPROCESSOR:
			case SCE_T3_LINE_COMMENT:
				ColouriseToEndOfLine(sc, sc.state, SCE_T3_DEFAULT);
				break;
			case SCE_T3_X_PREPROCESSOR:
			case SCE_T3_X_LINE_COMMENT:
				ColouriseToEndOfLine(sc, sc.state, SCE_T3_X_DEFAULT);
				break;
			case SCE_T3_S_STRING:
			case SCE_T3_D_STRING:
			case SCE_T3_X_S_STRING:
			case SCE_T3_X_D_STRING:
				ColouriseTADS3String(sc);
				visibleChars++;
				break;
			case SCE_T3_S_MSG_PARAM:
			case SCE_T3_D_MSG_PARAM:
			case SCE_T3_X_S_MSG_PARAM:
			case SCE_T3_X_D_MSG_PARAM:
				ColouriseTADS3MsgParam(sc);
				break;
			case SCE_T3_S_LIB_DIRECTIVE:
			case SCE_T3_D_LIB_DIRECTIVE:
			case SCE_T3_X_S_LIB_DIRECTIVE:
			case SCE_T3_X_D_LIB_DIRECTIVE:
				ColouriseTADS3LibDirective(sc);
				break;
			case SCE_T3_S_H_DEFAULT:
			case SCE_T3_D_H_DEFAULT:
			case SCE_T3_X_S_H_DEFAULT:
			case SCE_T3_X_D_H_DEFAULT:
				ColouriseTADS3HTMLTag(sc);
				break;
			case SCE_T3_BLOCK_COMMENT:
				ColouriseTADS3Comment(sc, SCE_T3_BLOCK_COMMENT, SCE_T3_DEFAULT);
				break;
			case SCE_T3_X_BLOCK_COMMENT:
				ColouriseTADS3Comment(sc, SCE_T3_X_BLOCK_COMMENT, SCE_T3_X_DEFAULT);
				break;
			case SCE_T3_DEFAULT:
				if (IsASpaceOrTab(sc.ch)) {
					sc.Forward();
				} else if (sc.ch == '#' && visibleChars == 0) {
					ColouriseToEndOfLine(sc, SCE_T3_PREPROCESSOR, SCE_T3_DEFAULT);
				} else if (sc.Match('/', '*')) {
					ColouriseTADS3Comment(sc, SCE_T3_BLOCK_COMMENT, SCE_T3_DEFAULT);
					visibleChars++;
				} else if (sc.Match('/', '/')) {
					ColouriseToEndOfLine(sc, SCE_T3_LINE_COMMENT, SCE_T3_DEFAULT);
				} else if (sc.ch == '\'' || sc.ch == '"') {
					ColouriseTADS3String(sc);
					visibleChars++;
				} else if (IsATADS3Operator(sc.ch)) {
					ColouriseTADS3Operator(sc);
					visibleChars++;
				} else if (IsANumberStart(sc)) {
					ColouriseTADS3Number(sc);
					visibleChars++;
				} else if (IsABracket(sc.ch)) {
					ColouriseTADS3Bracket(sc);
					visibleChars++;
				} else if (IsAWordStart(sc.ch)) {
					ColouriseTADS3Keyword(sc, keywordlists);
					visibleChars++;
				} else {
					sc.Forward();
					visibleChars++;
				}
				break;
			case SCE_T3_X_DEFAULT:
				if (IsASpaceOrTab(sc.ch)) {
					sc.Forward();
				} else if (sc.ch == '#' && visibleChars == 0) {
					ColouriseToEndOfLine(sc, SCE_T3_X_PREPROCESSOR, SCE_T3_X_DEFAULT);
				} else if (sc.Match('/', '*')) {
					ColouriseTADS3Comment(sc, SCE_T3_X_BLOCK_COMMENT, SCE_T3_X_DEFAULT);
					visibleChars++;
				} else if (sc.Match('/', '/')) {
					ColouriseToEndOfLine(sc, SCE_T3_X_LINE_COMMENT, SCE_T3_X_DEFAULT);
				} else if (sc.ch == '\'' || sc.ch == '"') {
					ColouriseTADS3String(sc);
					visibleChars++;
				} else if (bracketLevel == 0 && sc.Match('>', '>')) {
					sc.Forward(2);
					sc.SetState(SCE_T3_D_STRING);
				} else if (IsATADS3Operator(sc.ch)) {
					ColouriseTADS3Operator(sc);
					visibleChars++;
				} else if (IsANumberStart(sc)) {
					ColouriseTADS3Number(sc);
					visibleChars++;
				} else if (IsABracket(sc.ch)) {
					if (sc.ch == '(') {
						bracketLevel++;
					} else if (sc.ch == ')') {
						bracketLevel && bracketLevel--;
					}
					ColouriseTADS3Bracket(sc);
					visibleChars++;
				} else if (IsAWordStart(sc.ch)) {
					ColouriseTADS3Keyword(sc, keywordlists);
					visibleChars++;
				} else {
					sc.Forward();
					visibleChars++;
				}
				break;
			default:
				sc.SetState(SCE_T3_DEFAULT);
				sc.Forward();
		}
	}
	sc.Complete();
}

static inline bool IsStreamCommentStyle(int style) {
	return style == SCE_T3_BLOCK_COMMENT
		|| style == SCE_T3_X_BLOCK_COMMENT;
}

static inline bool IsStringTransition(int s1, int s2) {
	switch (s1) {
		case SCE_T3_S_STRING:
			return s2 != s1
				&& s2 != SCE_T3_S_LIB_DIRECTIVE
				&& s2 != SCE_T3_S_MSG_PARAM
				&& s2 != SCE_T3_HTML_TAG;
		case SCE_T3_D_STRING:
			return s2 != s1
				&& s2 != SCE_T3_D_LIB_DIRECTIVE
				&& s2 != SCE_T3_D_MSG_PARAM
				&& s2 != SCE_T3_HTML_TAG
				&& s2 != SCE_T3_X_DEFAULT;
		case SCE_T3_X_S_STRING:
			return s2 != s1
				&& s2 != SCE_T3_X_S_LIB_DIRECTIVE
				&& s2 != SCE_T3_X_S_MSG_PARAM
				&& s2 != SCE_T3_HTML_TAG;
		case SCE_T3_X_D_STRING:
			return s2 != s1
				&& s2 != SCE_T3_X_D_LIB_DIRECTIVE
				&& s2 != SCE_T3_X_D_MSG_PARAM
				&& s2 != SCE_T3_HTML_TAG;
		default:
			return false;
	}
}

static void FoldTADS3Doc(unsigned int startPos, int length, int initStyle,
                            WordList *[], Accessor &styler) {
	unsigned int endPos = startPos + length;
	int lineCurrent = styler.GetLine(startPos);
	int levelCurrent = SC_FOLDLEVELBASE;
	if (lineCurrent > 0)
		levelCurrent = styler.LevelAt(lineCurrent-1) >> 16;
	int levelMinCurrent = levelCurrent;
	int levelNext = levelCurrent;
	char chNext = styler[startPos];
	int styleNext = styler.StyleAt(startPos);
	int style = initStyle;
	for (unsigned int i = startPos; i < endPos; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		int stylePrev = style;
		style = styleNext;
		styleNext = styler.StyleAt(i + 1);
		bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
		if (IsStreamCommentStyle(style)) {
			if (!IsStreamCommentStyle(stylePrev)) {
				levelNext++;
			} else if (!IsStreamCommentStyle(styleNext) && !atEOL) {
				// Comments don't end at end of line and the next character may be unstyled.
				levelNext--;
			}
		} else if (style == SCE_T3_BRACKET) {
			if (ch == '{' || ch == '[') {
				// Measure the minimum before a '{' to allow
				// folding on "} else {"
				if (levelMinCurrent > levelNext) {
					levelMinCurrent = levelNext;
				}
				levelNext++;
			} else if (ch == '}' || ch == ']') {
				levelNext--;
			}
		} else if ((ch == '\'' || ch == '"')) {
			if (IsStringTransition(style, stylePrev)) {
				if (levelMinCurrent > levelNext) {
					levelMinCurrent = levelNext;
				}
				levelNext++;
			} else if (IsStringTransition(style, styleNext)) {
				levelNext--;
			}
		}

		if (atEOL) {
			int lev = levelMinCurrent | levelNext << 16;
			if (levelMinCurrent < levelNext)
				lev |= SC_FOLDLEVELHEADERFLAG;
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			levelCurrent = levelNext;
			levelMinCurrent = levelCurrent;
		}
	}
}

static const char * const tads3WordList[] = {
	"TADS3 Keywords",
	"User defined 1",
	"User defined 2",
	0
};

LexerModule lmTADS3(SCLEX_TADS3, ColouriseTADS3Doc, "tads3", FoldTADS3Doc, tads3WordList);
