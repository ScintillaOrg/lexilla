// SciTE - Scintilla based Text Editor
// SString.h - a simple string class
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SSTRING_H
#define SSTRING_H

#if PLAT_WIN
#define strcasecmp  stricmp
#define strncasecmp strnicmp
#endif

// Define another string class.
// While it would be 'better' to use std::string, that doubles the executable size.

inline char *StringDup(const char *s, int len=-1) {
	if (!s)
		return 0;
	if (len == -1)
		len = strlen(s);
	char *sNew = new char[len + 1];
	if (sNew) {
		strncpy(sNew, s, len);
		sNew[len] = '\0';
	}
	return sNew;
}

class SString {
	char *s;
	int ssize;
public:
	typedef const char* const_iterator;
	typedef int size_type;
	static size_type npos;
	const char* begin(void) const {
		return s;
	}
	const char* end(void) const {
		return &s[ssize];
	}
	size_type size(void) const {
		if (s)
			return ssize;
		else
			return 0;
	}
	SString &assign(const char* sother, int size_ = -1) {
		char *t = s;
		s = StringDup(sother,size_);
		ssize = (s) ? strlen(s) : 0;
		delete []t;
		return *this;
	}
	SString &assign(const SString& sother, int size_ = -1) {
		return assign(sother.s,size_);
	}
	SString &assign(const_iterator ibeg, const_iterator iend) {
		return assign(ibeg,iend - ibeg);
	}
	SString() {
		s = 0;
		ssize = 0;
	}
	SString(const SString &source) {
		s = StringDup(source.s);
		ssize = (s) ? strlen(s) : 0;
	}
	SString(const char *s_) {
		s = StringDup(s_);
		ssize = (s) ? strlen(s) : 0;
	}
	SString(int i) {
		char number[100];
		sprintf(number, "%0d", i);
		s = StringDup(number);
		ssize = (s) ? strlen(s) : 0;
	}
	~SString() {
		delete []s;
		s = 0;
		ssize = 0;
	}
	SString &operator=(const SString &source) {
		if (this != &source) {
			delete []s;
			s = StringDup(source.s);
			ssize = (s) ? strlen(s) : 0;
		}
		return *this;
	}
	bool operator==(const SString &other) const {
		if ((s == 0) && (other.s == 0))
			return true;
		if ((s == 0) || (other.s == 0))
			return false;
		return strcmp(s, other.s) == 0;
	}
	bool operator!=(const SString &other) const {
		return !operator==(other);
	}
	bool operator==(const char *sother) const {
		if ((s == 0) && (sother == 0))
			return true;
		if ((s == 0) || (sother == 0))
			return false;
		return strcmp(s, sother) == 0;
	}
	bool operator!=(const char *sother) const {
		return !operator==(sother);
	}
	const char *c_str() const {
		if (s)
			return s;
		else
			return "";
	}
	int length() const {
		if (s)
			return strlen(s);
		else
			return 0;
	}
	char operator[](int i) const {
		if (s)
			return s[i];
		else
			return '\0';
	}
	SString &operator +=(const char *sother) {
		return append(sother,-1);
	}
	SString &operator +=(const SString &sother) {
		return append(sother.s,sother.ssize);
	}
	SString &operator +=(char ch) {
		return append(&ch,1);
	}
	SString &append(const char* sother, int lenOther) {
		int len = length();
		if(lenOther < 0) 
			lenOther = strlen(sother);
		char *sNew = new char[len + lenOther + 1];
		if (sNew) {
			if (s)
				memcpy(sNew, s, len);
			strncpy(&sNew[len], sother, lenOther);
			sNew[len + lenOther] = '\0';
			delete []s;
			s = sNew;
			ssize = (s) ? strlen(s) : 0;
		}
		return *this;
	}
	int value() const {
		if (s)
			return atoi(s);
		else 
			return 0;
	}
	void substitute(char find, char replace) {
		char *t = s;
		while (t) {
			t = strchr(t, find);
			if (t)
				*t = replace;
		}
	}
	//added by ajkc - 08122000
	char charAt(int i) {
		return s[i];
	}
	//added by ajkc - 08122000
	SString substring(int startPos, int endPos = -1) {
		SString result;

		//do some checks first so we do the "right" thing
		if (endPos > this->size() || endPos == -1)
			endPos = this->size();

		if (startPos < 0)
			startPos = 0;

		if (startPos == endPos) {
			//result += s[startPos];
			result += "";	// Why? -- Neil
			return result;
		}

		if (startPos > endPos) {
			int tmp = endPos;
			endPos = startPos;
			startPos = tmp;
		}

		//printf("SString: substring startPos %d endPos %d\n",startPos,endPos);
		for (int i = startPos; i < endPos; i++) {
			//printf("SString: substring %d: %c\n",i,s[i]);
			result += s[i];
		}

		//printf("SString: substring: returning: %s\n", result.c_str());
		return result;
	}
	// I don't think this really belongs here -- Neil
	void correctPath() {
#ifdef unix
		substitute('\\', '/');
#else
		substitute('/', '\\');
#endif
	}
};

#endif
