// Unit Tests for Scintilla internal data structures

#include <cstring>

#include <string>
#include <algorithm>

#include "Platform.h"

#include "UniConversion.h"

#include "catch.hpp"

using namespace Scintilla;

// Test UniConversion.
// Use examples from Wikipedia:
// https://en.wikipedia.org/wiki/UTF-8

TEST_CASE("UTF16Length") {

	SECTION("UTF16Length ASCII") {
		// Latin Small Letter A
		const char *s = "a";
		size_t len = UTF16Length(s, strlen(s));
		REQUIRE(len == 1U);
	}

	SECTION("UTF16Length Example1") {
		// Dollar Sign
		const char *s = "\x24";
		size_t len = UTF16Length(s, strlen(s));
		REQUIRE(len == 1U);
	}

	SECTION("UTF16Length Example2") {
		// Cent Sign
		const char *s = "\xC2\xA2";
		size_t len = UTF16Length(s, strlen(s));
		REQUIRE(len == 1U);
	}

	SECTION("UTF16Length Example3") {
		// Euro Sign
		const char *s = "\xE2\x82\xAC";
		size_t len = UTF16Length(s, strlen(s));
		REQUIRE(len == 1U);
	}

	SECTION("UTF16Length Example4") {
		// Gothic Letter Hwair
		const char *s = "\xF0\x90\x8D\x88";
		size_t len = UTF16Length(s, strlen(s));
		REQUIRE(len == 2U);
	}
}

TEST_CASE("UniConversion") {

	// UTF16FromUTF8

	SECTION("UTF16FromUTF8 ASCII") {
		const char s[] = {'a', 0};
		wchar_t tbuf[1] = {0};
		size_t tlen = UTF16FromUTF8(s, 1, tbuf, 1);
		REQUIRE(tlen == 1U);
		REQUIRE(tbuf[0] == 'a');
	}

	SECTION("UTF16FromUTF8 Example1") {
		const char s[] = {'\x24', 0};
		wchar_t tbuf[1] = {0};
		size_t tlen = UTF16FromUTF8(s, 1, tbuf, 1);
		REQUIRE(tlen == 1U);
		REQUIRE(tbuf[0] == 0x24);
	}

	SECTION("UTF16FromUTF8 Example2") {
		const char s[] = {'\xC2', '\xA2', 0};
		wchar_t tbuf[1] = {0};
		size_t tlen = UTF16FromUTF8(s, 2, tbuf, 1);
		REQUIRE(tlen == 1U);
		REQUIRE(tbuf[0] == 0xA2);
	}

	SECTION("UTF16FromUTF8 Example3") {
		const char s[] = {'\xE2', '\x82', '\xAC', 0};
		wchar_t tbuf[1] = {0};
		size_t tlen = UTF16FromUTF8(s, 3, tbuf, 1);;
		REQUIRE(tlen == 1U);
		REQUIRE(tbuf[0] == 0x20AC);
	}

	SECTION("UTF16FromUTF8 Example4") {
		const char s[] = {'\xF0', '\x90', '\x8D', '\x88', 0};
		wchar_t tbuf[2] = {0, 0};
		size_t tlen = UTF16FromUTF8(s, 4, tbuf, 2);
		REQUIRE(tlen == 2U);
		REQUIRE(tbuf[0] == 0xD800);
		REQUIRE(tbuf[1] == 0xDF48);
	}

	// UTF32FromUTF8

	SECTION("UTF32FromUTF8 ASCII") {
		const char s[] = {'a', 0};
		unsigned int tbuf[1] = {0};
		size_t tlen = UTF32FromUTF8(s, 1, tbuf, 1);
		REQUIRE(tlen == 1U);
		REQUIRE(tbuf[0] == static_cast<unsigned int>('a'));
	}

	SECTION("UTF32FromUTF8 Example1") {
		const char s[] = {'\x24', 0};
		unsigned int tbuf[1] = {0};
		size_t tlen = UTF32FromUTF8(s, 1, tbuf, 1);
		REQUIRE(tlen == 1U);
		REQUIRE(tbuf[0] == 0x24);
	}

	SECTION("UTF32FromUTF8 Example2") {
		const char s[] = {'\xC2', '\xA2', 0};
		unsigned int tbuf[1] = {0};
		size_t tlen = UTF32FromUTF8(s, 2, tbuf, 1);
		REQUIRE(tlen == 1U);
		REQUIRE(tbuf[0] == 0xA2);
	}

	SECTION("UTF32FromUTF8 Example3") {
		const char s[] = {'\xE2', '\x82', '\xAC', 0};
		unsigned int tbuf[1] = {0};
		size_t tlen = UTF32FromUTF8(s, 3, tbuf, 1);
		REQUIRE(tlen == 1U);
		REQUIRE(tbuf[0] == 0x20AC);
	}

	SECTION("UTF32FromUTF8 Example4") {
		const char s[] = {'\xF0', '\x90', '\x8D', '\x88', 0};
		unsigned int tbuf[1] = {0};
		size_t tlen = UTF32FromUTF8(s, 4, tbuf, 1);
		REQUIRE(tlen == 1U);
		REQUIRE(tbuf[0] == 0x10348);
	}
}

