// Scintilla source code edit control
/** @file LexMSSQL.cxx
 ** Lexer for MSSQL.
 **/
// Copyright 1998-2002 by Filip Yaghob <fy@eg.cz>


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

#define KW_MSSQL_STATEMENTS         0
#define KW_MSSQL_DATA_TYPES         1
#define KW_MSSQL_SYSTEM_TABLES      2
#define KW_MSSQL_GLOBAL_VARIABLES   3
#define KW_MSSQL_FUNCTIONS          4

//~ val SCE_MSSQL_DEFAULT=0
//~ val SCE_MSSQL_COMMENT=1
//~ val SCE_MSSQL_LINE_COMMENT=2
//~ val SCE_MSSQL_NUMBER=3
//~ val SCE_MSSQL_STRING=4
//~ val SCE_MSSQL_OPERATOR=5
//~ val SCE_MSSQL_IDENTIFIER=6
//~ val SCE_MSSQL_VARIABLE=7
//~ val SCE_MSSQL_COLUMN_NAME=8
//~ val SCE_MSSQL_STATEMENT=9
//~ val SCE_MSSQL_DATATYPE=10
//~ val SCE_MSSQL_SYSTABLE=11
//~ val SCE_MSSQL_GLOBAL_VARIABLE=12
//~ val SCE_MSSQL_FUNCTION=13

static bool isMSSQLOperator(char ch) {
	if (isascii(ch) && isalnum(ch))
		return false;
	// '.' left out as it is used to make up numbers
	if (ch == '%' || ch == '^' || ch == '&' || ch == '*' ||
        ch == '-' || ch == '+' || ch == '=' || ch == '|' ||
        ch == '<' || ch == '>' || ch == '/' ||
        ch == '!' || ch == '~')
		return true;
	return false;
}

static void classifyWordSQL(unsigned int start,
                            unsigned int end,
                            WordList *keywordlists[],
                            Accessor &styler,
                            unsigned int actualState) {
	char s[100];
	bool wordIsNumber = isdigit(styler[start]) || (styler[start] == '.');

	WordList &kwStatements          = *keywordlists[KW_MSSQL_STATEMENTS];
    WordList &kwDataTypes           = *keywordlists[KW_MSSQL_DATA_TYPES];
    WordList &kwSystemTables        = *keywordlists[KW_MSSQL_SYSTEM_TABLES];
    WordList &kwGlobalVariables     = *keywordlists[KW_MSSQL_GLOBAL_VARIABLES];
    WordList &kwFunctions           = *keywordlists[KW_MSSQL_FUNCTIONS];

    if (actualState == SCE_MSSQL_GLOBAL_VARIABLE)
        start += 2;

	for (unsigned int i = 0; i < end - start + 1 && i < 30; i++) {
		s[i] = static_cast<char>(tolower(styler[start + i]));
		s[i + 1] = '\0';
	}
	char chAttr = SCE_MSSQL_IDENTIFIER;
	if (wordIsNumber)
		chAttr = SCE_MSSQL_NUMBER;
	else {
		if (kwStatements.InList(s))
			chAttr = SCE_MSSQL_STATEMENT;
        if (kwDataTypes.InList(s))
            chAttr = SCE_MSSQL_DATATYPE;
        if (kwSystemTables.InList(s))
            chAttr = SCE_MSSQL_SYSTABLE;
        if (kwGlobalVariables.InList(s))
            chAttr = SCE_MSSQL_GLOBAL_VARIABLE;
        if (kwFunctions.InList(s))
            chAttr = SCE_MSSQL_FUNCTION;
	}
	styler.ColourTo(end, chAttr);
}

