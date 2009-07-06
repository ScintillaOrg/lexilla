// Scintilla source code edit control
/** @file Selection.h
 ** Classes maintaining the selection.
 **/
// Copyright 2009 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SELECTION_H
#define SELECTION_H

#ifdef SCI_NAMESPACE
namespace Scintilla {
#endif

class SelectionPosition {
	int position;
	int virtualSpace;
public:
	explicit SelectionPosition(int position_=INVALID_POSITION, int virtualSpace_=0) : position(position_), virtualSpace(virtualSpace_) {
		PLATFORM_ASSERT(virtualSpace < 800000);
		if (virtualSpace < 0)
			virtualSpace = 0;
	}
	void Reset() {
		position = 0;
		virtualSpace = 0;
	}
	void MoveForInsertDelete(bool insertion, int startChange, int length);
	bool operator ==(const SelectionPosition &other) const {
		return position == other.position && virtualSpace == other.virtualSpace;
	}
	bool operator <(const SelectionPosition &other) const;
	bool operator >(const SelectionPosition &other) const;
	bool operator <=(const SelectionPosition &other) const;
	bool operator >=(const SelectionPosition &other) const;
	int Position() const {
		return position;
	}
	void SetPosition(int position_) {
		position = position_;
		virtualSpace = 0;
	}
	int VirtualSpace() const {
		return virtualSpace;
	}
	void SetVirtualSpace(int virtualSpace_) {
		PLATFORM_ASSERT(virtualSpace_ < 800000);
		if (virtualSpace_ >= 0)
			virtualSpace = virtualSpace_;
	}
	void Add(int increment) {
		position = position + increment;
	}
	bool IsValid() const {
		return position >= 0;
	}
};

struct SelectionRange {
	SelectionPosition caret;
	SelectionPosition anchor;

	SelectionRange() {
	}
	SelectionRange(SelectionPosition single) : caret(single), anchor(single) {
	}
	SelectionRange(SelectionPosition caret_, SelectionPosition anchor_) : caret(caret_), anchor(anchor_) {
	}
	SelectionRange(int caret_, int anchor_) : caret(caret_), anchor(anchor_) {
	}
	bool Empty() const {
		return anchor == caret;
	}
	int Length() const;
	// int Width() const;	// Like Length but takes virtual space into account
	bool operator ==(const SelectionRange &other) const {
		return caret == other.caret && anchor == other.anchor;
	}
	void Reset() {
		anchor.Reset();
		caret.Reset();
	}
	void ClearVirtualSpace() {
		anchor.SetVirtualSpace(0);
		caret.SetVirtualSpace(0);
	}
	bool Contains(int pos) const;
	bool Contains(SelectionPosition sp) const;
	bool ContainsCharacter(int posCharacter) const;
	bool Intersect(int start, int end, SelectionPosition &selStart, SelectionPosition &selEnd) const;
	SelectionPosition Start() const {
		return (anchor < caret) ? anchor : caret;
	}
	SelectionPosition End() const {
		return (anchor < caret) ? caret : anchor;
	}
	bool Trim(SelectionPosition startPos, SelectionPosition endPos);
	// If range is all virtual collapse to start of virtual space
	void MinimizeVirtualSpace();
};

class Selection {
	SelectionRange *ranges;
	SelectionRange rangeRectangular;
	size_t allocated;
	size_t nRanges;
	size_t mainRange;
	bool moveExtends;
	void Allocate();
public:
	enum selTypes { noSel, selStream, selRectangle, selLines, selThin };
	selTypes selType;

	Selection();
	~Selection();
	bool IsRectangular() const;
	int MainCaret() const;
	int MainAnchor() const;
	SelectionRange &Rectangular();
	size_t Count() const;
	size_t Main() const;
	void SetMain(size_t r);
	SelectionRange &Range(size_t r);
	SelectionRange &RangeMain();
	void ClearVirtualSpace(size_t r);
	bool MoveExtends() const;
	void SetMoveExtends(bool moveExtends_);
	bool Empty() const;
	SelectionPosition Last() const;
	int Length() const;
	void MovePositions(bool insertion, int startChange, int length);
	void TrimSelection(SelectionPosition startPos, SelectionPosition endPos);
	void AddSelection(SelectionPosition spPos);
	void AddSelection(SelectionPosition spStartPos, SelectionPosition spEndPos, bool anchorLeft);
	bool CharacterInSelection(int posCharacter) const;
	bool InSelectionForEOL(int pos) const;
	int VirtualSpaceFor(int pos) const;
	void Clear();
	void EmptyRanges();
};

#ifdef SCI_NAMESPACE
}
#endif

#endif
