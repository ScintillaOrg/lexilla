// SciTE - Scintilla based Text Editor
// KeyWords.cxx - colourise for particular languages
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

LexerModule *LexerModule::base = 0;

LexerModule::LexerModule(int language_, LexerFunction fn_) :
	language(language_), fn(fn_) {
	next = base;
	base = this;
}

void LexerModule::Colourise(unsigned int startPos, int lengthDoc, int initStyle,
		int language, WordList *keywordlists[], Accessor &styler) {
	LexerModule *lm = base;
	while (lm) {
		if (lm->language == language) {
			lm->fn(startPos, lengthDoc, initStyle, keywordlists, styler);
			return;
		}
		lm = lm->next;
	}
	// Unknown language
	// Null language means all style bytes are 0 so just mark the end - no need to fill in.
	if (lengthDoc > 0) {
		styler.StartAt(startPos + lengthDoc - 1);
		styler.StartSegment(startPos + lengthDoc - 1);
		styler.ColourTo(startPos + lengthDoc - 1, 0);
	}
}

#ifdef __vms

// The following code forces a reference to all of the Scintilla lexers.
// If we don't do something like this, then the linker tends to "optimize"
// them away. (eric@sourcegear.com)

// Taken from wxWindow's stc.cpp. Walter.

int wxForceScintillaLexers(void) {
  extern LexerModule lmCPP;
  extern LexerModule lmHTML;
  extern LexerModule lmXML;
  extern LexerModule lmProps;
  extern LexerModule lmErrorList;
  extern LexerModule lmMake;
  extern LexerModule lmBatch;
  extern LexerModule lmPerl;
  extern LexerModule lmPython;
  extern LexerModule lmSQL;
  extern LexerModule lmVB;

  if (
      &lmCPP
      && &lmHTML
      && &lmXML
      && &lmProps
      && &lmErrorList
      && &lmMake
      && &lmBatch
      && &lmPerl
      && &lmPython
      && &lmSQL
      && &lmVB
      )
    {
      return 1;
    }
  else
    {
      return 0;
    }
}
#endif
