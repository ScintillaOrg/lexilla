// Scintilla source code edit control
/** @file LexZig.cxx
 ** Lexer for Zig language.
 **/
// Based on Zufu Liu's Notepad4 Zig lexer
// Modified for Scintilla by Jiri Techet, 2024
// The License.txt file describes the conditions under which this software may be distributed.

#include <cassert>
#include <cstring>

#include <algorithm>
#include <string>
#include <string_view>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"

using namespace Lexilla;

namespace {
// Use an unnamed namespace to protect the functions and classes from name conflicts

constexpr bool IsAGraphic(int ch) noexcept {
	// excludes C0 control characters and whitespace
	return ch > 32 && ch < 127;
}

constexpr bool IsIdentifierStart(int ch) noexcept {
	return IsUpperOrLowerCase(ch) || ch == '_';
}

constexpr bool IsIdentifierStartEx(int ch) noexcept {
	return IsIdentifierStart(ch) || ch >= 0x80;
}

constexpr bool IsNumberStart(int ch, int chNext) noexcept {
	return IsADigit(ch) || (ch == '.' && IsADigit(chNext));
}

constexpr bool IsIdentifierChar(int ch) noexcept {
	return IsAlphaNumeric(ch) || ch == '_';
}

constexpr bool IsNumberContinue(int chPrev, int ch, int chNext) noexcept {
	return ((ch == '+' || ch == '-') && (chPrev == 'e' || chPrev == 'E'))
		|| (ch == '.' && chNext != '.');
}

constexpr bool IsDecimalNumber(int chPrev, int ch, int chNext) noexcept {
	return IsIdentifierChar(ch) || IsNumberContinue(chPrev, ch, chNext);
}

constexpr bool IsIdentifierCharEx(int ch) noexcept {
	return IsIdentifierChar(ch) || ch >= 0x80;
}

// https://ziglang.org/documentation/master/#Escape-Sequences
struct EscapeSequence {
	int outerState = SCE_ZIG_DEFAULT;
	int digitsLeft = 0;
	bool brace = false;

