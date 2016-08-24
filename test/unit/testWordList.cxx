// Unit Tests for Scintilla internal data structures

#include <string.h>

#include "WordList.h"

#include "catch.hpp"

// Test WordList.

TEST_CASE("WordList") {

	WordList wl;

	SECTION("IsEmptyInitially") {
		REQUIRE(0 == wl.Length());
		REQUIRE(!wl.InList("struct"));
	}

	SECTION("InList") {
		wl.Set("else struct");
		REQUIRE(2 == wl.Length());
		REQUIRE(wl.InList("struct"));
		REQUIRE(!wl.InList("class"));
	}

	SECTION("WordAt") {
		wl.Set("else struct");
		REQUIRE(0 == strcmp(wl.WordAt(0), "else"));
	}

}
