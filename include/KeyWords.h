// Scintilla source code edit control
/** @file KeyWords.h
 ** Colourise for particular languages.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

typedef void (*LexerFunction)(unsigned int startPos, int lengthDoc, int initStyle,
                  WordList *keywordlists[], Accessor &styler);
                  
/**
 */
class LexerModule {
	static LexerModule *base;
	LexerModule *next;
	int language;
	LexerFunction fn;

public:
	LexerModule(int language_, LexerFunction fn_);
	static void Colourise(unsigned int startPos, int lengthDoc, int initStyle,
                  int language, WordList *keywordlists[], Accessor &styler);
};

/**
 * Check if a character is a space.
 * This is ASCII specific but is safe with chars >= 0x80.
 */
inline bool isspacechar(unsigned char ch) {
    return (ch == ' ') || ((ch >= 0x09) && (ch <= 0x0d));
}

inline bool iswordchar(char ch) {
	return isascii(ch) && (isalnum(ch) || ch == '.' || ch == '_');
}

inline bool iswordstart(char ch) {
	return isascii(ch) && (isalnum(ch) || ch == '_');
}

inline bool isoperator(char ch) {
	if (isascii(ch) && isalnum(ch))
		return false;
	// '.' left out as it is used to make up numbers
	if (ch == '%' || ch == '^' || ch == '&' || ch == '*' ||
	        ch == '(' || ch == ')' || ch == '-' || ch == '+' ||
	        ch == '=' || ch == '|' || ch == '{' || ch == '}' ||
	        ch == '[' || ch == ']' || ch == ':' || ch == ';' ||
	        ch == '<' || ch == '>' || ch == ',' || ch == '/' ||
	        ch == '?' || ch == '!' || ch == '.' || ch == '~')
		return true;
	return false;
}