	// highlight any character as escape sequence.
	void resetEscapeState(int state, int chNext) noexcept {
		outerState = state;
		digitsLeft = 1;
		brace = false;
		if (chNext == 'x') {
			digitsLeft = 3;
		} else if (chNext == 'u') {
			digitsLeft = 5;
		}
	}
	void resetEscapeState(int state) noexcept {
		outerState = state;
		digitsLeft = 1;
		brace = false;
	}
	bool atEscapeEnd(int ch) noexcept {
		--digitsLeft;
		return digitsLeft <= 0 || !IsAHeXDigit(ch);
	}
};

enum {
	ZigLineStateMaskLineComment = 1, // line comment
	ZigLineStateMaskMultilineString = 1 << 1, // multiline string
};

enum class KeywordType {
	None = SCE_ZIG_DEFAULT,
	Function = SCE_ZIG_FUNCTION,
};

enum {
	KeywordIndex_Primary = 0,
	KeywordIndex_Secondary = 1,
	KeywordIndex_Tertiary = 2,
	KeywordIndex_Type = 3,
};

const char *const zigWordListDesc[] = {
	"Primary keywords",
	"Secondary keywords",
	"Tertiary keywords",
	"Global type definitions",
	nullptr
};

void ColouriseZigDoc(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, WordList *keywordLists[], Accessor &styler) {
	KeywordType kwType = KeywordType::None;
	int visibleChars = 0;
	int lineState = 0;
	EscapeSequence escSeq;

	StyleContext sc(startPos, lengthDoc, initStyle, styler);

	while (sc.More()) {
		switch (sc.state) {
		case SCE_ZIG_OPERATOR:
			sc.SetState(SCE_ZIG_DEFAULT);
			break;

		case SCE_ZIG_NUMBER:
			if (!IsDecimalNumber(sc.chPrev, sc.ch, sc.chNext)) {
				sc.SetState(SCE_ZIG_DEFAULT);
			}
			break;

		case SCE_ZIG_IDENTIFIER:
		case SCE_ZIG_BUILTIN_FUNCTION:
			if (!IsIdentifierCharEx(sc.ch)) {
				if (sc.state == SCE_ZIG_IDENTIFIER) {
					char s[64];
					sc.GetCurrent(s, sizeof(s));
					if (kwType != KeywordType::None) {
						sc.ChangeState(static_cast<int>(kwType));
					} else if (keywordLists[KeywordIndex_Primary]->InList(s)) {
						sc.ChangeState(SCE_ZIG_KW_PRIMARY);
						kwType = KeywordType::None;
						if (strcmp(s, "fn") == 0) {
							kwType = KeywordType::Function;
						}
					} else if (keywordLists[KeywordIndex_Secondary]->InList(s)) {
						sc.ChangeState(SCE_ZIG_KW_SECONDARY);
					} else if (keywordLists[KeywordIndex_Tertiary]->InList(s)) {
						sc.ChangeState(SCE_ZIG_KW_TERTIARY);
					} else if (keywordLists[KeywordIndex_Type]->InList(s)) {
						sc.ChangeState(SCE_ZIG_KW_TYPE);
					}
				}
				if (sc.state != SCE_ZIG_KW_PRIMARY) {
					kwType = KeywordType::None;
				}
				sc.SetState(SCE_ZIG_DEFAULT);
			}
			break;

		case SCE_ZIG_CHARACTER:
		case SCE_ZIG_STRING:
		case SCE_ZIG_MULTISTRING:
			if (sc.atLineStart) {
				sc.SetState(SCE_ZIG_DEFAULT);
			} else if (sc.ch == '\\' && sc.state != SCE_ZIG_MULTISTRING) {
				escSeq.resetEscapeState(sc.state, sc.chNext);
				sc.SetState(SCE_ZIG_ESCAPECHAR);
				sc.Forward();
				if (sc.Match('u', '{')) {
					escSeq.brace = true;
					escSeq.digitsLeft = 9;
					sc.Forward();
				}
			} else if ((sc.ch == '\'' && sc.state == SCE_ZIG_CHARACTER) || (sc.ch == '\"' && sc.state == SCE_ZIG_STRING)) {
				sc.ForwardSetState(SCE_ZIG_DEFAULT);
			} else if (sc.state != SCE_ZIG_CHARACTER) {
				if (sc.ch == '{' || sc.ch == '}') {
					if (sc.ch == sc.chNext) {
						escSeq.resetEscapeState(sc.state);
						sc.SetState(SCE_ZIG_ESCAPECHAR);
						sc.Forward();
                    }
                }
			}
			break;

		case SCE_ZIG_ESCAPECHAR:
			if (escSeq.atEscapeEnd(sc.ch)) {
				if (escSeq.brace && sc.ch == '}') {
					sc.Forward();
				}
				sc.SetState(escSeq.outerState);
				continue;
			}
			break;

		case SCE_ZIG_COMMENTLINE:
		case SCE_ZIG_COMMENTLINEDOC:
		case SCE_ZIG_COMMENTLINETOP:
			if (sc.atLineStart) {
				sc.SetState(SCE_ZIG_DEFAULT);
			}
			break;
		}

		if (sc.state == SCE_ZIG_DEFAULT) {
			if (sc.Match('/', '/')) {
				if (visibleChars == 0) {
					lineState = ZigLineStateMaskLineComment;
				}
				sc.SetState(SCE_ZIG_COMMENTLINE);
				sc.Forward(2);
				if (sc.ch == '!') {
					sc.ChangeState(SCE_ZIG_COMMENTLINETOP);
				} else if (sc.ch == '/' && sc.chNext != '/') {
					sc.ChangeState(SCE_ZIG_COMMENTLINEDOC);
				}
			} else if (sc.Match('\\', '\\')) {
				lineState = ZigLineStateMaskMultilineString;
				sc.SetState(SCE_ZIG_MULTISTRING);
			} else if (sc.ch == '\"') {
				sc.SetState(SCE_ZIG_STRING);
			} else if (sc.ch == '\'') {
				sc.SetState(SCE_ZIG_CHARACTER);
			} else if (IsNumberStart(sc.ch, sc.chNext)) {
				sc.SetState(SCE_ZIG_NUMBER);
			} else if ((sc.ch == '@' && IsIdentifierStartEx(sc.chNext)) || IsIdentifierStartEx(sc.ch)) {
				sc.SetState((sc.ch == '@') ? SCE_ZIG_BUILTIN_FUNCTION : SCE_ZIG_IDENTIFIER);
			} else if (IsAGraphic(sc.ch)) {
				sc.SetState(SCE_ZIG_OPERATOR);
			}
		}

		if (visibleChars == 0 && !isspacechar(sc.ch)) {
			visibleChars++;
		}
		if (sc.atLineEnd) {
			styler.SetLineState(sc.currentLine, lineState);
			lineState = 0;
			kwType = KeywordType::None;
			visibleChars = 0;
		}
		sc.Forward();
	}

	sc.Complete();
}

struct FoldLineState {
	int lineComment;
	int multilineString;
	constexpr explicit FoldLineState(int lineState) noexcept:
		lineComment(lineState & ZigLineStateMaskLineComment),
		multilineString((lineState >> 1) & 1) {
	}
};

void FoldZigDoc(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, WordList *[] /*keywordLists*/, Accessor &styler) {
	const Sci_PositionU endPos = startPos + lengthDoc;
	Sci_Position lineCurrent = styler.GetLine(startPos);
	FoldLineState foldPrev(0);
	int levelCurrent = SC_FOLDLEVELBASE;
	if (lineCurrent > 0) {
		levelCurrent = styler.LevelAt(lineCurrent - 1) >> 16;
		foldPrev = FoldLineState(styler.GetLineState(lineCurrent - 1));
	}

	int levelNext = levelCurrent;
	FoldLineState foldCurrent(styler.GetLineState(lineCurrent));
	Sci_PositionU lineStartNext = styler.LineStart(lineCurrent + 1);
	lineStartNext = std::min(lineStartNext, endPos);

	while (startPos < endPos) {
		initStyle = styler.StyleIndexAt(startPos);

		if (initStyle == SCE_ZIG_OPERATOR) {
			const char ch = styler[startPos];
			if (ch == '{' || ch == '[' || ch == '(') {
				levelNext++;
			} else if (ch == '}' || ch == ']' || ch == ')') {
				levelNext--;
			}
		}

		++startPos;
		if (startPos == lineStartNext) {
			const FoldLineState foldNext(styler.GetLineState(lineCurrent + 1));
			levelNext = std::max(levelNext, SC_FOLDLEVELBASE);
			if (foldCurrent.lineComment) {
				levelNext += foldNext.lineComment - foldPrev.lineComment;
			} else if (foldCurrent.multilineString) {
				levelNext += foldNext.multilineString - foldPrev.multilineString;
			}

			const int levelUse = levelCurrent;
			int lev = levelUse | (levelNext << 16);
			if (levelUse < levelNext) {
				lev |= SC_FOLDLEVELHEADERFLAG;
			}
			styler.SetLevel(lineCurrent, lev);

			lineCurrent++;
			lineStartNext = styler.LineStart(lineCurrent + 1);
			lineStartNext = std::min(lineStartNext, endPos);
			levelCurrent = levelNext;
			foldPrev = foldCurrent;
			foldCurrent = foldNext;
		}
	}
}

}  // unnamed namespace end

extern const LexerModule lmZig(SCLEX_ZIG, ColouriseZigDoc, "zig", FoldZigDoc, zigWordListDesc);
