// SciTE - Scintilla based Text Editor
// LexHTML.cxx - lexer for HTML
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

enum { eScriptNone = 0, eScriptJS, eScriptVBS, eScriptPython, eScriptPHP, eScriptXML };
static int segIsScriptingIndicator(Accessor &styler, unsigned int start, unsigned int end, int prevValue) {
	char s[30 + 1];
	s[0] = '\0';
	for (unsigned int i = 0; i < end - start + 1 && i < 30; i++) {
		s[i] = static_cast<char>(tolower(styler[start + i]));
		s[i + 1] = '\0';
	}
	//Platform::DebugPrintf("Scripting indicator [%s]\n", s);
	if (strstr(s, "vbs"))
		return eScriptVBS;
	if (strstr(s, "pyth"))
		return eScriptPython;
	if (strstr(s, "javas"))
		return eScriptJS;
	if (strstr(s, "jscr"))
		return eScriptJS;
	if (strstr(s, "php"))
		return eScriptPHP;
	if (strstr(s, "xml"))
		return eScriptXML;

	return prevValue;
}

static int PrintScriptingIndicatorOffset(Accessor &styler, unsigned int start, unsigned int end) {
	int iResult = 0;
	char s[30 + 1];
	s[0] = '\0';
	for (unsigned int i = 0; i < end - start + 1 && i < 30; i++) {
		s[i] = static_cast<char>(tolower(styler[start + i]));
		s[i + 1] = '\0';
	}
	if (0 == strncmp(s, "php", 3)) {
		iResult = 3;
	}

	return iResult;
}

static void classifyAttribHTML(unsigned int start, unsigned int end, WordList &keywords, Accessor &styler) {
	bool wordIsNumber = isdigit(styler[start]) || (styler[start] == '.') ||
	                    (styler[start] == '-') || (styler[start] == '#');
	char chAttr = SCE_H_ATTRIBUTEUNKNOWN;
	if (wordIsNumber) {
		chAttr = SCE_H_NUMBER;
	} else {
		char s[30 + 1];
		s[0] = '\0';
		for (unsigned int i = 0; i < end - start + 1 && i < 30; i++) {
			s[i] = static_cast<char>(tolower(styler[start + i]));
			s[i + 1] = '\0';
		}
		if (keywords.InList(s))
			chAttr = SCE_H_ATTRIBUTE;
	}
	if ((chAttr == SCE_H_ATTRIBUTEUNKNOWN) && !keywords)
		// No keywords -> all are known
		chAttr = SCE_H_ATTRIBUTE;
	styler.ColourTo(end, chAttr);
}

static int classifyTagHTML(unsigned int start, unsigned int end,
                           WordList &keywords, Accessor &styler) {
	char s[30 + 1];
	// Copy after the '<'
	unsigned int i = 0;
	for (unsigned int cPos = start; cPos <= end && i < 30; cPos++) {
		char ch = styler[cPos];
		if (ch != '<')
			s[i++] = static_cast<char>(tolower(ch));
	}
	s[i] = '\0';
	char chAttr = SCE_H_TAGUNKNOWN;
	if (s[0] == '!' && s[1] == '-' && s[2] == '-') {	//Comment
		chAttr = SCE_H_COMMENT;
	} else if (strcmp(s, "![cdata[") == 0) {	// In lower case because already converted
		chAttr = SCE_H_CDATA;
	} else if (s[0] == '/') {	// Closing tag
		if (keywords.InList(s + 1))
			chAttr = SCE_H_TAG;
	} else {
		if (keywords.InList(s)) {
			chAttr = SCE_H_TAG;
		}
		if (0 == strcmp(s, "script")) {
			chAttr = SCE_H_SCRIPT;
		}
	}
	if ((chAttr == SCE_H_TAGUNKNOWN) && !keywords)
		// No keywords -> all are known
		chAttr = SCE_H_TAG;
	styler.ColourTo(end, chAttr);
	return chAttr;
}

static void classifyWordHTJS(unsigned int start, unsigned int end,
                             WordList &keywords, Accessor &styler) {
	char chAttr = SCE_HJ_WORD;
	bool wordIsNumber = isdigit(styler[start]) || (styler[start] == '.');
	if (wordIsNumber)
		chAttr = SCE_HJ_NUMBER;
	else {
		char s[30 + 1];
		for (unsigned int i = 0; i < end - start + 1 && i < 30; i++) {
			s[i] = styler[start + i];
			s[i + 1] = '\0';
		}
		if (keywords.InList(s))
			chAttr = SCE_HJ_KEYWORD;
	}
	styler.ColourTo(end, chAttr);
}

static void classifyWordHTJSA(unsigned int start, unsigned int end,
                              WordList &keywords, Accessor &styler) {
	char chAttr = SCE_HJA_WORD;
	bool wordIsNumber = isdigit(styler[start]) || (styler[start] == '.');
	if (wordIsNumber)
		chAttr = SCE_HJA_NUMBER;
	else {
		char s[30 + 1];
		for (unsigned int i = 0; i < end - start + 1 && i < 30; i++) {
			s[i] = styler[start + i];
			s[i + 1] = '\0';
		}
		if (keywords.InList(s))
			chAttr = SCE_HJA_KEYWORD;
	}
	styler.ColourTo(end, chAttr);
}

static int classifyWordHTVB(unsigned int start, unsigned int end, WordList &keywords, Accessor &styler) {
	char chAttr = SCE_HB_IDENTIFIER;
	bool wordIsNumber = isdigit(styler[start]) || (styler[start] == '.');
	if (wordIsNumber)
		chAttr = SCE_HB_NUMBER;
	else {
		char s[30 + 1];
		for (unsigned int i = 0; i < end - start + 1 && i < 30; i++) {
			s[i] = static_cast<char>(tolower(styler[start + i]));
			s[i + 1] = '\0';
		}
		if (keywords.InList(s)) {
			chAttr = SCE_HB_WORD;
			if (strcmp(s, "rem") == 0)
				chAttr = SCE_HB_COMMENTLINE;
		}
	}
	styler.ColourTo(end, chAttr);
	if (chAttr == SCE_HB_COMMENTLINE)
		return SCE_HB_COMMENTLINE;
	else
		return SCE_HB_DEFAULT;
}

static int classifyWordHTVBA(unsigned int start, unsigned int end, WordList &keywords, Accessor &styler) {
	char chAttr = SCE_HBA_IDENTIFIER;
	bool wordIsNumber = isdigit(styler[start]) || (styler[start] == '.');
	if (wordIsNumber)
		chAttr = SCE_HBA_NUMBER;
	else {
		char s[30 + 1];
		for (unsigned int i = 0; i < end - start + 1 && i < 30; i++) {
			s[i] = static_cast<char>(tolower(styler[start + i]));
			s[i + 1] = '\0';
		}
		if (keywords.InList(s)) {
			chAttr = SCE_HBA_WORD;
			if (strcmp(s, "rem") == 0)
				chAttr = SCE_HBA_COMMENTLINE;
		}
	}
	styler.ColourTo(end, chAttr);
	if (chAttr == SCE_HBA_COMMENTLINE)
		return SCE_HBA_COMMENTLINE;
	else
		return SCE_HBA_DEFAULT;
}

