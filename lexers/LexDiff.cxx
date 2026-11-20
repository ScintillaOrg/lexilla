// Scintilla source code edit control
/** @file LexDiff.cxx
 ** Lexer for diff results.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdarg>

#include <string>
#include <string_view>
#include <map>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"
#include "OptionSet.h"
#include "DefaultLexer.h"

using namespace Scintilla;
using namespace Lexilla;

namespace {

// Options used for LexerDiff
struct OptionsDiff {
	bool fold = false;
};

const char *const emptyWordListDesc[] = {
	nullptr
};

struct OptionSetDiff : public OptionSet<OptionsDiff> {
	OptionSetDiff() {
		DefineProperty("fold", &OptionsDiff::fold);
	}
};

const LexicalClass lexicalClasses[] = {
	// Lexer diff SCLEX_DIFF SCE_DIFF_
	0, "SCE_DIFF_DEFAULT", "default", "Default",
	1, "SCE_DIFF_COMMENT", "comment", "Comment",
	2, "SCE_DIFF_COMMAND", "literal string", "Command",
	3, "SCE_DIFF_HEADER", "literal string", "Source and destination file",
	4, "SCE_DIFF_POSITION", "literal string", "Position",
	5, "SCE_DIFF_DELETED", "default", "Line removal",
	6, "SCE_DIFF_ADDED", "default", "Line addition",
	7, "SCE_DIFF_CHANGED", "default", "Line changed",
	8, "SCE_DIFF_PATCH_ADD", "default", "Patch add",
	9, "SCE_DIFF_PATCH_DELETE", "default", "Patch delete",
	10, "SCE_DIFF_REMOVED_PATCH_ADD", "default", "Removed patch add",
	11, "SCE_DIFF_REMOVED_PATCH_DELETE", "default", "Removed patch delete",
};

class LexerDiff : public DefaultLexer {
	OptionsDiff options;
	OptionSetDiff osDiff;
public:
	LexerDiff() :
		DefaultLexer("diff", SCLEX_DIFF, lexicalClasses, std::size(lexicalClasses)) {
		SetOptionSet(&osDiff);
	}

	Sci_Position SCI_METHOD PropertySet(const char *key, const char *val) override {
		if (osDiff.PropertySet(&options, key, val)) {
			return 0;
		}
		return -1;
	}

	static void ColouriseLine(const std::string &lineBuffer, Sci_PositionU endLine, LexAccessor &styler);

	void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess) override;
	void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess) override;

	static ILexer5 *LexerFactoryDiff() {
		return new LexerDiff();
	}
};

void LexerDiff::ColouriseLine(const std::string &lineBuffer, Sci_PositionU endLine, LexAccessor &styler) {
	// It is needed to remember the current state to recognize starting
	// comment lines before the first "diff " or "--- ". If a real
	// difference starts then each line starting with ' ' is a whitespace
	// otherwise it is considered a comment (Only in..., Binary file...)
	if (lineBuffer.starts_with("diff ")) {
		styler.ColourTo(endLine, SCE_DIFF_COMMAND);
	} else if (lineBuffer.starts_with("Index: ")) {  // For subversion's diff
		styler.ColourTo(endLine, SCE_DIFF_COMMAND);
	} else if (lineBuffer.starts_with("---") && lineBuffer[3] != '-') {
		// In a context diff, --- appears in both the header and the position markers
		if (lineBuffer[3] == ' ' && atoi(lineBuffer.c_str() + 4) && (lineBuffer.find('/') == std::string::npos))
			styler.ColourTo(endLine, SCE_DIFF_POSITION);
		else if (lineBuffer[3] == '\r' || lineBuffer[3] == '\n')
			styler.ColourTo(endLine, SCE_DIFF_POSITION);
		else if (lineBuffer[3] == ' ')
			styler.ColourTo(endLine, SCE_DIFF_HEADER);
		else
			styler.ColourTo(endLine, SCE_DIFF_DELETED);
	} else if (lineBuffer.starts_with("+++ ")) {
		// I don't know of any diff where "+++ " is a position marker, but for
		// consistency, do the same as with "--- " and "*** ".
		if (atoi(lineBuffer.c_str() + 4) && (lineBuffer.find('/') == std::string::npos))
			styler.ColourTo(endLine, SCE_DIFF_POSITION);
		else
			styler.ColourTo(endLine, SCE_DIFF_HEADER);
	} else if (lineBuffer.starts_with("====")) {  // For p4's diff
		styler.ColourTo(endLine, SCE_DIFF_HEADER);
	} else if (lineBuffer.starts_with("***")) {
		// In a context diff, *** appears in both the header and the position markers.
		// Also ******** is a chunk header, but here it's treated as part of the
		// position marker since there is no separate style for a chunk header.
		if (lineBuffer[3] == ' ' && atoi(lineBuffer.c_str() + 4) && (lineBuffer.find('/') == std::string::npos))
			styler.ColourTo(endLine, SCE_DIFF_POSITION);
		else if (lineBuffer[3] == '*')
			styler.ColourTo(endLine, SCE_DIFF_POSITION);
		else
			styler.ColourTo(endLine, SCE_DIFF_HEADER);
	} else if (lineBuffer.starts_with("? ")) {    // For difflib
		styler.ColourTo(endLine, SCE_DIFF_HEADER);
	} else if (lineBuffer[0] == '@') {
		styler.ColourTo(endLine, SCE_DIFF_POSITION);
	} else if (lineBuffer[0] >= '0' && lineBuffer[0] <= '9') {
		styler.ColourTo(endLine, SCE_DIFF_POSITION);
	} else if (lineBuffer.starts_with("++")) {
		styler.ColourTo(endLine, SCE_DIFF_PATCH_ADD);
	} else if (lineBuffer.starts_with("+-")) {
		styler.ColourTo(endLine, SCE_DIFF_PATCH_DELETE);
	} else if (lineBuffer.starts_with("-+")) {
		styler.ColourTo(endLine, SCE_DIFF_REMOVED_PATCH_ADD);
	} else if (lineBuffer.starts_with("--")) {
		styler.ColourTo(endLine, SCE_DIFF_REMOVED_PATCH_DELETE);
	} else if (lineBuffer[0] == '-' || lineBuffer[0] == '<') {
		styler.ColourTo(endLine, SCE_DIFF_DELETED);
	} else if (lineBuffer[0] == '+' || lineBuffer[0] == '>') {
		styler.ColourTo(endLine, SCE_DIFF_ADDED);
	} else if (lineBuffer[0] == '!') {
		styler.ColourTo(endLine, SCE_DIFF_CHANGED);
	} else if (lineBuffer[0] != ' ') {
		styler.ColourTo(endLine, SCE_DIFF_COMMENT);
	} else {
		styler.ColourTo(endLine, SCE_DIFF_DEFAULT);
	}
}

void LexerDiff::Lex(Sci_PositionU startPos, Sci_Position length, int /* initStyle */, IDocument *pAccess) {
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
		ColouriseLine(lineBuffer, endLine - 1, styler);
		line++;
		startOfLine = endLine;
	}

	styler.Flush();
}

