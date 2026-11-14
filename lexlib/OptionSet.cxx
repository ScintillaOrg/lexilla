// Scintilla source code edit control
/** @file OptionSet.cxx
 ** Manage descriptive information about an options struct for a lexer.
 ** Hold the names, positions, and descriptions of boolean, integer and string options and
 ** allow setting options and retrieving metadata about the options.
 **/
// Copyright 2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <cstdlib>
#include <cassert>

#include <string>
#include <string_view>
#include <map>

#include "Scintilla.h"

#include "OptionSet.h"

namespace Lexilla {

std::string JoinWordListDescriptions(const char *const wordListDescriptions[]) {
	std::string wordLists;
	if (wordListDescriptions) {
		for (size_t wl = 0; wordListDescriptions[wl]; wl++) {
			if (wl > 0)
				wordLists += "\n";
			wordLists += wordListDescriptions[wl];
		}
	}
	return wordLists;
}

bool SetOptionValue(bool &b, const char *val) {
	const bool option = atoi(val) != 0;
	if (b != option) {
		b = option;
		return true;
	}
	return false;
}

bool SetOptionValue(int &i, const char *val) {
	const int option = atoi(val);
	if (i != option) {
		i = option;
		return true;
	}
	return false;
}

bool SetOptionValue(std::string &s, const char *val) {
	if (s != val) {
		s = val;
		return true;
	}
	return false;
}

}