static void ColouriseMSSQLDoc(unsigned int startPos, int length,
                              int initStyle, WordList *keywordlists[], Accessor &styler) {


	styler.StartAt(startPos);

	bool fold = styler.GetPropertyInt("fold") != 0;
	int lineCurrent = styler.GetLine(startPos);
	int spaceFlags = 0;
/*
	WordList &kwStatements          = *keywordlists[KW_MSSQL_STATEMENTS];
    WordList &kwDataTypes           = *keywordlists[KW_MSSQL_DATA_TYPES];
    WordList &kwSystemTables        = *keywordlists[KW_MSSQL_SYSTEM_TABLES];
    WordList &kwGlobalVariables     = *keywordlists[KW_MSSQL_GLOBAL_VARIABLES];
    WordList &kwFunctions           = *keywordlists[KW_MSSQL_FUNCTIONS];

	char s[100];
	int iixx = 0;
	s[0] = 's';	s[1] = 'e'; s[2] = 'l'; s[3] = 'e'; s[4] = 'c'; s[5] = 't'; s[6] = 0;
	if (kwStatements.InList(s))
		iixx = 1;
	s[0] = 's';	s[1] = 'e'; s[2] = 'r'; s[3] = 'v'; s[4] = 'e'; s[5] = 'r'; s[6] = 'n'; s[7] = 'a'; s[8] = 'm'; s[9] = 'e'; s[10] = 0;
	if (kwGlobalVariables.InList(s))
		iixx += 2;
*/
	int state = initStyle;
	char chPrev = ' ';
	char chNext = styler[startPos];
	styler.StartSegment(startPos);
	unsigned int lengthDoc = startPos + length;
	for (unsigned int i = startPos; i < lengthDoc; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);

		if ((ch == '\r' && chNext != '\n') || (ch == '\n')) {
			int indentCurrent = styler.IndentAmount(lineCurrent, &spaceFlags);
			int lev = indentCurrent;
			if (!(indentCurrent & SC_FOLDLEVELWHITEFLAG)) {
				// Only non whitespace lines can be headers
				int indentNext = styler.IndentAmount(lineCurrent + 1, &spaceFlags);
				if (indentCurrent < (indentNext & ~SC_FOLDLEVELWHITEFLAG)) {
					lev |= SC_FOLDLEVELHEADERFLAG;
				}
			}
			if (fold) {
				styler.SetLevel(lineCurrent, lev);
			}
		}

		if (styler.IsLeadByte(ch)) {
			chNext = styler.SafeGetCharAt(i + 2);
			chPrev = ' ';
			i += 1;
			continue;
		}

		if (state == SCE_MSSQL_DEFAULT) {
			if (iswordstart(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_MSSQL_IDENTIFIER;
			} else if (ch == '/' && chNext == '*') {
				styler.ColourTo(i - 1, state);
				state = SCE_MSSQL_COMMENT;
			} else if (ch == '-' && chNext == '-') {
				styler.ColourTo(i - 1, state);
				state = SCE_MSSQL_LINE_COMMENT;
			} else if (ch == '\'') {
				styler.ColourTo(i - 1, state);
				state = SCE_MSSQL_STRING;
			} else if ((ch == '"') || (ch == '[')) {
				styler.ColourTo(i - 1, state);
				state = SCE_MSSQL_COLUMN_NAME;
			} else if (isMSSQLOperator(ch)) {
				styler.ColourTo(i - 1, state);
				styler.ColourTo(i, SCE_MSSQL_OPERATOR);
                //~ style = SCE_MSSQL_DEFAULT;
			} else if (ch == '@') {
                styler.ColourTo(i - 1, state);
                if (chNext == '@') {
                    state = SCE_MSSQL_GLOBAL_VARIABLE;
//                    i += 2;
                } else
                    state = SCE_MSSQL_VARIABLE;
            }
		} else if ( (state == SCE_MSSQL_IDENTIFIER) ||
                    (state == SCE_MSSQL_DATATYPE) ||
                    //~ (state == SCE_MSSQL_COLUMN_NAME) ||
                    (state == SCE_MSSQL_FUNCTION) ||
//                    (state == SCE_MSSQL_GLOBAL_VARIABLE) ||
                    (state == SCE_MSSQL_VARIABLE)) {
			if (!iswordchar(ch)) {
                if ((state == SCE_MSSQL_VARIABLE) || (state == SCE_MSSQL_COLUMN_NAME))
                    styler.ColourTo(i - 1, state);
                else
                    classifyWordSQL(styler.GetStartSegment(), i - 1, keywordlists, styler, state);

				state = SCE_MSSQL_DEFAULT;
				if (ch == '/' && chNext == '*') {
					state = SCE_MSSQL_COMMENT;
				} else if (ch == '-' && chNext == '-') {
					state = SCE_MSSQL_LINE_COMMENT;
				} else if (ch == '\'') {
					state = SCE_MSSQL_STRING;
				} else if ((ch == '"') || (ch == '[')) {
					state = SCE_MSSQL_COLUMN_NAME;
				} else if (isMSSQLOperator(ch)) {
					styler.ColourTo(i, SCE_MSSQL_OPERATOR);
				} else if (ch == '@')
					state = (chNext == '@') ? SCE_MSSQL_GLOBAL_VARIABLE : SCE_MSSQL_VARIABLE;
			}
		} else {
			if (state == SCE_MSSQL_COMMENT) {
				if (ch == '/' && chPrev == '*') {
					if (((i > (styler.GetStartSegment() + 2)) || ((initStyle == SCE_MSSQL_COMMENT) &&
					    (styler.GetStartSegment() == startPos)))) {
						styler.ColourTo(i, state);
						//~ state = SCE_MSSQL_COMMENT;
                        state = SCE_MSSQL_DEFAULT;
					}
				}
			} else if (state == SCE_MSSQL_LINE_COMMENT) {
				if (ch == '\r' || ch == '\n') {
					styler.ColourTo(i - 1, state);
					state = SCE_MSSQL_DEFAULT;
				}
			} else if (state == SCE_MSSQL_STRING) {
				if (ch == '\'') {
					if ( chNext == '\'' ) {
						i++;
					} else {
						styler.ColourTo(i, state);
						state = SCE_MSSQL_DEFAULT;
						i++;
					}
					ch = chNext;
					chNext = styler.SafeGetCharAt(i + 1);
				}
			} else if (state == SCE_MSSQL_COLUMN_NAME) {
				if (ch == '"') {
					if (chNext == '"') {
						i++;
					} else {
						styler.ColourTo(i, state);
						state = SCE_MSSQL_DEFAULT;
						//i++;
					}
					ch = chNext;
					chNext = styler.SafeGetCharAt(i + 1);
				} else if (ch == ']') {
                    styler.ColourTo(i, state);
                    state = SCE_MSSQL_DEFAULT;
                    //i++;
                }
			} else if (state == SCE_MSSQL_GLOBAL_VARIABLE) {
				if ((ch != '@') && !iswordchar(ch)) {
					classifyWordSQL(styler.GetStartSegment(), i - 1, keywordlists, styler, state);
					state = SCE_MSSQL_DEFAULT;
				}
			}
			if (state == SCE_MSSQL_DEFAULT) {    // One of the above succeeded
                if (iswordstart(ch)) {
                    styler.ColourTo(i - 1, state);
                    state = SCE_MSSQL_IDENTIFIER;
                } else if (ch == '/' && chNext == '*') {
                    styler.ColourTo(i - 1, state);
                    state = SCE_MSSQL_COMMENT;
                } else if (ch == '-' && chNext == '-') {
                    styler.ColourTo(i - 1, state);
                    state = SCE_MSSQL_LINE_COMMENT;
                } else if (ch == '\'') {
                    styler.ColourTo(i - 1, state);
                    state = SCE_MSSQL_STRING;
                } else if ((ch == '"') || (ch == '[')) {
                    styler.ColourTo(i - 1, state);
                    state = SCE_MSSQL_COLUMN_NAME;
                } else if (isMSSQLOperator(ch) && !((ch == '/') && (chPrev == '*'))) {
                    styler.ColourTo(i - 1, state);
                    styler.ColourTo(i, SCE_MSSQL_OPERATOR);
                } else if (ch == '@') {
                    styler.ColourTo(i - 1, state);
					state = (chNext == '@') ? SCE_MSSQL_GLOBAL_VARIABLE : SCE_MSSQL_VARIABLE;
                }
			}
		}
		chPrev = ch;
	}
	styler.ColourTo(lengthDoc - 1, state);
}

static const char * const sqlWordListDesc[] = {
	"Statements",
    "Data Types",
    "System tables",
    "Global variables",
    "Functions",
	0,
};

LexerModule lmMSSQL(SCLEX_MSSQL, ColouriseMSSQLDoc, "mssql", 0, sqlWordListDesc);
