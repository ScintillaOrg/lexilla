// SciTE - Scintilla based Text Editor
/** @file SString.h
 ** A simple string class.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SSTRING_H
#define SSTRING_H

// These functions are implemented because each platform calls them something different.
int CompareCaseInsensitive(const char *a, const char *b);
int CompareNCaseInsensitive(const char *a, const char *b, size_t len);
bool EqualCaseInsensitive(const char *a, const char *b);

// Define another string class.
// While it would be 'better' to use std::string, that doubles the executable size.
// An SString may contain embedded nul characters.

/**
 * @brief A simple string class.
 *
 * Hold the length of the string for quick operations,
 * can have a buffer bigger than the string to avoid too many memory allocations and copies.
 * May have embedded zeroes as a result of @a substitute, but relies too heavily on C string
 * functions to allow reliable manipulations of these strings, other than simple appends, etc.
 **/
class SString {
public:
	/** Type of string lengths (sizes) and positions (indexes). */
	typedef size_t lenpos_t;
	/** Out of bounds value indicating that the string argument should be measured. */
	enum { measure_length=0xffffffffU};

private:
	char *s;				///< The C string
	lenpos_t sSize;			///< The size of the buffer, less 1: ie. the maximum size of the string
	lenpos_t sLen;			///< The size of the string in s
	lenpos_t sizeGrowth;	///< Minimum growth size when appending strings
	enum { sizeGrowthDefault = 64 };

	bool grow(lenpos_t lenNew);
	SString &assign(const char *sOther, lenpos_t sSize_=measure_length);

public:
	SString() : s(0), sSize(0), sLen(0), sizeGrowth(sizeGrowthDefault) {
	}
	SString(const SString &source) : sizeGrowth(sizeGrowthDefault) {
		s = StringAllocate(source.s);
		sSize = sLen = (s) ? strlen(s) : 0;
	}
	SString(const char *s_) : sizeGrowth(sizeGrowthDefault) {
		s = StringAllocate(s_);
		sSize = sLen = (s) ? strlen(s) : 0;
	}
	SString(const char *s_, lenpos_t first, lenpos_t last) : sizeGrowth(sizeGrowthDefault) {
		// note: expects the "last" argument to point one beyond the range end (a la STL iterators)
		s = StringAllocate(s_ + first, last - first);
		sSize = sLen = (s) ? strlen(s) : 0;
	}
	SString(int i);
	SString(double d, int precision);
	~SString() {
		delete []s;
		s = 0;
		sSize = 0;
		sLen = 0;
	}
	void clear() {
		if (s) {
			*s = '\0';
		}
		sLen = 0;
	}
	/** Size of buffer. */
	lenpos_t size() const {
		if (s)
			return sSize;
		else
			return 0;
	}
	/** Size of string in buffer. */
	lenpos_t length() const {
		return sLen;
	}
	SString &operator=(const char *source) {
		return assign(source);
	}
	SString &operator=(const SString &source) {
		if (this != &source) {
			assign(source.c_str(), source.sLen);
		}
		return *this;
	}
	bool operator==(const SString &sOther) const;
	bool operator!=(const SString &sOther) const {
		return !operator==(sOther);
	}
	bool operator==(const char *sOther) const;
	bool operator!=(const char *sOther) const {
		return !operator==(sOther);
	}
	bool contains(char ch) {
		return (s && *s) ? strchr(s, ch) != 0 : false;
	}
	void setsizegrowth(lenpos_t sizeGrowth_) {
		sizeGrowth = sizeGrowth_;
	}
	const char *c_str() const {
		return s ? s : "";
	}
	/** Attach to a string allocated by means of StringAlloc(len). */
	SString &attach(char *s_, lenpos_t sLen_ = measure_length, lenpos_t sSize_ = measure_length) {
		delete []s;
		s = s_;
		if (!s) {
			sLen = sSize = 0;
		} else {
			sLen = (sLen_ == measure_length ? strlen(s) : sLen_);
			sSize = (sSize_ == measure_length ? sLen + 1 : sSize_);
		}
		return *this;
	}
	/** Give ownership of buffer to caller which must use delete[] to free buffer. */
	char *detach() {
		char *sRet = s;
		s = 0;
		sSize = 0;
		sLen = 0;
		return sRet;
	}
	char operator[](lenpos_t i) const {
		return (s && i < sSize)	? s[i] : '\0';
	}
	SString substr(lenpos_t subPos, lenpos_t subLen=measure_length) const;
	SString &lowercase(lenpos_t subPos = 0, lenpos_t subLen=measure_length);
	SString &uppercase(lenpos_t subPos = 0, lenpos_t subLen=measure_length);
	SString &append(const char *sOther, lenpos_t sLenOther=measure_length, char sep = '\0');
	SString &operator+=(const char *sOther) {
		return append(sOther, static_cast<lenpos_t>(measure_length));
	}
	SString &operator+=(const SString &sOther) {
		return append(sOther.s, sOther.sLen);
	}
	SString &operator+=(char ch) {
		return append(&ch, 1);
	}
	SString &appendwithseparator(const char *sOther, char sep) {
		return append(sOther, strlen(sOther), sep);
	}
	SString &insert(lenpos_t pos, const char *sOther, lenpos_t sLenOther=measure_length);

	/**
	 * Remove @a len characters from the @a pos position, included.
	 * Characters at pos + len and beyond replace characters at pos.
	 * If @a len is 0, or greater than the length of the string
	 * starting at @a pos, the string is just truncated at @a pos.
	 */
	void remove(lenpos_t pos, lenpos_t len);

	SString &change(lenpos_t pos, char ch) {
		if (pos < sLen) {					// character changed must be in string bounds
			*(s + pos) = ch;
		}
		return *this;
	}
	/** Read an integral numeric value from the string. */
	int value() const {
		return s ? atoi(s) : 0;
	}
	bool startswith(const char *prefix);
	bool endswith(const char *suffix);
	int search(const char *sFind, lenpos_t start=0) const;
	bool contains(const char *sFind) {
		return search(sFind) >= 0;
	}
	int substitute(char chFind, char chReplace);
	int substitute(const char *sFind, const char *sReplace);
	int remove(const char *sFind) {
		return substitute(sFind, "");
	}
	/**
	 * Duplicate a C string.
	 * Allocate memory of the given size, or big enough to fit the string if length isn't given;
	 * then copy the given string in the allocated memory.
	 * @return the pointer to the new string
	 */
	static char *StringAllocate(
		const char *s,			///< The string to duplicate
		lenpos_t len=measure_length);	///< The length of memory to allocate. Optional.
	/**
	 * Allocate uninitialized memory big enough to fit a string of the given length
	 * @return the pointer to the new string
	 */
	static char *StringAllocate(lenpos_t len);
};

/**
 * Duplicate a C string.
 * Allocate memory of the given size, or big enough to fit the string if length isn't given;
 * then copy the given string in the allocated memory.
 * @return the pointer to the new string
 */
inline char *StringDup(
	const char *s,			///< The string to duplicate
	SString::lenpos_t len=SString::measure_length)	///< The length of memory to allocate. Optional.
{
	return SString::StringAllocate(s, len);
}

#endif
