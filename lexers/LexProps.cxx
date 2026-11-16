// Scintilla source code edit control
/** @file LexProps.cxx
 ** Lexer for properties files.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <cstdlib>
#include <cassert>

#include <string>
#include <string_view>
#include <map>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "CharacterSet.h"
#include "LexerModule.h"
#include "OptionSet.h"
#include "DefaultLexer.h"

using namespace Scintilla;
using namespace Lexilla;

namespace {

// Options used for LexerProps
struct OptionsProps {
	bool allowInitialSpaces = true;
	bool fold = false;
	bool foldCompact = true;
};

const char *const emptyWordListDesc[] = {
	nullptr
};

struct OptionSetProps : public OptionSet<OptionsProps> {
	OptionSetProps() {
		DefineProperty("lexer.props.allow.initial.spaces", &OptionsProps::allowInitialSpaces,
			"For properties files, set to 0 to style all lines that start with whitespace in the default style. "
			"This is not suitable for SciTE .properties files which use indentation for flow control but "
			"can be used for RFC2822 text where indentation is used for continuation lines."
		);

		DefineProperty("fold", &OptionsProps::fold);
		DefineProperty("fold.compact", &OptionsProps::foldCompact);
	}
};

const LexicalClass lexicalClasses[] = {
	// Lexer props SCLEX_PROPERTIES SCE_PROPS_
	0, "SCE_PROPS_DEFAULT", "default", "Default",
	1, "SCE_PROPS_COMMENT", "comment", "Comment",
	2, "SCE_PROPS_SECTION", "keyword", "Section",
	3, "SCE_PROPS_ASSIGNMENT", "operator", "Assignment operator",
	4, "SCE_PROPS_DEFVAL", "literal string", "Default value (@)",
	5, "SCE_PROPS_KEY", "identifier", "Key",
};

class LexerProps : public DefaultLexer {
	OptionsProps options;
	OptionSetProps osProps;
public:
	LexerProps() :
		DefaultLexer("props", SCLEX_PROPERTIES, lexicalClasses, std::size(lexicalClasses)) {
		SetOptionSet(&osProps);
	}

	Sci_Position SCI_METHOD PropertySet(const char *key, const char *val) override;

	void ColouriseLine(const std::string &lineBuffer, Sci_PositionU startLine, Sci_PositionU endPos, LexAccessor &styler) const;

	void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess) override;
	void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess) override;

	static ILexer5 *LexerFactoryProps() {
		return new LexerProps();
	}
};

constexpr bool isAssignChar(char ch) noexcept {
	return (ch == '=') || (ch == ':');
}

Sci_Position SCI_METHOD LexerProps::PropertySet(const char *key, const char *val) {
	if (osProps.PropertySet(&options, key, val)) {
		return 0;
	}
	return -1;
}

void LexerProps::ColouriseLine(
	const std::string &lineBuffer,
	Sci_PositionU startLine,
	Sci_PositionU endPos,
	LexAccessor &styler) const {

	const Sci_PositionU lengthLine = lineBuffer.length();
	Sci_PositionU i = 0;
	if (options.allowInitialSpaces) {
		while ((i < lengthLine) && isspacechar(lineBuffer[i]))	// Skip initial spaces
			i++;
	} else {
		if (isspacechar(lineBuffer[i])) // don't allow initial spaces
			i = lengthLine;
	}

	if (i < lengthLine) {
		if (lineBuffer[i] == '#' || lineBuffer[i] == '!' || lineBuffer[i] == ';') {
			styler.ColourTo(endPos, SCE_PROPS_COMMENT);
		} else if (lineBuffer[i] == '[') {
			styler.ColourTo(endPos, SCE_PROPS_SECTION);
		} else if (lineBuffer[i] == '@') {
			styler.ColourTo(startLine + i, SCE_PROPS_DEFVAL);
			if (isAssignChar(lineBuffer[i++]))
				styler.ColourTo(startLine + i, SCE_PROPS_ASSIGNMENT);
			styler.ColourTo(endPos, SCE_PROPS_DEFAULT);
		} else {
			// Search for the '=' character
			while ((i < lengthLine) && !isAssignChar(lineBuffer[i]))
				i++;
			if ((i < lengthLine) && isAssignChar(lineBuffer[i])) {
				styler.ColourTo(startLine + i - 1, SCE_PROPS_KEY);
				styler.ColourTo(startLine + i, SCE_PROPS_ASSIGNMENT);
				styler.ColourTo(endPos, SCE_PROPS_DEFAULT);
			} else {
				styler.ColourTo(endPos, SCE_PROPS_DEFAULT);
			}
		}
	} else {
		styler.ColourTo(endPos, SCE_PROPS_DEFAULT);
	}
}

void LexerProps::Lex(Sci_PositionU startPos, Sci_Position length, int /* initStyle */, IDocument *pAccess) {
	LexAccessor styler(pAccess);
	std::string lineBuffer;
	styler.StartAt(startPos);
	styler.StartSegment(startPos);

	const Sci_Position endPos = startPos + length;
	Sci_Position line = styler.GetLine(startPos);
	Sci_Position startOfLine = styler.LineStart(line);
	while (startOfLine < endPos) {
		const Sci_Position endLine = styler.LineStart(line + 1);
		lineBuffer.resize(endLine - startOfLine);
		styler.GetRange(startOfLine, endLine, lineBuffer.data(), lineBuffer.size() + 1);
		ColouriseLine(lineBuffer, startOfLine, endLine - 1, styler);
		line++;
		startOfLine = endLine;
	}

	styler.Flush();
}

