// Scintilla source code edit control

// File: LexTeX.cxx - general context conformant tex coloring scheme
// Author: Hans Hagen - PRAGMA ADE - Hasselt NL - www.pragma-ade.com
// Version: August 18, 2003

// Copyright: 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

// This lexer is derived from the one written for the texwork environment (1999++) which in
// turn is inspired on texedit (1991++) which finds its roots in wdt (1986).

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
#include "StyleContext.h"

// val SCE_TEX_DEFAULT = 0
// val SCE_TEX_SPECIAL = 1
// val SCE_TEX_GROUP   = 2
// val SCE_TEX_SYMBOL  = 3
// val SCE_TEX_COMMAND = 4
// val SCE_TEX_TEXT    = 5

// Definitions in SciTEGlobal.properties:
//
// TeX Highlighting
//
// # Default
// style.tex.0=fore:#7F7F00
// # Special
// style.tex.1=fore:#007F7F
// # Group
// style.tex.2=fore:#880000
// # Symbol
// style.tex.3=fore:#7F7F00
// # Command
// style.tex.4=fore:#008800
// # Text
// style.tex.5=fore:#000000

// lexer.tex.interface.default=0
// lexer.tex.comment.process=0

// Auxiliary functions:

static inline bool endOfLine(Accessor &styler, unsigned int i) {
	return
      (styler[i] == '\n') || ((styler[i] == '\r') && (styler.SafeGetCharAt(i + 1) != '\n')) ;
}

static inline bool isTeXzero(char ch) {
	return
      (ch == '%') ;
}

static inline bool isTeXone(char ch) {
	return
      (ch == '[') || (ch == ']') || (ch == '=') || (ch == '#') ||
      (ch == '(') || (ch == ')') || (ch == '<') || (ch == '>') ||
      (ch == '"') ;
}

static inline bool isTeXtwo(char ch) {
	return
      (ch == '{') || (ch == '}') || (ch == '$') ;
}

static inline bool isTeXthree(char ch) {
	return
      (ch == '~') || (ch == '^') || (ch == '_') || (ch == '&') ||
      (ch == '-') || (ch == '+') || (ch == '\"') || (ch == '`') ||
      (ch == '/') || (ch == '|') || (ch == '%') ;
}

static inline bool isTeXfour(char ch) {
	return
      (ch == '\\') ;
}

static inline bool isTeXfive(char ch) {
	return
      ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')) ||
      (ch == '@') || (ch == '!') || (ch == '?') ;
}

static inline bool isTeXsix(char ch) {
	return
      (ch == ' ') ;
}


// Coloring functions:

bool newifDone = false ;

static void ColourTeXRange(
    unsigned int texMode,
    char *key,
    unsigned int endPos,
    WordList &keywords,
    bool useKeywords,
    Accessor &styler) {

    bool autoIf = true ;

    switch (texMode) {
        case 0 :
            styler.ColourTo(endPos, SCE_TEX_DEFAULT) ;
			newifDone = false ;
            break ;
        case 1 :
            styler.ColourTo(endPos, SCE_TEX_SPECIAL) ;
			newifDone = false ;
            break ;
        case 2 :
            styler.ColourTo(endPos, SCE_TEX_GROUP) ;
			newifDone = false ;
            break ;
        case 3 :
            styler.ColourTo(endPos, SCE_TEX_SYMBOL) ;
			newifDone = false ;
            break ;
        case 4 :
            if (! keywords || ! useKeywords) {
                styler.ColourTo(endPos, SCE_TEX_COMMAND) ;
				newifDone = false ;
            } else if (key[1] == '\0') {
                styler.ColourTo(endPos, SCE_TEX_COMMAND) ;
				newifDone = false ;
            } else if (keywords.InList(key)) {
                styler.ColourTo(endPos, SCE_TEX_COMMAND) ;
				newifDone = autoIf && (strcmp(key,"newif") == 0) ;
            } else if (autoIf && ! newifDone && (key[0] == 'i') && (key[1] == 'f') && keywords.InList("if")) {
                styler.ColourTo(endPos, SCE_TEX_COMMAND) ;
			} else {
                styler.ColourTo(endPos, SCE_TEX_TEXT) ;
				newifDone = false ;
            }
            break ;
        case 5 :
            styler.ColourTo(endPos, SCE_TEX_TEXT) ;
			newifDone = newifDone || (strspn(key," ") == strlen(key)) ;
            break ;
    }

}