static void classifyWordHTPy(unsigned int start, unsigned int end, WordList &keywords, Accessor &styler, char *prevWord) {
	bool wordIsNumber = isdigit(styler[start]);
	char s[30 + 1];
	for (unsigned int i = 0; i < end - start + 1 && i < 30; i++) {
		s[i] = styler[start + i];
		s[i + 1] = '\0';
	}
	char chAttr = SCE_HP_IDENTIFIER;
	if (0 == strcmp(prevWord, "class"))
		chAttr = SCE_HP_CLASSNAME;
	else if (0 == strcmp(prevWord, "def"))
		chAttr = SCE_HP_DEFNAME;
	else if (wordIsNumber)
		chAttr = SCE_HP_NUMBER;
	else if (keywords.InList(s))
		chAttr = SCE_HP_WORD;
	styler.ColourTo(end, chAttr);
	strcpy(prevWord, s);
}

static void classifyWordHTPyA(unsigned int start, unsigned int end, WordList &keywords, Accessor &styler, char *prevWord) {
	bool wordIsNumber = isdigit(styler[start]);
	char s[30 + 1];
	for (unsigned int i = 0; i < end - start + 1 && i < 30; i++) {
		s[i] = styler[start + i];
		s[i + 1] = '\0';
	}
	char chAttr = SCE_HPA_IDENTIFIER;
	if (0 == strcmp(prevWord, "class"))
		chAttr = SCE_HPA_CLASSNAME;
	else if (0 == strcmp(prevWord, "def"))
		chAttr = SCE_HPA_DEFNAME;
	else if (wordIsNumber)
		chAttr = SCE_HPA_NUMBER;
	else if (keywords.InList(s))
		chAttr = SCE_HPA_WORD;
	styler.ColourTo(end, chAttr);
	strcpy(prevWord, s);
}

// Update the word colour to default or keyword
// Called when in a PHP word
static void classifyWordHTPHPA(unsigned int start, unsigned int end, WordList &keywords, Accessor &styler) {
	char chAttr = SCE_HPHP_DEFAULT;
	bool wordIsNumber = isdigit(styler[start]);
	if (wordIsNumber)
		chAttr = SCE_HPHP_NUMBER;
	else {
		char s[30 + 1];
		for (unsigned int i = 0; i < end - start + 1 && i < 30; i++) {
			s[i] = styler[start + i];
			s[i + 1] = '\0';
		}
		if (keywords.InList(s))
			chAttr = SCE_HPHP_WORD;
	}
	styler.ColourTo(end, chAttr);
}

// Return the first state to reach when entering a scripting language
static int StateForScript(int scriptLanguage, int inScriptTag) {
	switch (scriptLanguage) {
	case eScriptVBS:
		if (inScriptTag)
			return SCE_HB_START;
		else
			return SCE_HBA_START;
	case eScriptPython:
		if (inScriptTag)
			return SCE_HP_START;
		else
			return SCE_HPA_START;
	case eScriptPHP:
		return SCE_HPHP_DEFAULT;
	case eScriptXML:
		return SCE_H_TAGUNKNOWN;
	default :
		return SCE_HJ_START;
	}
}

inline bool ishtmlwordchar(char ch) {
	return isalnum(ch) || ch == '.' || ch == '-' || ch == '_' || ch == ':' || ch == '!' || ch == '#';
}

static bool InTagState(int state) {
	return state == SCE_H_TAG || state == SCE_H_TAGUNKNOWN ||
	       state == SCE_H_SCRIPT ||
	       state == SCE_H_ATTRIBUTE || state == SCE_H_ATTRIBUTEUNKNOWN ||
	       state == SCE_H_NUMBER || state == SCE_H_OTHER ||
	       state == SCE_H_DOUBLESTRING || state == SCE_H_SINGLESTRING;
}

static bool isLineEnd(char ch) {
	return ch == '\r' || ch == '\n';
}