// adaption by ksc, using the "} else {" trick of 1.53
// 030721
void SCI_METHOD LexerProps::Fold(Sci_PositionU startPos, Sci_Position length, int /* initStyle */, IDocument *pAccess) {
	if (!options.fold)
		return;

	LexAccessor styler(pAccess);

	const Sci_PositionU endPos = startPos + length;
	int visibleChars = 0;
	Sci_Position lineCurrent = styler.GetLine(startPos);

	char chNext = styler[startPos];
	bool headerPoint = false;
	int levelPrevious = (lineCurrent > 0) ? styler.LevelAt(lineCurrent - 1) : SC_FOLDLEVELBASE;

	for (Sci_PositionU i = startPos; i < endPos; i++) {
		const char ch = chNext;
		chNext = styler[i+1];

		const int style = styler.StyleIndexAt(i);
		const bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');

		if (style == SCE_PROPS_SECTION) {
			headerPoint = true;
		}

		if (atEOL) {
			int lev = levelPrevious & SC_FOLDLEVELNUMBERMASK;
			if (headerPoint) {
				lev = SC_FOLDLEVELBASE | SC_FOLDLEVELHEADERFLAG;
				if (levelPrevious & SC_FOLDLEVELHEADERFLAG) {
					// previous section is empty
					styler.SetLevel(lineCurrent - 1, SC_FOLDLEVELBASE);
				}
			} else if (levelPrevious & SC_FOLDLEVELHEADERFLAG) {
				lev += 1;
			}

			if (visibleChars == 0 && options.foldCompact)
				lev |= SC_FOLDLEVELWHITEFLAG;
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}

			lineCurrent++;
			visibleChars = 0;
			headerPoint = false;
			levelPrevious = lev;
		}
		if (!isspacechar(ch))
			visibleChars++;
	}

	int level = levelPrevious & SC_FOLDLEVELNUMBERMASK;
	if (levelPrevious & SC_FOLDLEVELHEADERFLAG) {
		level += 1;
	}
	const int flagsNext = styler.LevelAt(lineCurrent);
	styler.SetLevel(lineCurrent, level | (flagsNext & ~SC_FOLDLEVELNUMBERMASK));
}

}

extern const LexerModule lmProps(SCLEX_PROPERTIES, LexerProps::LexerFactoryProps, "props", emptyWordListDesc);