static void ColouriseTeXLine(
    char *lineBuffer,
    unsigned int lengthLine,
    unsigned int startPos,
    WordList &keywords,
    bool useKeywords,
    Accessor &styler) {

    char ch;
    bool cs = false ;
    unsigned int offset = 0 ;
    unsigned int mode = 5 ;
    unsigned int k = 0 ;
    char key[1024] ; // length check in calling routine
    unsigned int start = startPos-1 ;

    bool comment = (styler.GetPropertyInt("lexer.tex.comment.process", 0) == 0) ;

    // we use a cheap append to key method, ugly, but fast and ok

    while (offset < lengthLine) {

        ch = lineBuffer[offset] ;

        if (cs) {
			cs = false ;
			key[k] = ch ; ++k ; key[k] = '\0' ; // ugly but ok
        } else if ((comment) && ((mode == 0) || (isTeXzero(ch)))) {
            ColourTeXRange(mode,key,start,keywords,useKeywords,styler) ; k = 0 ;
            mode = 0 ;
        } else if (isTeXone(ch)) {
            if (mode != 1) { ColourTeXRange(mode,key,start,keywords,useKeywords,styler) ; k = 0 ; }
            mode = 1 ;
        } else if (isTeXtwo(ch)) {
            if (mode != 2) { ColourTeXRange(mode,key,start,keywords,useKeywords,styler) ; k = 0 ; }
            mode = 2 ;
        } else if (isTeXthree(ch)) {
            if (mode != 3) { ColourTeXRange(mode,key,start,keywords,useKeywords,styler) ; k = 0 ; }
            mode = 3 ;
        } else if (isTeXfour(ch)) {
			if (keywords || (mode != 4)) {
                ColourTeXRange(mode,key,start,keywords,useKeywords,styler) ; k = 0 ; cs = true ;
			}
			mode = 4 ;
        } else if (isTeXfive(ch)) {
            if (mode < 4) {
				ColourTeXRange(mode,key,start,keywords,useKeywords,styler) ; k = 0 ; mode = 5 ;
	            key[k] = ch ; ++k ; key[k] = '\0' ; // ugly but ok
			} else if ((mode == 4) && (k == 1) && isTeXfour(key[0])) {
				ColourTeXRange(mode,key,start,keywords,useKeywords,styler) ; k = 0 ; mode = 5 ;
			} else {
	            key[k] = ch ; ++k ; key[k] = '\0' ; // ugly but ok
			}
        } else if (isTeXsix(ch)) {
            if (mode != 5) { ColourTeXRange(mode,key,start,keywords,useKeywords,styler) ; k = 0 ; }
            mode = 5 ;
        } else if (mode != 5) {
            ColourTeXRange(mode,key,start,keywords,useKeywords,styler) ; k = 0 ; mode = 5 ;
        }

        ++offset ;
        ++start ;

    }

    ColourTeXRange(mode,key,start,keywords,useKeywords,styler) ;

}

static int CheckTeXInterface(
    unsigned int startPos,
    int length,
    Accessor &styler) {

    char lineBuffer[1024] ;
	unsigned int linePos = 0 ;

    int defaultInterface = styler.GetPropertyInt("lexer.tex.interface.default", 1) ;

    // some day we can make something lexer.tex.mapping=(all,0)(nl,1)(en,2)...

    if (styler.SafeGetCharAt(0) == '%') {
        for (unsigned int i = 0; i < startPos + length; i++) {
            lineBuffer[linePos++] = styler[i];
            if (endOfLine(styler, i) || (linePos >= sizeof(lineBuffer) - 1)) {
                lineBuffer[linePos] = '\0';
                if (strstr(lineBuffer, "interface=all")) {
                    return 0 ;
				} else if (strstr(lineBuffer, "interface=tex")) {
                    return 1 ;
                } else if (strstr(lineBuffer, "interface=nl")) {
                    return 2 ;
                } else if (strstr(lineBuffer, "interface=en")) {
                    return 3 ;
                } else if (strstr(lineBuffer, "interface=de")) {
                    return 4 ;
                } else if (strstr(lineBuffer, "interface=cz")) {
                    return 5 ;
                } else if (strstr(lineBuffer, "interface=it")) {
                    return 6 ;
                } else if (strstr(lineBuffer, "interface=ro")) {
                    return 7 ;
                } else if (strstr(lineBuffer, "interface=latex")) {
					// we will move latex cum suis up to 91+ when more keyword lists are supported 
                    return 8 ;
				} else if (styler.SafeGetCharAt(1) == 'D' && strstr(lineBuffer, "%D \\module")) {
					// better would be to limit the search to just one line
					return 3 ;
                } else {
                    return defaultInterface ;
                }
            }
		}
    }

    return defaultInterface ;
}

// Main handler:
//
// The lexer works on a per line basis. I'm not familiar with the internals of scintilla, but
// since the lexer does not look back or forward beyond the current view, some optimization can
// be accomplished by providing just the viewport. The following code is more or less copied
// from the LexOthers.cxx file.

static void ColouriseTeXDoc(
    unsigned int startPos,
    int length,
    int /*initStyle*/,
    WordList *keywordlists[],
    Accessor &styler) {

	styler.StartAt(startPos) ;
	styler.StartSegment(startPos) ;

    int currentInterface = CheckTeXInterface(startPos,length,styler) ;

    bool useKeywords = true ;

    if (currentInterface == 0) {
        useKeywords = false ;
        currentInterface = 1 ;
    }

    WordList &keywords = *keywordlists[currentInterface-1] ;

	char lineBuffer[1024] ;
	unsigned int linePos = 0 ;

    for (unsigned int i = startPos; i < startPos + length; i++) {
		lineBuffer[linePos++] = styler[i] ;
		if (endOfLine(styler, i) || (linePos >= sizeof(lineBuffer) - 1)) {
			// End of line (or of line buffer) met, colourise it
			lineBuffer[linePos] = '\0' ;
			ColouriseTeXLine(lineBuffer, linePos, i-linePos+1, keywords, useKeywords, styler) ;
			linePos = 0 ;
		}
	}

	if (linePos > 0) {
        // Last line does not have ending characters
		ColouriseTeXLine(lineBuffer, linePos, startPos+length-linePos, keywords, useKeywords, styler) ;
	}

}

// Hooks into the system:

static const char * const texWordListDesc[] = {
    "TeX, eTeX, pdfTeX, Omega"
    "ConTeXt Dutch",
    "ConTeXt English",
    "ConTeXt German",
    "ConTeXt Czech",
    "ConTeXt Italian",
    "ConTeXt Romanian",
	0,
} ;

LexerModule lmTeX(SCLEX_TEX, ColouriseTeXDoc, "tex", 0, texWordListDesc);