TEST_CASE("UTF8Classify") {

	// These tests are supposed to hit every return statement in UTF8Classify once in order
	// except the last which is hit twice.

	// Single byte

	SECTION("UTF8Classify Simple ASCII") {
		const char *s = "a";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == 1);
	}

	SECTION("UTF8Classify Invalid Too large lead") {
		const char *s = "\xF5";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (1|UTF8MaskInvalid));
	}

	// 4 byte lead

	SECTION("UTF8Classify 4 byte lead, string less than 4 long") {
		const char *s = "\xF0";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (1 | UTF8MaskInvalid));
	}

	SECTION("UTF8Classify 1FFFF non-character") {
		const char *s = "\xF0\x9F\xBF\xBF";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (4 | UTF8MaskInvalid));
	}

	SECTION("UTF8Classify 1 Greater than max Unicode 110000") {
		// Maximum Unicode value is 10FFFF so 110000 is out of range
		const char *s = "\xF4\x90\x80\x80";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (1 | UTF8MaskInvalid));
	}

	SECTION("UTF8Classify 4 byte overlong") {
		const char *s = "\xF0\x80\x80\x80";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (1 | UTF8MaskInvalid));
	}

	SECTION("UTF8Classify 4 byte valid character") {
		const char *s = "\xF0\x9F\x8C\x90";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == 4);
	}

	SECTION("UTF8Classify 4 byte bad trails") {
		const char *s = "\xF0xyz";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (1 | UTF8MaskInvalid));
	}

	// 3 byte lead

	SECTION("UTF8Classify 3 byte lead, string less than 3 long") {
		const char *s = "\xEF";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (1 | UTF8MaskInvalid));
	}

	SECTION("UTF8Classify 3 byte lead, overlong") {
		const char *s = "\xE0\x80\xAF";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (1 | UTF8MaskInvalid));
	}

	SECTION("UTF8Classify 3 byte lead, surrogate") {
		const char *s = "\xED\xA0\x80";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (1 | UTF8MaskInvalid));
	}

	SECTION("UTF8Classify FFFE non-character") {
		const char *s = "\xEF\xBF\xBE";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (3 | UTF8MaskInvalid));
	}

	SECTION("UTF8Classify FFFF non-character") {
		const char *s = "\xEF\xBF\xBF";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (3 | UTF8MaskInvalid));
	}

	SECTION("UTF8Classify FDD0 non-character") {
		const char *s = "\xEF\xB7\x90";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (3 | UTF8MaskInvalid));
	}

	SECTION("UTF8Classify 3 byte valid character") {
		const char *s = "\xE2\x82\xAC";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == 3);
	}

	SECTION("UTF8Classify 3 byte bad trails") {
		const char *s = "\xE2qq";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (1 | UTF8MaskInvalid));
	}

	// 2 byte lead

	SECTION("UTF8Classify 2 byte lead, string less than 2 long") {
		const char *s = "\xD0";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (1 | UTF8MaskInvalid));
	}

	SECTION("UTF8Classify 2 byte valid character") {
		const char *s = "\xD0\x80";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == 2);
	}

	SECTION("UTF8Classify 2 byte lead trail is invalid") {
		const char *s = "\xD0q";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (1 | UTF8MaskInvalid));
	}

	SECTION("UTF8Classify Overlong") {
		const char *s = "\xC0";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (1 | UTF8MaskInvalid));
	}

	SECTION("UTF8Classify single trail byte") {
		const char *s = "\x80";
		REQUIRE(UTF8Classify(reinterpret_cast<const unsigned char *>(s), strlen(s)) == (1 | UTF8MaskInvalid));
	}
}