static void ColouriseHyperTextDoc(unsigned int startPos, int length, int initStyle, WordList *keywordlists[],
                                  Accessor &styler) {

	WordList &keywords = *keywordlists[0];
	WordList &keywords2 = *keywordlists[1];
	WordList &keywords3 = *keywordlists[2];
	WordList &keywords4 = *keywordlists[3];
	WordList &keywords5 = *keywordlists[4];

	// Lexer for HTML requires more lexical states (7 bits worth) than most lexers
	styler.StartAt(startPos, 127);
	bool lastTagWasScript = false;
	char prevWord[200];
	prevWord[0] = '\0';
	int scriptLanguage = eScriptJS;
	int state = initStyle;

	// If inside a tag, it may be a script tag, so reread from the start to ensure any language tags are seen
	if (InTagState(state)) {
		while ((startPos > 1) && (InTagState(styler.StyleAt(startPos - 1)))) {
			startPos--;
		}
		state = SCE_H_DEFAULT;
	}
	styler.StartAt(startPos, 127);

	int lineState = eScriptVBS;
	int lineCurrent = styler.GetLine(startPos);
	if (lineCurrent > 0)
		lineState = styler.GetLineState(lineCurrent);
	int beforeNonHTML = (lineState & 0x01) >> 0;
	int inNonHTML = (lineState & 0x02) >> 1;
	int inScriptTag = (lineState & 0x04) >> 2;
	int defaultScript = (lineState & 0xF0) >> 4;

	char chPrev = ' ';
	char chPrev2 = ' ';
	styler.StartSegment(startPos);
	int lengthDoc = startPos + length;
	for (int i = startPos; i < lengthDoc; i++) {
		char ch = styler[i];
		char chNext = styler.SafeGetCharAt(i + 1);
		char chNext2 = styler.SafeGetCharAt(i + 2);

		// Handle DBCS codepages
		if (styler.IsLeadByte(ch)) {
			chPrev2 = ' ';
			chPrev = ' ';
			i += 1;
			continue;
		}

		if ((ch == '\r' && chNext != '\n') || (ch == '\n')) {
			// New line -> record any line state onto /next/ line
			lineCurrent++;
			styler.SetLineState(lineCurrent,
			                    (beforeNonHTML & 0x01) | ((inNonHTML & 0x01) << 1) | ((inScriptTag & 0x01) << 2) | ((defaultScript & 0x0F) << 4));
		}

		if (inScriptTag && (ch == '<') && (chNext == '/')) {
			// Check if it's the end of the script tag (or any other HTML tag)
			switch (state) {
				// in these cases, you can embed HTML tags (to confirm !!!!!!!!!!!!!!!!!!!!!!)
			case SCE_H_DOUBLESTRING:
			case SCE_H_SINGLESTRING:
			case SCE_HJ_COMMENTLINE:
			case SCE_HJ_DOUBLESTRING:
			case SCE_HJ_SINGLESTRING:
			case SCE_HJA_COMMENTLINE:
			case SCE_HJA_DOUBLESTRING:
			case SCE_HJA_SINGLESTRING:
			case SCE_HB_COMMENTLINE:
			case SCE_HB_STRING:
			case SCE_HBA_COMMENTLINE:
			case SCE_HBA_STRING:
			case SCE_HP_COMMENTLINE:
			case SCE_HP_STRING:
			case SCE_HP_TRIPLE:
			case SCE_HP_TRIPLEDOUBLE:
			case SCE_HPA_COMMENTLINE:
			case SCE_HPA_STRING:
			case SCE_HPA_TRIPLE:
			case SCE_HPA_TRIPLEDOUBLE:
				break;
			default :
				// maybe we should check here if it's a tag and if it's SCRIPT

				styler.ColourTo(i - 1, state);
				state = SCE_H_TAGUNKNOWN;
				inNonHTML = 0;
				inScriptTag = 0;
				i++;
				continue;
				break;
			}
		}

		/////////////////////////////////////
		// handle the start of PHP pre-processor = Non-HTML
		if ((ch == '<') && (chNext == '?')) {
			styler.ColourTo(i - 1, state);
			beforeNonHTML = state;
			scriptLanguage = segIsScriptingIndicator(styler, styler.GetStartSegment(), i + 10, eScriptPHP);
			i += 1 + PrintScriptingIndicatorOffset(styler, i + 2, i + 10);
			if (scriptLanguage == eScriptXML)
				styler.ColourTo(i, SCE_H_XMLSTART);
			else
				styler.ColourTo(i, SCE_H_QUESTION);
			state = StateForScript(scriptLanguage, inScriptTag);
			inNonHTML = 1;
			continue;
		}
		// handle the start of ASP pre-processor = Non-HTML
		if ((ch == '<') && (chNext == '%')) {
			styler.ColourTo(i - 1, state);
			beforeNonHTML = state;
			if (chNext2 == '@') {
				i += 2; // place as if it was the second next char treated
				state = SCE_H_ASPAT;
			} else {
				if (chNext2 == '=') {
					i += 2; // place as if it was the second next char treated
				}
				else {
					i++; // place as if it was the next char treated
				}

				state = StateForScript(defaultScript, inScriptTag);
			}
			styler.ColourTo(i, SCE_H_ASP);
			inNonHTML = 1;
			inScriptTag = 0;
			continue;
		}
		/////////////////////////////////////
		/////////////////////////////////////
		// handle of end of a pre-processor = Non-HTML
		/* this code is probably un-used (kept for safety)
				if (inNonHTML && inScriptTag && (ch == '<') && (chNext == '/')) {
					// Bounce out of any ASP mode
					switch (state)
					{
						case SCE_HJA_WORD:
							classifyWordHTJSA(styler.GetStartSegment(), i - 1, keywords2, styler);
							break;
						case SCE_HBA_WORD:
							classifyWordHTVBA(styler.GetStartSegment(), i - 1, keywords3, styler);
							break;
						case SCE_HPA_WORD:
							classifyWordHTPyA(styler.GetStartSegment(), i - 1, keywords4, styler, prevWord);
							break;
						case SCE_HPHP_WORD:
							classifyWordHTPHPA(styler.GetStartSegment(), i - 1, keywords5, styler);
							break;
						default :
							styler.ColourTo(i - 1, state);
							break;
					}
					if ((ch == '<') && (chNext == '/')) {
						state = SCE_H_TAGUNKNOWN;
					} else {
						i++;
						if (ch == '%')
							styler.ColourTo(i, SCE_H_ASP);
						else
		//					styler.ColourTo(i, SCE_H_XMLEND);
							styler.ColourTo(i, SCE_HPHP_);
						state = beforeNonHTML;
					}
					beforeNonHTML = SCE_H_DEFAULT;
					inNonHTML = 0;
					inScriptTag = 0;
					continue;
				}
		*/
		// handle the end of a pre-processor = Non-HTML
		if (inNonHTML && ((ch == '?') || (ch == '%')) && (chNext == '>')) {
			if (state == SCE_H_ASPAT) {
				defaultScript = segIsScriptingIndicator(styler, styler.GetStartSegment(), i - 1, defaultScript);
			}
			// Bounce out of any ASP mode
			switch (state) {
			case SCE_HJA_WORD:
				classifyWordHTJSA(styler.GetStartSegment(), i - 1, keywords2, styler);
				break;
			case SCE_HBA_WORD:
				classifyWordHTVBA(styler.GetStartSegment(), i - 1, keywords3, styler);
				break;
			case SCE_HPA_WORD:
				classifyWordHTPyA(styler.GetStartSegment(), i - 1, keywords4, styler, prevWord);
				break;
			case SCE_HPHP_WORD:
				classifyWordHTPHPA(styler.GetStartSegment(), i - 1, keywords5, styler);
				break;
			default :
				styler.ColourTo(i - 1, state);
				break;
			}
			if ((ch == '<') && (chNext == '/')) {
				state = SCE_H_TAGUNKNOWN;
			} else {
				i++;
				if (ch == '%')
					styler.ColourTo(i, SCE_H_ASP);
				else if (scriptLanguage == eScriptXML)
					styler.ColourTo(i, SCE_H_XMLEND);
				else
					styler.ColourTo(i, SCE_H_QUESTION);
				state = beforeNonHTML;
			}
			beforeNonHTML = SCE_H_DEFAULT;
			inNonHTML = 0;
			inScriptTag = 0;
			continue;
		}
		/////////////////////////////////////

		switch (state) {
		case SCE_H_DEFAULT:
			if (ch == '<') {
				styler.ColourTo(i - 1, state);
				state = SCE_H_TAGUNKNOWN;
			} else if (ch == '&') {
				styler.ColourTo(i - 1, SCE_H_DEFAULT);
				state = SCE_H_ENTITY;
			}
			break;
		case SCE_H_COMMENT:
			if ((ch == '>') && (chPrev == '-')) {
				styler.ColourTo(i, state);
				state = SCE_H_DEFAULT;
			}
			break;
		case SCE_H_CDATA:
			if ((ch == '>') && (chPrev == ']') && (chPrev2 == ']')) {
				styler.ColourTo(i, state);
				state = SCE_H_DEFAULT;
			}
			break;
		case SCE_H_ENTITY:
			if (ch == ';') {
				styler.ColourTo(i, state);
				state = SCE_H_DEFAULT;
			}
			if (ch != '#' && !isalnum(ch)) {	// Should check that '#' follows '&', but it is unlikely anyway...
				styler.ColourTo(i, SCE_H_TAGUNKNOWN);
				state = SCE_H_DEFAULT;
			}
			break;
		case SCE_H_TAGUNKNOWN:
			if (!ishtmlwordchar(ch) && ch != '/' && ch != '-' && ch != '[') {
				int eClass = classifyTagHTML(styler.GetStartSegment(), i - 1, keywords, styler);
				lastTagWasScript = eClass == SCE_H_SCRIPT;
				if (lastTagWasScript) {
					inScriptTag = 1;
					scriptLanguage = eScriptJS; // default to javascript
					eClass = SCE_H_TAG;
				}
				if ((ch == '>') && (eClass != SCE_H_COMMENT)) {
					styler.ColourTo(i, SCE_H_TAG);
					if (lastTagWasScript) {
						state = StateForScript(scriptLanguage, inScriptTag);
					} else {
						state = SCE_H_DEFAULT;
					}
				} else {
					if (eClass == SCE_H_COMMENT) {
						state = SCE_H_COMMENT;
					} else if (eClass == SCE_H_CDATA) {
						state = SCE_H_CDATA;
					} else {
						state = SCE_H_OTHER;
					}
				}
			}
			break;
		case SCE_H_ATTRIBUTE:
			if (!ishtmlwordchar(ch) && ch != '/' && ch != '-') {
				if (lastTagWasScript) {
					scriptLanguage = segIsScriptingIndicator(styler, styler.GetStartSegment(), i - 1, scriptLanguage);
				}
				classifyAttribHTML(styler.GetStartSegment(), i - 1, keywords, styler);
				if (ch == '>') {
					styler.ColourTo(i, SCE_H_TAG);
					if (lastTagWasScript) {
						state = StateForScript(scriptLanguage, inScriptTag);
					} else {
						state = SCE_H_DEFAULT;
					}
				} else {
					state = SCE_H_OTHER;
				}
			}
			break;
		case SCE_H_OTHER:
			if (ch == '>') {
				styler.ColourTo(i - 1, state);
				styler.ColourTo(i, SCE_H_TAG);
				if (lastTagWasScript) {
					state = StateForScript(scriptLanguage, inScriptTag);
				} else {
					state = SCE_H_DEFAULT;
				}
			} else if (ch == '\"') {
				styler.ColourTo(i - 1, state);
				state = SCE_H_DOUBLESTRING;
			} else if (ch == '\'') {
				styler.ColourTo(i - 1, state);
				state = SCE_H_SINGLESTRING;
			} else if (ch == '/' && chNext == '>') {
				styler.ColourTo(i - 1, state);
				styler.ColourTo(i + 1, SCE_H_TAGEND);
				i++;
				ch = chNext;
				state = SCE_H_DEFAULT;
			} else if (ch == '?' && chNext == '>') {
				styler.ColourTo(i - 1, state);
				styler.ColourTo(i + 1, SCE_H_XMLEND);
				i++;
				ch = chNext;
				state = SCE_H_DEFAULT;
			} else if (ishtmlwordchar(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_H_ATTRIBUTE;
			}
			break;
		case SCE_H_DOUBLESTRING:
			if (ch == '\"') {
				if (lastTagWasScript) {
					scriptLanguage = segIsScriptingIndicator(styler, styler.GetStartSegment(), i, scriptLanguage);
				}
				styler.ColourTo(i, SCE_H_DOUBLESTRING);
				state = SCE_H_OTHER;
			}
			break;
		case SCE_H_SINGLESTRING:
			if (ch == '\'') {
				if (lastTagWasScript) {
					scriptLanguage = segIsScriptingIndicator(styler, styler.GetStartSegment(), i, scriptLanguage);
				}
				styler.ColourTo(i, SCE_H_SINGLESTRING);
				state = SCE_H_OTHER;
			}
			break;
		case SCE_HJ_DEFAULT:
		case SCE_HJ_START:
		case SCE_HJ_SYMBOLS:
			if (iswordstart(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_HJ_WORD;
			} else if (ch == '/' && chNext == '*') {
				styler.ColourTo(i - 1, state);
				if (chNext2 == '*')
					state = SCE_HJ_COMMENTDOC;
				else
					state = SCE_HJ_COMMENT;
			} else if (ch == '/' && chNext == '/') {
				styler.ColourTo(i - 1, state);
				state = SCE_HJ_COMMENTLINE;
			} else if (ch == '\"') {
				styler.ColourTo(i - 1, state);
				state = SCE_HJ_DOUBLESTRING;
			} else if (ch == '\'') {
				styler.ColourTo(i - 1, state);
				state = SCE_HJ_SINGLESTRING;
			} else if ((ch == '<') && (chNext == '/')) {
				styler.ColourTo(i - 1, state);
				state = SCE_H_TAGUNKNOWN;
				inScriptTag = 0;
			} else if ((ch == '<') && (chNext == '!') && (chNext2 == '-') &&
			           styler.SafeGetCharAt(i + 3) == '-') {
				styler.ColourTo(i - 1, state);
				state = SCE_HJ_COMMENTLINE;
			} else if ((ch == '-') && (chNext == '-') && (chNext2 == '>')) {
				styler.ColourTo(i - 1, state);
				state = SCE_HJ_COMMENTLINE;
				i += 2;
			} else if (isoperator(ch)) {
				styler.ColourTo(i - 1, state);
				styler.ColourTo(i, SCE_HJ_SYMBOLS);
				state = SCE_HJ_DEFAULT;
			} else if ((ch == ' ') || (ch == '\t')) {
				if (state == SCE_HJ_START) {
					styler.ColourTo(i - 1, state);
					state = SCE_HJ_DEFAULT;
				}
			}
			break;
		case SCE_HJ_WORD:
			if (!iswordchar(ch)) {
				classifyWordHTJS(styler.GetStartSegment(), i - 1, keywords2, styler);
				//styler.ColourTo(i - 1, eHTJSKeyword);
				state = SCE_HJ_DEFAULT;
				if (ch == '/' && chNext == '*') {
					if (chNext2 == '*')
						state = SCE_HJ_COMMENTDOC;
					else
						state = SCE_HJ_COMMENT;
				} else if (ch == '/' && chNext == '/') {
					state = SCE_HJ_COMMENTLINE;
				} else if (ch == '\"') {
					state = SCE_HJ_DOUBLESTRING;
				} else if (ch == '\'') {
					state = SCE_HJ_SINGLESTRING;
				} else if ((ch == '-') && (chNext == '-') && (chNext2 == '>')) {
					styler.ColourTo(i - 1, state);
					state = SCE_HJ_COMMENTLINE;
					i += 2;
				} else if (isoperator(ch)) {
					styler.ColourTo(i, SCE_HJ_SYMBOLS);
					state = SCE_HJ_DEFAULT;
				}
			}
			break;
		case SCE_HJ_COMMENT:
		case SCE_HJ_COMMENTDOC:
			if (ch == '/' && chPrev == '*') {
				styler.ColourTo(i, state);
				state = SCE_HJ_DEFAULT;
			} else if ((ch == '<') && (chNext == '/')) {
				styler.ColourTo(i - 1, state);
				styler.ColourTo(i + 1, SCE_H_TAGEND);
				i++;
				ch = chNext;
				state = SCE_H_DEFAULT;
				inScriptTag = 0;
			}
			break;
		case SCE_HJ_COMMENTLINE:
			if (ch == '\r' || ch == '\n') {
				styler.ColourTo(i - 1, SCE_HJ_COMMENTLINE);
				state = SCE_HJ_DEFAULT;
			} else if ((ch == '<') && (chNext == '/')) {
				// Common to hide end script tag in comment
				styler.ColourTo(i - 1, SCE_HJ_COMMENTLINE);
				state = SCE_H_TAGUNKNOWN;
				inScriptTag = 0;
			}
			break;
		case SCE_HJ_DOUBLESTRING:
			if (ch == '\\') {
				if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
					i++;
				}
			} else if (ch == '\"') {
				styler.ColourTo(i, SCE_HJ_DOUBLESTRING);
				state = SCE_HJ_DEFAULT;
				i++;
				ch = chNext;
			} else if (isLineEnd(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_HJ_STRINGEOL;
			}
			break;
		case SCE_HJ_SINGLESTRING:
			if (ch == '\\') {
				if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
					i++;
				}
			} else if (ch == '\'') {
				styler.ColourTo(i, SCE_HJ_SINGLESTRING);
				state = SCE_HJ_DEFAULT;
				i++;
				ch = chNext;
			} else if (isLineEnd(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_HJ_STRINGEOL;
			}
			break;
		case SCE_HJ_STRINGEOL:
			if (!isLineEnd(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_HJ_DEFAULT;
			} else if (!isLineEnd(chNext)) {
				styler.ColourTo(i, state);
				state = SCE_HJ_DEFAULT;
			}
			break;
		case SCE_HJA_DEFAULT:
		case SCE_HJA_START:
		case SCE_HJA_SYMBOLS:
			if (iswordstart(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_HJA_WORD;
			} else if (ch == '/' && chNext == '*') {
				styler.ColourTo(i - 1, state);
				if (chNext2 == '*')
					state = SCE_HJA_COMMENTDOC;
				else
					state = SCE_HJA_COMMENT;
			} else if (ch == '/' && chNext == '/') {
				styler.ColourTo(i - 1, state);
				state = SCE_HJA_COMMENTLINE;
			} else if (ch == '\"') {
				styler.ColourTo(i - 1, state);
				state = SCE_HJA_DOUBLESTRING;
			} else if (ch == '\'') {
				styler.ColourTo(i - 1, state);
				state = SCE_HJA_SINGLESTRING;
			} else if ((ch == '<') && (chNext == '/')) {
				styler.ColourTo(i - 1, state);
				state = SCE_H_TAGUNKNOWN;
				inScriptTag = 0;
			} else if ((ch == '<') && (chNext == '!') && (chNext2 == '-') &&
			           styler.SafeGetCharAt(i + 3) == '-') {
				styler.ColourTo(i - 1, state);
				state = SCE_HJA_COMMENTLINE;
			} else if ((ch == '-') && (chNext == '-') && (chNext2 == '>')) {
				styler.ColourTo(i - 1, state);
				state = SCE_HJA_COMMENTLINE;
				i += 2;
			} else if (isoperator(ch)) {
				styler.ColourTo(i - 1, state);
				styler.ColourTo(i, SCE_HJA_SYMBOLS);
				state = SCE_HJA_DEFAULT;
			} else if ((ch == ' ') || (ch == '\t')) {
				if (state == SCE_HJA_START) {
					styler.ColourTo(i - 1, state);
					state = SCE_HJA_DEFAULT;
				}
			}
			break;
		case SCE_HJA_WORD:
			if (!iswordchar(ch)) {
				classifyWordHTJSA(styler.GetStartSegment(), i - 1, keywords2, styler);
				//styler.ColourTo(i - 1, eHTJSKeyword);
				state = SCE_HJA_DEFAULT;
				if (ch == '/' && chNext == '*') {
					if (chNext2 == '*')
						state = SCE_HJA_COMMENTDOC;
					else
						state = SCE_HJA_COMMENT;
				} else if (ch == '/' && chNext == '/') {
					state = SCE_HJA_COMMENTLINE;
				} else if (ch == '\"') {
					state = SCE_HJA_DOUBLESTRING;
				} else if (ch == '\'') {
					state = SCE_HJA_SINGLESTRING;
				} else if ((ch == '-') && (chNext == '-') && (chNext2 == '>')) {
					styler.ColourTo(i - 1, state);
					state = SCE_HJA_COMMENTLINE;
					i += 2;
				} else if (isoperator(ch)) {
					styler.ColourTo(i, SCE_HJA_SYMBOLS);
					state = SCE_HJA_DEFAULT;
				}
			}
			break;
		case SCE_HJA_COMMENT:
		case SCE_HJA_COMMENTDOC:
			if (ch == '/' && chPrev == '*') {
				styler.ColourTo(i, state);
				state = SCE_HJA_DEFAULT;
			} else if ((ch == '<') && (chNext == '/')) {
				styler.ColourTo(i - 1, state);
				styler.ColourTo(i + 1, SCE_H_TAGEND);
				i++;
				ch = chNext;
				state = SCE_H_DEFAULT;
				inScriptTag = 0;
			}
			break;
		case SCE_HJA_COMMENTLINE:
			if (ch == '\r' || ch == '\n') {
				styler.ColourTo(i - 1, SCE_HJA_COMMENTLINE);
				state = SCE_HJA_DEFAULT;
			} else if ((ch == '<') && (chNext == '/')) {
				// Common to hide end script tag in comment
				styler.ColourTo(i - 1, SCE_HJA_COMMENTLINE);
				state = SCE_H_TAGUNKNOWN;
				inScriptTag = 0;
			}
			break;
		case SCE_HJA_DOUBLESTRING:
			if (ch == '\\') {
				if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
					i++;
				}
			} else if (ch == '\"') {
				styler.ColourTo(i, SCE_HJA_DOUBLESTRING);
				state = SCE_HJA_DEFAULT;
				i++;
				ch = chNext;
			} else if ((ch == '-') && (chNext == '-') && (chNext2 == '>')) {
				styler.ColourTo(i - 1, state);
				state = SCE_HJA_COMMENTLINE;
				i += 2;
			} else if (isLineEnd(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_HJA_STRINGEOL;
			}
			break;
		case SCE_HJA_SINGLESTRING:
			if (ch == '\\') {
				if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
					i++;
				}
			} else if (ch == '\'') {
				styler.ColourTo(i, SCE_HJA_SINGLESTRING);
				state = SCE_HJA_DEFAULT;
				i++;
				ch = chNext;
			} else if ((ch == '-') && (chNext == '-') && (chNext2 == '>')) {
				styler.ColourTo(i - 1, state);
				state = SCE_HJA_COMMENTLINE;
				i += 2;
			} else if (isLineEnd(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_HJA_STRINGEOL;
			}
			break;
		case SCE_HJA_STRINGEOL:
			if (!isLineEnd(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_HJA_DEFAULT;
			} else if (!isLineEnd(chNext)) {
				styler.ColourTo(i, state);
				state = SCE_HJA_DEFAULT;
			}
			break;
		case SCE_HB_DEFAULT:
		case SCE_HB_START:
			if (iswordstart(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_HB_WORD;
			} else if (ch == '\'') {
				styler.ColourTo(i - 1, state);
				state = SCE_HB_COMMENTLINE;
			} else if (ch == '\"') {
				styler.ColourTo(i - 1, state);
				state = SCE_HB_STRING;
			} else if ((ch == '<') && (chNext == '/')) {
				styler.ColourTo(i - 1, state);
				state = SCE_H_TAGUNKNOWN;
				inScriptTag = 0;
			} else if ((ch == '<') && (chNext == '!') && (chNext2 == '-') &&
			           styler.SafeGetCharAt(i + 3) == '-') {
				styler.ColourTo(i - 1, state);
				state = SCE_HB_COMMENTLINE;
			} else if (isoperator(ch)) {
				styler.ColourTo(i - 1, state);
				styler.ColourTo(i, SCE_HB_DEFAULT);
				state = SCE_HB_DEFAULT;
			} else if ((ch == ' ') || (ch == '\t')) {
				if (state == SCE_HB_START) {
					styler.ColourTo(i - 1, state);
					state = SCE_HB_DEFAULT;
				}
			}
			break;
		case SCE_HB_WORD:
			if (!iswordchar(ch)) {
				state = classifyWordHTVB(styler.GetStartSegment(), i - 1, keywords3, styler);
				if (state == SCE_HB_DEFAULT) {
					if (ch == '\"') {
						state = SCE_HB_STRING;
					} else if (ch == '\'') {
						state = SCE_HB_COMMENTLINE;
					} else if (isoperator(ch)) {
						styler.ColourTo(i, SCE_HB_DEFAULT);
						state = SCE_HB_DEFAULT;
					}
				}
			}
			break;
		case SCE_HB_STRING:
			if (ch == '\"') {
				styler.ColourTo(i, state);
				state = SCE_HB_DEFAULT;
				i++;
				ch = chNext;
			} else if (ch == '\r' || ch == '\n') {
				styler.ColourTo(i - 1, state);
				state = SCE_HB_STRINGEOL;
			}
			break;
		case SCE_HB_COMMENTLINE:
			if (ch == '\r' || ch == '\n') {
				styler.ColourTo(i - 1, state);
				state = SCE_HB_DEFAULT;
			} else if ((ch == '<') && (chNext == '/')) {
				// Common to hide end script tag in comment
				styler.ColourTo(i - 1, state);
				state = SCE_H_TAGUNKNOWN;
				inScriptTag = 0;
			}
			break;
		case SCE_HB_STRINGEOL:
			if (!isLineEnd(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_HB_DEFAULT;
			} else if (!isLineEnd(chNext)) {
				styler.ColourTo(i, state);
				state = SCE_HB_DEFAULT;
			}
			break;
		case SCE_HBA_DEFAULT:
		case SCE_HBA_START:
			if (iswordstart(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_HBA_WORD;
			} else if (ch == '\'') {
				styler.ColourTo(i - 1, state);
				state = SCE_HBA_COMMENTLINE;
			} else if (ch == '\"') {
				styler.ColourTo(i - 1, state);
				state = SCE_HBA_STRING;
			} else if ((ch == '<') && (chNext == '/')) {
				styler.ColourTo(i - 1, state);
				state = SCE_H_TAGUNKNOWN;
				inScriptTag = 0;
			} else if ((ch == '<') && (chNext == '!') && (chNext2 == '-') &&
			           styler.SafeGetCharAt(i + 3) == '-') {
				styler.ColourTo(i - 1, state);
				state = SCE_HBA_COMMENTLINE;
			} else if (isoperator(ch)) {
				styler.ColourTo(i - 1, state);
				styler.ColourTo(i, SCE_HBA_DEFAULT);
				state = SCE_HBA_DEFAULT;
			} else if ((ch == ' ') || (ch == '\t')) {
				if (state == SCE_HBA_START) {
					styler.ColourTo(i - 1, state);
					state = SCE_HBA_DEFAULT;
				}
			}
			break;
		case SCE_HBA_WORD:
			if (!iswordchar(ch)) {
				state = classifyWordHTVBA(styler.GetStartSegment(), i - 1, keywords3, styler);
				if (state == SCE_HBA_DEFAULT) {
					if (ch == '\"') {
						state = SCE_HBA_STRING;
					} else if (ch == '\'') {
						state = SCE_HBA_COMMENTLINE;
					} else if (isoperator(ch)) {
						styler.ColourTo(i, SCE_HBA_DEFAULT);
						state = SCE_HBA_DEFAULT;
					}
				}
			}
			break;
		case SCE_HBA_STRING:
			if (ch == '\"') {
				styler.ColourTo(i, state);
				state = SCE_HBA_DEFAULT;
				i++;
				ch = chNext;
			} else if (ch == '\r' || ch == '\n') {
				styler.ColourTo(i - 1, state);
				state = SCE_HBA_STRINGEOL;
			}
			break;
		case SCE_HBA_COMMENTLINE:
			if (ch == '\r' || ch == '\n') {
				styler.ColourTo(i - 1, state);
				state = SCE_HBA_DEFAULT;
			} else if ((ch == '<') && (chNext == '/')) {
				// Common to hide end script tag in comment
				styler.ColourTo(i - 1, state);
				state = SCE_H_TAGUNKNOWN;
				inScriptTag = 0;
			}
			break;
		case SCE_HBA_STRINGEOL:
			if (!isLineEnd(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_HBA_DEFAULT;
			} else if (!isLineEnd(chNext)) {
				styler.ColourTo(i, state);
				state = SCE_HBA_DEFAULT;
			}
			break;
		case SCE_HP_DEFAULT:
		case SCE_HP_START:
			if (iswordstart(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_HP_WORD;
			} else if ((ch == '<') && (chNext == '/')) {
				styler.ColourTo(i - 1, state);
				state = SCE_H_TAGUNKNOWN;
				inScriptTag = 0;
			} else if ((ch == '<') && (chNext == '!') && (chNext2 == '-') &&
			           styler.SafeGetCharAt(i + 3) == '-') {
				styler.ColourTo(i - 1, state);
				state = SCE_HP_COMMENTLINE;
			} else if (ch == '#') {
				styler.ColourTo(i - 1, state);
				state = SCE_HP_COMMENTLINE;
			} else if (ch == '\"') {
				styler.ColourTo(i - 1, state);
				if (chNext == '\"' && chNext2 == '\"') {
					i += 2;
					state = SCE_HP_TRIPLEDOUBLE;
					ch = ' ';
					chPrev = ' ';
					chNext = styler.SafeGetCharAt(i + 1);
				} else {
					state = SCE_HP_STRING;
				}
			} else if (ch == '\'') {
				styler.ColourTo(i - 1, state);
				if (chNext == '\'' && chNext2 == '\'') {
					i += 2;
					state = SCE_HP_TRIPLE;
					ch = ' ';
					chPrev = ' ';
					chNext = styler.SafeGetCharAt(i + 1);
				} else {
					state = SCE_HP_CHARACTER;
				}
			} else if (isoperator(ch)) {
				styler.ColourTo(i - 1, state);
				styler.ColourTo(i, SCE_HP_OPERATOR);
			} else if ((ch == ' ') || (ch == '\t')) {
				if (state == SCE_HP_START) {
					styler.ColourTo(i - 1, state);
					state = SCE_HP_DEFAULT;
				}
			}
			break;
		case SCE_HP_WORD:
			if (!iswordchar(ch)) {
				classifyWordHTPy(styler.GetStartSegment(), i - 1, keywords4, styler, prevWord);
				state = SCE_HP_DEFAULT;
				if (ch == '#') {
					state = SCE_HP_COMMENTLINE;
				} else if (ch == '\"') {
					if (chNext == '\"' && chNext2 == '\"') {
						i += 2;
						state = SCE_HP_TRIPLEDOUBLE;
						ch = ' ';
						chPrev = ' ';
						chNext = styler.SafeGetCharAt(i + 1);
					} else {
						state = SCE_HP_STRING;
					}
				} else if (ch == '\'') {
					if (chNext == '\'' && chNext2 == '\'') {
						i += 2;
						state = SCE_HP_TRIPLE;
						ch = ' ';
						chPrev = ' ';
						chNext = styler.SafeGetCharAt(i + 1);
					} else {
						state = SCE_HP_CHARACTER;
					}
				} else if (isoperator(ch)) {
					styler.ColourTo(i, SCE_HP_OPERATOR);
				}
			}
			break;
		case SCE_HP_COMMENTLINE:
			if (ch == '\r' || ch == '\n') {
				styler.ColourTo(i - 1, state);
				state = SCE_HP_DEFAULT;
			}
			break;
		case SCE_HP_STRING:
			if (ch == '\\') {
				if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
					i++;
					ch = chNext;
					chNext = styler.SafeGetCharAt(i + 1);
				}
			} else if (ch == '\"') {
				styler.ColourTo(i, state);
				state = SCE_HP_DEFAULT;
			}
			break;
		case SCE_HP_CHARACTER:
			if (ch == '\\') {
				if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
					i++;
					ch = chNext;
					chNext = styler.SafeGetCharAt(i + 1);
				}
			} else if (ch == '\'') {
				styler.ColourTo(i, state);
				state = SCE_HP_DEFAULT;
			}
			break;
		case SCE_HP_TRIPLE:
			if (ch == '\'' && chPrev == '\'' && chPrev2 == '\'') {
				styler.ColourTo(i, state);
				state = SCE_HP_DEFAULT;
			}
			break;
		case SCE_HP_TRIPLEDOUBLE:
			if (ch == '\"' && chPrev == '\"' && chPrev2 == '\"') {
				styler.ColourTo(i, state);
				state = SCE_HP_DEFAULT;
			}
			break;
		case SCE_HPA_DEFAULT:
		case SCE_HPA_START:
			if (iswordstart(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_HPA_WORD;
			} else if ((ch == '<') && (chNext == '/')) {
				styler.ColourTo(i - 1, state);
				state = SCE_H_TAGUNKNOWN;
				inScriptTag = 0;
			} else if ((ch == '<') && (chNext == '!') && (chNext2 == '-') &&
			           styler.SafeGetCharAt(i + 3) == '-') {
				styler.ColourTo(i - 1, state);
				state = SCE_HPA_COMMENTLINE;
			} else if (ch == '#') {
				styler.ColourTo(i - 1, state);
				state = SCE_HPA_COMMENTLINE;
			} else if (ch == '\"') {
				styler.ColourTo(i - 1, state);
				if (chNext == '\"' && chNext2 == '\"') {
					i += 2;
					state = SCE_HPA_TRIPLEDOUBLE;
					ch = ' ';
					chPrev = ' ';
					chNext = styler.SafeGetCharAt(i + 1);
				} else {
					state = SCE_HPA_STRING;
				}
			} else if (ch == '\'') {
				styler.ColourTo(i - 1, state);
				if (chNext == '\'' && chNext2 == '\'') {
					i += 2;
					state = SCE_HPA_TRIPLE;
					ch = ' ';
					chPrev = ' ';
					chNext = styler.SafeGetCharAt(i + 1);
				} else {
					state = SCE_HPA_CHARACTER;
				}
			} else if (isoperator(ch)) {
				styler.ColourTo(i - 1, state);
				styler.ColourTo(i, SCE_HPA_OPERATOR);
			} else if ((ch == ' ') || (ch == '\t')) {
				if (state == SCE_HPA_START) {
					styler.ColourTo(i - 1, state);
					state = SCE_HPA_DEFAULT;
				}
			}
			break;
		case SCE_HPA_WORD:
			if (!iswordchar(ch)) {
				classifyWordHTPyA(styler.GetStartSegment(), i - 1, keywords4, styler, prevWord);
				state = SCE_HPA_DEFAULT;
				if (ch == '#') {
					state = SCE_HPA_COMMENTLINE;
				} else if (ch == '\"') {
					if (chNext == '\"' && chNext2 == '\"') {
						i += 2;
						state = SCE_HPA_TRIPLEDOUBLE;
						ch = ' ';
						chPrev = ' ';
						chNext = styler.SafeGetCharAt(i + 1);
					} else {
						state = SCE_HPA_STRING;
					}
				} else if (ch == '\'') {
					if (chNext == '\'' && chNext2 == '\'') {
						i += 2;
						state = SCE_HPA_TRIPLE;
						ch = ' ';
						chPrev = ' ';
						chNext = styler.SafeGetCharAt(i + 1);
					} else {
						state = SCE_HPA_CHARACTER;
					}
				} else if (isoperator(ch)) {
					styler.ColourTo(i, SCE_HPA_OPERATOR);
				}
			}
			break;
		case SCE_HPA_COMMENTLINE:
			if (ch == '\r' || ch == '\n') {
				styler.ColourTo(i - 1, state);
				state = SCE_HPA_DEFAULT;
			}
			break;
		case SCE_HPA_STRING:
			if (ch == '\\') {
				if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
					i++;
					ch = chNext;
					chNext = styler.SafeGetCharAt(i + 1);
				}
			} else if (ch == '\"') {
				styler.ColourTo(i, state);
				state = SCE_HPA_DEFAULT;
			}
			break;
		case SCE_HPA_CHARACTER:
			if (ch == '\\') {
				if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
					i++;
					ch = chNext;
					chNext = styler.SafeGetCharAt(i + 1);
				}
			} else if (ch == '\'') {
				styler.ColourTo(i, state);
				state = SCE_HPA_DEFAULT;
			}
			break;
		case SCE_HPA_TRIPLE:
			if (ch == '\'' && chPrev == '\'' && chPrev2 == '\'') {
				styler.ColourTo(i, state);
				state = SCE_HPA_DEFAULT;
			}
			break;
		case SCE_HPA_TRIPLEDOUBLE:
			if (ch == '\"' && chPrev == '\"' && chPrev2 == '\"') {
				styler.ColourTo(i, state);
				state = SCE_HPA_DEFAULT;
			}
			break;
			///////////// start - PHP state handling
		case SCE_HPHP_WORD:
			if (!iswordchar(ch)) {
				if (ch == '/' && chNext == '*') {
					state = SCE_HPHP_COMMENT;
				} else if (ch == '/' && chNext == '/') {
					state = SCE_HPHP_COMMENTLINE;
				} else if (ch == '\"') {
					state = SCE_HPHP_HSTRING;
				} else if (ch == '\'') {
					state = SCE_HPHP_SIMPLESTRING;
				} else if (ch == '$') {
					state = SCE_HPHP_VARIABLE;
				} else if (isoperator(ch)) {
					state = SCE_HPHP_DEFAULT;
				} else {
					state = SCE_HPHP_DEFAULT;
				}
				classifyWordHTPHPA(styler.GetStartSegment(), i - 1, keywords5, styler);
			} else if ((ch == '<') && (chNext == '/')) {
				styler.ColourTo(i - 1, state);
				state = SCE_H_TAGUNKNOWN;
				inScriptTag = 0;
			}
			break;
		case SCE_HPHP_NUMBER:
			if (!isdigit(ch)) {
				styler.ColourTo(i - 1, SCE_HPHP_NUMBER);
				state = SCE_HPHP_DEFAULT;
			}
			break;
		case SCE_HPHP_VARIABLE:
			if (!iswordchar(ch)) {
				styler.ColourTo(i - 1, SCE_HPHP_VARIABLE);
				state = SCE_HPHP_DEFAULT;
			}
			break;
		case SCE_HPHP_COMMENT:
			if (ch == '/' && chPrev == '*') {
				styler.ColourTo(i, state);
				state = SCE_HPHP_DEFAULT;
			}
			break;
		case SCE_HPHP_COMMENTLINE:
			if (ch == '\r' || ch == '\n') {
				styler.ColourTo(i - 1, state);
				state = SCE_HPHP_DEFAULT;
			}
			break;
		case SCE_HPHP_HSTRING:
			if (ch == '\\') {
				if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
					i++;
					ch = chNext;
					chNext = styler.SafeGetCharAt(i + 1);
				}
			} else if (ch == '\"') {
				styler.ColourTo(i, state);
				state = SCE_HPHP_DEFAULT;
			} else if (chNext == '\r' || chNext == '\n') {
				styler.ColourTo(i - 1, SCE_HPHP_STRINGEOL);
				state = SCE_HPHP_STRINGEOL;
			}
			break;
		case SCE_HPHP_SIMPLESTRING:
			if ((ch == '\r' || ch == '\n') && (chPrev != '\\')) {
				styler.ColourTo(i - 1, SCE_HPHP_STRINGEOL);
				state = SCE_HPHP_STRINGEOL;
			} else if (ch == '\\') {
				if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
					i++;
					ch = chNext;
					chNext = styler.SafeGetCharAt(i + 1);
				}
			} else if (ch == '\'') {
				styler.ColourTo(i, state);
				state = SCE_HPHP_DEFAULT;
			}
			break;
		case SCE_HPHP_STRINGEOL:
			break;
		case SCE_HPHP_DEFAULT:
			styler.ColourTo(i - 1, state);
			if (isdigit(ch)) {
				state = SCE_HPHP_NUMBER;
			} else if (iswordstart(ch)) {
				state = SCE_HPHP_WORD;
			} else if (ch == '/' && chNext == '*') {
				state = SCE_HPHP_COMMENT;
			} else if (ch == '/' && chNext == '/') {
				state = SCE_HPHP_COMMENTLINE;
			} else if (ch == '\"') {
				state = SCE_HPHP_HSTRING;
			} else if (ch == '\'') {
				state = SCE_HPHP_SIMPLESTRING;
			} else if (ch == '$') {
				state = SCE_HPHP_VARIABLE;
			}
			break;
			///////////// end - PHP state handling
		}


		if (state == SCE_HB_DEFAULT) {    // One of the above succeeded
			if (ch == '\"') {
				state = SCE_HB_STRING;
			} else if (ch == '\'') {
				state = SCE_HB_COMMENTLINE;
			} else if (iswordstart(ch)) {
				state = SCE_HB_WORD;
			} else if (isoperator(ch)) {
				styler.ColourTo(i, SCE_HB_DEFAULT);
			}
		}
		if (state == SCE_HBA_DEFAULT) {    // One of the above succeeded
			if (ch == '\"') {
				state = SCE_HBA_STRING;
			} else if (ch == '\'') {
				state = SCE_HBA_COMMENTLINE;
			} else if (iswordstart(ch)) {
				state = SCE_HBA_WORD;
			} else if (isoperator(ch)) {
				styler.ColourTo(i, SCE_HBA_DEFAULT);
			}
		}
		if (state == SCE_HJ_DEFAULT) {    // One of the above succeeded
			if (ch == '/' && chNext == '*') {
				if (styler.SafeGetCharAt(i + 2) == '*')
					state = SCE_HJ_COMMENTDOC;
				else
					state = SCE_HJ_COMMENT;
			} else if (ch == '/' && chNext == '/') {
				state = SCE_HJ_COMMENTLINE;
			} else if (ch == '\"') {
				state = SCE_HJ_DOUBLESTRING;
			} else if (ch == '\'') {
				state = SCE_HJ_SINGLESTRING;
			} else if (iswordstart(ch)) {
				state = SCE_HJ_WORD;
			} else if (isoperator(ch)) {
				styler.ColourTo(i, SCE_HJ_SYMBOLS);
			}
		}
		if (state == SCE_HJA_DEFAULT) {    // One of the above succeeded
			if (ch == '/' && chNext == '*') {
				if (styler.SafeGetCharAt(i + 2) == '*')
					state = SCE_HJA_COMMENTDOC;
				else
					state = SCE_HJA_COMMENT;
			} else if (ch == '/' && chNext == '/') {
				state = SCE_HJA_COMMENTLINE;
			} else if (ch == '\"') {
				state = SCE_HJA_DOUBLESTRING;
			} else if (ch == '\'') {
				state = SCE_HJA_SINGLESTRING;
			} else if (iswordstart(ch)) {
				state = SCE_HJA_WORD;
			} else if (isoperator(ch)) {
				styler.ColourTo(i, SCE_HJA_SYMBOLS);
			}
		}
		chPrev2 = chPrev;
		chPrev = ch;
	}

	styler.ColourTo(lengthDoc - 1, state);
}

LexerModule lmHTML(SCLEX_HTML, ColouriseHyperTextDoc);
LexerModule lmXML(SCLEX_XML, ColouriseHyperTextDoc);
