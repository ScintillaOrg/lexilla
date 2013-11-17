// Unit Tests for Scintilla internal data structures

#include <string.h>

#include <stdexcept>
#include <algorithm>

#include "Platform.h"

#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "Decoration.h"

#include "catch.hpp"

const int indicator=4;

// Test Decoration.

TEST_CASE("Decoration") {

	Decoration deco(indicator);

	SECTION("HasCorrectIndicator") {
		REQUIRE(indicator == deco.indicator);
	}

	SECTION("IsEmptyInitially") {
		REQUIRE(0 == deco.rs.Length());
		REQUIRE(1 == deco.rs.Runs());
		REQUIRE(deco.Empty());
	}

	SECTION("SimpleSpace") {
		deco.rs.InsertSpace(0, 1);
		REQUIRE(deco.Empty());
	}

	SECTION("SimpleRun") {
		deco.rs.InsertSpace(0, 1);
		deco.rs.SetValueAt(0, 2);
		REQUIRE(!deco.Empty());
	}
}
