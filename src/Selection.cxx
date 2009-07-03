// Scintilla source code edit control
/** @file Selection.cxx
 ** Classes maintaining the selection.
 **/
// Copyright 2009 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>

#include "Platform.h"

#include "Scintilla.h"

#include "Selection.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

void SelectionPosition::MoveForInsertDelete(bool insertion, int startChange, int length) {
	if (insertion) {
		if (position > startChange) {
			position += length;
		}
	} else {
		if (position > startChange) {
			int endDeletion = startChange + length;
			if (position > endDeletion) {
				position -= length;
			} else {
				position = startChange;
			}
		}
	}
}

bool SelectionPosition::operator <(const SelectionPosition &other) const {
	if (position == other.position)
		return virtualSpace < other.virtualSpace;
	else
		return position < other.position;
}

bool SelectionPosition::operator >(const SelectionPosition &other) const {
	if (position == other.position)
		return virtualSpace > other.virtualSpace;
	else
		return position > other.position;
}

bool SelectionPosition::operator <=(const SelectionPosition &other) const {
	if (position == other.position && virtualSpace == other.virtualSpace)
		return true;
	else
		return other > *this;
}

bool SelectionPosition::operator >=(const SelectionPosition &other) const {
	if (position == other.position && virtualSpace == other.virtualSpace)
		return true;
	else
		return *this > other;
}

int SelectionRange::Length() const {
	if (anchor > caret) {
		return anchor.Position() - caret.Position();
	} else {
		return caret.Position() - anchor.Position();
	}
}

bool SelectionRange::Contains(int pos) const {
	if (anchor > caret)
		return (pos >= caret.Position()) && (pos <= anchor.Position());
	else
		return (pos >= anchor.Position()) && (pos <= caret.Position());
}

bool SelectionRange::Contains(SelectionPosition sp) const {
	if (anchor > caret)
		return (sp >= caret) && (sp <= anchor);
	else
		return (sp >= anchor) && (sp <= caret);
}

bool SelectionRange::ContainsCharacter(int posCharacter) const {
	if (anchor > caret)
		return (posCharacter >= caret.Position()) && (posCharacter < anchor.Position());
	else
		return (posCharacter >= anchor.Position()) && (posCharacter < caret.Position());
}

bool SelectionRange::Intersect(int start, int end, SelectionPosition &selStart, SelectionPosition &selEnd) const {
	SelectionPosition spEnd(end, 100000);	// Large amount of virtual space
	SelectionPosition first;
	SelectionPosition last;
	if (anchor > caret) {
		first = caret;
		last = anchor;
	} else {
		first = anchor;
		last = caret;
	}
	if ((first < spEnd) && (last.Position() > start)) {
		if (start > first.Position()) 
			selStart = SelectionPosition(start);
		else 
			selStart = first;
		if (spEnd < last) 
			selEnd = SelectionPosition(end);
		else 
			selEnd = last;
		return true;
	} else {
		return false;
	}
}

bool SelectionRange::Trim(SelectionPosition startPos, SelectionPosition endPos) {
	SelectionPosition startRange = startPos;
	SelectionPosition endRange = endPos;
	if (startPos > endPos) {
		startRange = endPos;
		endRange = startPos;
	}
	SelectionPosition start = Start();
	SelectionPosition end = End();
	PLATFORM_ASSERT(start <= end);
	PLATFORM_ASSERT(startRange <= endRange);
	if ((startRange <= end) && (endRange >= start)) {
		if ((start > startRange) && (end < endRange)) {
			// Completely covered by range -> empty at start
			end = start;
		} else if ((start < startRange) && (end > endRange)) {
			// Completely covers range -> empty at start
			end = start;
		} else if (start <= startRange) {
			// Trim end
			end = startRange;
		} else { // 
			PLATFORM_ASSERT(end >= endRange);
			// Trim start
			start = endRange;
		}
		if (anchor > caret) {
			caret = start;
			anchor = end;
		} else {
			anchor = start;
			caret = end;
		}
		return Empty();
	} else {
		return false;
	}
}

// If range is all virtual collapse to start of virtual space
void SelectionRange::MinimizeVirtualSpace() {
	if (caret.Position() == anchor.Position()) {
		int virtualSpace = caret.VirtualSpace();
		if (virtualSpace > anchor.VirtualSpace())
			virtualSpace = anchor.VirtualSpace();
		caret.SetVirtualSpace(virtualSpace);
		anchor.SetVirtualSpace(virtualSpace);
	}
}

void Selection::Allocate() {
	if (nRanges >= allocated) {
		size_t allocateNew = (allocated + 1) * 2;
		SelectionRange *rangesNew = new SelectionRange[allocateNew];
		for (size_t r=0; r<Count(); r++)
			rangesNew[r] = ranges[r];
		delete []ranges;
		ranges = rangesNew;
		allocated = allocateNew;
	}
}

