// SciTE - Scintilla based Text Editor
// PropSet.h - a java style properties file module
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef PROPSET_H
#define PROPSET_H
#include "SString.h"

bool EqualCaseInsensitive(const char *a, const char *b);

bool isprefix(const char *target, const char *prefix);

struct Property {
	unsigned int hash;
	char *key;
	char *val;
	Property *next;
	Property() : hash(0), key(0), val(0), next(0) {}
};

class PropSet {
private:
	enum { hashRoots=31 };
	Property *props[hashRoots];
public:
	PropSet *superPS;
	PropSet();
	~PropSet();
	void Set(const char *key, const char *val);
	void Set(char *keyval);
	SString Get(const char *key);
	SString GetExpanded(const char *key);
	SString Expand(const char *withvars);
	int GetInt(const char *key, int defaultValue=0);
	SString GetWild(const char *keybase, const char *filename);
	SString GetNewExpand(const char *keybase, const char *filename);
	void Clear();
};

class WordList {
public:
	// Each word contains at least one character - a empty word acts as sentinal at the end.
	char **words;
	char **wordsNoCase;
	char *list;
	int len;
	bool onlyLineEnds;	// Delimited by any white space or only line ends
	bool sorted;
	int starts[256];
	WordList(bool onlyLineEnds_ = false) : 
		words(0), wordsNoCase(0), list(0), len(0), onlyLineEnds(onlyLineEnds_), sorted(false) {}
	~WordList() { Clear(); }
	operator bool() { return len ? true : false; }
	const char *operator[](int ind) { return words[ind]; }
	void Clear();
	void Set(const char *s);
	char *Allocate(int size);
	void SetFromAllocated();
	bool InList(const char *s);
	const char *GetNearestWord(const char *wordStart, int searchLen = -1, bool ignoreCase = false);
	char *GetNearestWords(const char *wordStart, int searchLen = -1, bool ignoreCase = false);
};

inline bool nonFuncChar(char ch) {
	return strchr("\t\n\r !\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~", ch) != NULL;
}

#endif