void LexerDiff::Fold(Sci_PositionU startPos, Sci_Position length, int /* initStyle */, IDocument *pAccess) {
	if (!options.fold)
		return;

	LexAccessor styler(pAccess);

	const Sci_Position endPos = startPos + length;

	Sci_Position curLine = styler.GetLine(startPos);
	// Backtrack to previous line in case need to fix its fold status
	if ((startPos > 0) && (curLine > 0)) {
		curLine--;
	}
	Sci_Position curLineStart = styler.LineStart(curLine);

	int prevLevel = curLine > 0 ? styler.LevelAt(curLine - 1) : SC_FOLDLEVELBASE;

	do {
		int nextLevel = 0;
		const int lineType = styler.StyleAt(curLineStart);
		if (lineType == SCE_DIFF_COMMAND)
			nextLevel = SC_FOLDLEVELBASE | SC_FOLDLEVELHEADERFLAG;
		else if (lineType == SCE_DIFF_HEADER)
			nextLevel = (SC_FOLDLEVELBASE + 1) | SC_FOLDLEVELHEADERFLAG;
		else if (lineType == SCE_DIFF_POSITION && styler[curLineStart] != '-')
			nextLevel = (SC_FOLDLEVELBASE + 2) | SC_FOLDLEVELHEADERFLAG;
		else if (prevLevel & SC_FOLDLEVELHEADERFLAG)
			nextLevel = (prevLevel & SC_FOLDLEVELNUMBERMASK) + 1;
		else
			nextLevel = prevLevel;

		if ((nextLevel & SC_FOLDLEVELHEADERFLAG) && (nextLevel == prevLevel))
			styler.SetLevel(curLine-1, prevLevel & ~SC_FOLDLEVELHEADERFLAG);

		styler.SetLevel(curLine, nextLevel);
		prevLevel = nextLevel;

		curLineStart = styler.LineStart(++curLine);
	} while (curLineStart < endPos);

	styler.Flush();
}

}

extern const LexerModule lmDiff(SCLEX_DIFF, LexerDiff::LexerFactoryDiff, "diff", emptyWordListDesc);