Selection::Selection() : ranges(0), allocated(0), nRanges(0), mainRange(0), moveExtends(false), selType(selStream) {
	AddSelection(SelectionPosition(0));
}

Selection::~Selection() {
	delete []ranges;
}

bool Selection::IsRectangular() const {
	return (selType == selRectangle) || (selType == selThin);
}

int Selection::MainCaret() const {
	return ranges[mainRange].caret.Position();
}

int Selection::MainAnchor() const {
	return ranges[mainRange].anchor.Position();
}

SelectionRange &Selection::Rectangular() {
	return rangeRectangular;
}

size_t Selection::Count() const {
	return nRanges;
}

size_t Selection::Main() const {
	return mainRange;
}

void Selection::SetMain(size_t r) {
	PLATFORM_ASSERT(r < nRanges);
	mainRange = r;
}

SelectionRange &Selection::Range(size_t r) {
	return ranges[r];
}

SelectionRange &Selection::RangeMain() {
	return ranges[mainRange];
}

void Selection::ClearVirtualSpace(size_t r) {
	ranges[r].ClearVirtualSpace();
}

bool Selection::MoveExtends() const {
	return moveExtends;
}

void Selection::SetMoveExtends(bool moveExtends_) {
	moveExtends = moveExtends_;
}

bool Selection::Empty() const {
	for (size_t i=0; i<nRanges; i++) {
		if (!ranges[i].Empty())
			return false;
	}
	return true;
}

SelectionPosition Selection::Last() const {
	SelectionPosition lastPosition;
	for (size_t i=0; i<nRanges; i++) {
		if (lastPosition < ranges[i].caret)
			lastPosition = ranges[i].caret;
		if (lastPosition < ranges[i].anchor)
			lastPosition = ranges[i].anchor;
	}
	return lastPosition;
}

int Selection::Length() const {
	int len = 0;
	for (size_t i=0; i<nRanges; i++) {
		len += ranges[i].Length();
	}
	return len;
}

void Selection::MovePositions(bool insertion, int startChange, int length) {
	for (size_t i=0; i<nRanges; i++) {
		ranges[i].caret.MoveForInsertDelete(insertion, startChange, length);
		ranges[i].anchor.MoveForInsertDelete(insertion, startChange, length);
	}
}

void Selection::TrimSelection(SelectionPosition startPos, SelectionPosition endPos) {
	for (size_t i=0; i<nRanges;) {
		if ((i != mainRange) && (ranges[i].Trim(startPos, endPos))) {
			// Trimmed to empty so remove
			for (size_t j=i;j<nRanges-1;j++) {
				ranges[j] = ranges[j+1];
				if (j == mainRange-1)
					mainRange--;
			}
			nRanges--;
		} else {
			i++;
		}
	}
}

void Selection::AddSelection(SelectionPosition spPos) {
	Allocate();
	TrimSelection(spPos, spPos);
	ranges[nRanges] = SelectionRange(spPos);
	mainRange = nRanges;
	nRanges++;
}

void Selection::AddSelection(SelectionPosition spStartPos, SelectionPosition spEndPos, bool anchorLeft) {
	Allocate();
	TrimSelection(spStartPos, spEndPos);
	ranges[nRanges].caret = anchorLeft ? spEndPos : spStartPos;
	ranges[nRanges].anchor = anchorLeft ? spStartPos : spEndPos;
	mainRange = nRanges;
	nRanges++;
}

bool Selection::CharacterInSelection(int posCharacter) const {
	for (size_t i=0; i<nRanges; i++) {
		if (ranges[i].ContainsCharacter(posCharacter))
			return true;
	}
	return false;
}

bool Selection::InSelectionForEOL(int pos) const {
	for (size_t i=0; i<nRanges; i++) {
		if (!ranges[i].Empty() && (pos > ranges[i].Start().Position()) && (pos <= ranges[i].End().Position()))
			return true;
	}
	return false;
}

int Selection::VirtualSpaceFor(int pos) const {
	int virtualSpace = 0;
	for (size_t i=0; i<nRanges; i++) {
		if ((ranges[i].caret.Position() == pos) && (virtualSpace < ranges[i].caret.VirtualSpace()))
			virtualSpace = ranges[i].caret.VirtualSpace();
		if ((ranges[i].anchor.Position() == pos) && (virtualSpace < ranges[i].anchor.VirtualSpace()))
			virtualSpace = ranges[i].anchor.VirtualSpace();
	}
	return virtualSpace;
}

void Selection::Clear() {
	nRanges = 1;
	mainRange = 0;
	selType = selStream;
	moveExtends = false;
	ranges[mainRange].Reset();
	rangeRectangular.Reset();
}

void Selection::EmptyRanges() {
	nRanges = 0;
	mainRange = 0;
}
