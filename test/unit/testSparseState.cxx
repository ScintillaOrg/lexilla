// Unit Tests for Scintilla internal data structures

#include <string>
#include <vector>
#include <algorithm>

#include "Platform.h"

#include "SparseState.h"

#include <gtest/gtest.h>

// Test SparseState.

class SparseStateTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		pss = new SparseState<int>();
	}

	virtual void TearDown() {
		delete pss;
		pss = 0;
	}

	SparseState<int> *pss;
};

TEST_F(SparseStateTest, IsEmptyInitially) {
	EXPECT_EQ(0u, pss->size());
	int val = pss->ValueAt(0);
	EXPECT_EQ(0, val);
}

TEST_F(SparseStateTest, SimpleSetAndGet) {
	pss->Set(0, 22);
	pss->Set(1, 23);
	EXPECT_EQ(2u, pss->size());
	EXPECT_EQ(0, pss->ValueAt(-1));
	EXPECT_EQ(22, pss->ValueAt(0));
	EXPECT_EQ(23, pss->ValueAt(1));
	EXPECT_EQ(23, pss->ValueAt(2));
}

TEST_F(SparseStateTest, RetrieveBetween) {
	pss->Set(0, 10);
	pss->Set(2, 12);
	EXPECT_EQ(2u, pss->size());
	EXPECT_EQ(0, pss->ValueAt(-1));
	EXPECT_EQ(10, pss->ValueAt(0));
	EXPECT_EQ(10, pss->ValueAt(1));
	EXPECT_EQ(12, pss->ValueAt(2));
}

TEST_F(SparseStateTest, RetrieveBefore) {
	pss->Set(2, 12);
	EXPECT_EQ(1u, pss->size());
	EXPECT_EQ(0, pss->ValueAt(-1));
	EXPECT_EQ(0, pss->ValueAt(0));
	EXPECT_EQ(0, pss->ValueAt(1));
	EXPECT_EQ(12, pss->ValueAt(2));
}

TEST_F(SparseStateTest, Delete) {
	pss->Set(0, 30);
	pss->Set(2, 32);
	pss->Delete(2);
	EXPECT_EQ(1u, pss->size());
	EXPECT_EQ(0, pss->ValueAt(-1));
	EXPECT_EQ(30, pss->ValueAt(0));
	EXPECT_EQ(30, pss->ValueAt(1));
	EXPECT_EQ(30, pss->ValueAt(2));
}

TEST_F(SparseStateTest, DeleteBetweeen) {
	pss->Set(0, 30);
	pss->Set(2, 32);
	pss->Delete(1);
	EXPECT_EQ(1u, pss->size());
	EXPECT_EQ(0, pss->ValueAt(-1));
	EXPECT_EQ(30, pss->ValueAt(0));
	EXPECT_EQ(30, pss->ValueAt(1));
	EXPECT_EQ(30, pss->ValueAt(2));
}

TEST_F(SparseStateTest, ReplaceLast) {
	pss->Set(0, 30);
	pss->Set(2, 31);
	pss->Set(2, 32);
	EXPECT_EQ(2u, pss->size());
	EXPECT_EQ(0, pss->ValueAt(-1));
	EXPECT_EQ(30, pss->ValueAt(0));
	EXPECT_EQ(30, pss->ValueAt(1));
	EXPECT_EQ(32, pss->ValueAt(2));
	EXPECT_EQ(32, pss->ValueAt(3));
}

TEST_F(SparseStateTest, CheckOnlyChangeAppended) {
	pss->Set(0, 30);
	pss->Set(2, 31);
	pss->Set(3, 31);
	EXPECT_EQ(2u, pss->size());
}

class SparseStateStringTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		pss = new SparseState<std::string>();
	}

	virtual void TearDown() {
		delete pss;
		pss = 0;
	}

	SparseState<std::string> *pss;
};

TEST_F(SparseStateStringTest, IsEmptyInitially) {
	EXPECT_EQ(0u, pss->size());
	std::string val = pss->ValueAt(0);
	EXPECT_EQ("", val);
}

TEST_F(SparseStateStringTest, SimpleSetAndGet) {
	EXPECT_EQ(0u, pss->size());
	pss->Set(0, "22");
	pss->Set(1, "23");
	EXPECT_EQ(2u, pss->size());
	EXPECT_EQ("", pss->ValueAt(-1));
	EXPECT_EQ("22", pss->ValueAt(0));
	EXPECT_EQ("23", pss->ValueAt(1));
	EXPECT_EQ("23", pss->ValueAt(2));
}

TEST_F(SparseStateStringTest, DeleteBetweeen) {
	pss->Set(0, "30");
	pss->Set(2, "32");
	pss->Delete(1);
	EXPECT_EQ(1u, pss->size());
	EXPECT_EQ("", pss->ValueAt(-1));
	EXPECT_EQ("30", pss->ValueAt(0));
	EXPECT_EQ("30", pss->ValueAt(1));
	EXPECT_EQ("30", pss->ValueAt(2));
}
