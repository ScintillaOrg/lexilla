// Scintilla source code edit control
/** @file EditView.h
 ** Defines the appearance of the main text area of the editor window.
 **/
// Copyright 1998-2014 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef EDITVIEW_H
#define EDITVIEW_H

#ifdef SCI_NAMESPACE
namespace Scintilla {
#endif

struct PrintParameters {
	int magnification;
	int colourMode;
	WrapMode wrapState;
	PrintParameters();
};

/**
* EditView draws the main text area.
*/
class EditView {
public:
	PrintParameters printParameters;

	bool hideSelection;
	bool drawOverstrikeCaret;

	/** In bufferedDraw mode, graphics operations are drawn to a pixmap and then copied to
	* the screen. This avoids flashing but is about 30% slower. */
	bool bufferedDraw;
	/** In twoPhaseDraw mode, drawing is performed in two phases, first the background
	* and then the foreground. This avoids chopping off characters that overlap the next run. */
	bool twoPhaseDraw;

	int lineWidthMaxSeen;

	bool additionalCaretsBlink;
	bool additionalCaretsVisible;

	Surface *pixmapLine;
	Surface *pixmapIndentGuide;
	Surface *pixmapIndentGuideHighlight;

	LineLayoutCache llc;
	PositionCache posCache;

	EditView();

	void DropGraphics(bool freeObjects);
	void AllocateGraphics(const ViewStyle &vsDraw);
	void RefreshPixMaps(Surface *surfaceWindow, WindowID wid, const ViewStyle &vsDraw);

	LineLayout *RetrieveLineLayout(int lineNumber, const EditModel &model);
	void LayoutLine(int line, Surface *surface, const ViewStyle &vstyle, LineLayout *ll,
		const EditModel &model, int width = LineLayout::wrapWidthInfinite);

	Point LocationFromPosition(Surface *surface, SelectionPosition pos, int topLine, const EditModel &model, const ViewStyle &vs);
	SelectionPosition SPositionFromLocation(Surface *surface, Point pt, bool canReturnInvalid, bool charPosition, bool virtualSpace,
		const EditModel &model, const ViewStyle &vs);
	SelectionPosition SPositionFromLineX(Surface *surface, int lineDoc, int x, const EditModel &model, const ViewStyle &vs);
	int DisplayFromPosition(Surface *surface, int pos, const EditModel &model, const ViewStyle &vs);
	int StartEndDisplayLine(Surface *surface, int pos, bool start, const EditModel &model, const ViewStyle &vs);

	void DrawIndentGuide(Surface *surface, int lineVisible, int lineHeight, int start, PRectangle rcSegment, bool highlight);
	void DrawEOL(Surface *surface, const ViewStyle &vsDraw, PRectangle rcLine, LineLayout *ll,
		int line, int lineEnd, int xStart, int subLine, XYACCUMULATOR subLineStart,
		ColourOptional background, const EditModel &model);
	void DrawAnnotation(Surface *surface, const ViewStyle &vsDraw, int line, int xStart,
		PRectangle rcLine, LineLayout *ll, int subLine, const EditModel &model);
	void DrawCarets(Surface *surface, const ViewStyle &vsDraw, int line, int xStart,
		PRectangle rcLine, LineLayout *ll, int subLine, const EditModel &model) const;
	void DrawLine(Surface *surface, const ViewStyle &vsDraw, int line, int lineVisible, int xStart,
		PRectangle rcLine, LineLayout *ll, int subLine, const EditModel &model);
	void PaintText(Surface *surfaceWindow, PRectangle rcArea, PRectangle rcClient,
		const EditModel &model, const ViewStyle &vsDraw);
	long FormatRange(bool draw, Sci_RangeToFormat *pfr, Surface *surface, Surface *surfaceMeasure,
		const EditModel &model, const ViewStyle &vs);
};

/**
* Convenience class to ensure LineLayout objects are always disposed.
*/
class AutoLineLayout {
	LineLayoutCache &llc;
	LineLayout *ll;
	AutoLineLayout &operator=(const AutoLineLayout &);
public:
	AutoLineLayout(LineLayoutCache &llc_, LineLayout *ll_) : llc(llc_), ll(ll_) {}
	~AutoLineLayout() {
		llc.Dispose(ll);
		ll = 0;
	}
	LineLayout *operator->() const {
		return ll;
	}
	operator LineLayout *() const {
		return ll;
	}
	void Set(LineLayout *ll_) {
		llc.Dispose(ll);
		ll = ll_;
	}
};

#ifdef SCI_NAMESPACE
}
#endif

#endif
