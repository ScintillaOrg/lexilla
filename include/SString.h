// SciTE - Scintilla based Text Editor
// SString.h - a simple string class
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.
/** @file **/

#ifndef SSTRING_H
#define SSTRING_H

bool EqualCaseInsensitive(const char *a, const char *b);

#if PLAT_WIN
#define strcasecmp  stricmp
#define strncasecmp strnicmp
#endif

// Define another string class.
// While it would be 'better' to use std::string, that doubles the executable size.
// An SString may contain embedded nul characters.

/** Duplicate a C string.
 * Allocate memory of the given size, or big enough to fit the string if length isn't given;
 * then copy the given string in the allocated memory.
 * @return the pointer to the new string
 **/
inline char *StringDup(
	const char *s,	///< The string to duplicate
	int len=-1)		///< The length of memory to allocate. Optional.
{
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

/** A simple string class.
 * Hold the length of the string for quick operations,
 * can have a buffer bigger than the string to avoid too many memory allocations and copies.
 * May have embedded zeroes as a result of @a substitute, but rely too heavily on C string
 * functions to allow reliable manipulations of these strings.
 **/
class SString {
	char *s;		///< The C string
	int ssize;		///< The size of the buffer, less 1: ie. the maximum size of the string
	int slen;		///< The size of the string in s

	/// Minimum growth size when appending strings
	enum { sizingGranularity = 64 };

public:
	typedef const char* const_iterator;
	typedef int size_type;

	SString() {
		s = 0;
		ssize = 0;
		slen = 0;
	}
	SString(const SString &source) {
		s = StringDup(source.s);
		ssize = slen = (s) ? strlen(s) : 0;
	}
	SString(const char *s_) {
		s = StringDup(s_);
		ssize = slen = (s) ? strlen(s) : 0;
	}
	SString(const char *s_, int first, int last) {
		s = StringDup(s_ + first, last - first);
		ssize = slen = (s) ? strlen(s) : 0;
	}
	SString(int i) {
		char number[32];
		sprintf(number, "%0d", i);
		s = StringDup(number);
		ssize = slen = (s) ? strlen(s) : 0;
	}
	~SString() {
		delete []s;
		s = 0;
		ssize = 0;
		slen = 0;
	}
	void clear(void) {
		if (s) {
			*s = '\0';
		}
		slen = 0;
	}
	const char* begin(void) const {
		return s;
	}
	const char* end(void) const {
		return &s[slen];	// Point after the last character
	}
	size_type size(void) const {	// Size of buffer
		if (s)
			return ssize;
		else
			return 0;
	}
	int length() const {	// Size of string in buffer
		return slen;
	}
	SString &assign(const char* sother, int size_ = -1) {
		if (!sother) {
			size_ = 0;
		}
		if (size_ < 0) {
			size_ = strlen(sother);
		}
		if (ssize > 0 && size_ <= ssize) {	// Does not allocate new buffer if the current is big enough
			if (s && size_) {
			strncpy(s, sother, size_);
			}
			s[size_] = '\0';
			slen = size_;
		} else {
			delete []s;
			s = StringDup(sother, size_);
			if (s) {
			ssize = size_;	// Allow buffer bigger than real string, thus providing space to grow
				slen = strlen(s);
			} else {
				ssize = slen = 0;
			}
		}
		return *this;
	}
	SString &assign(const SString& sother, int size_ = -1) {
		return assign(sother.s, size_);
	}
	SString &assign(const_iterator ibeg, const_iterator iend) {
		return assign(ibeg, iend - ibeg);
	}
	SString &operator=(const char *source) {
		return assign(source);
	}
	SString &operator=(const SString &source) {
		if (this != &source) {
			assign(source.c_str());
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
	char operator[](int i) const {
		if (s && i < ssize)	// Or < slen? Depends on the use, both are OK
			return s[i];
		else
			return '\0';
	}
	SString &append(const char* sother, int lenOther = -1) {
		if (lenOther < 0)
			lenOther = strlen(sother);
		if (slen + lenOther + 1 < ssize) {
			// Conservative about growing the buffer: don't do it, unless really needed
			strncpy(&s[slen], sother, lenOther);
			s[slen + lenOther] = '\0';
			slen += lenOther;
		} else {
			// Grow the buffer bigger than really needed, to have room for other appends
			char *sNew = new char[slen + lenOther + sizingGranularity + 1];
			if (sNew) {
				if (s) {
					memcpy(sNew, s, slen);
					delete []s;
				}
				strncpy(&sNew[slen], sother, lenOther);
				sNew[slen + lenOther] = '\0';
				s = sNew;
				ssize = slen + lenOther + sizingGranularity;
				slen += lenOther;
			}
		}
		return *this;
	}
	SString &operator +=(const char *sother) {
		return append(sother, -1);
	}
	SString &operator +=(const SString &sother) {
		return append(sother.s, sother.ssize);
	}
	SString &operator +=(char ch) {
		return append(&ch, 1);
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
			if (t) {
				*t = replace;
				t++;
			}
		}
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
