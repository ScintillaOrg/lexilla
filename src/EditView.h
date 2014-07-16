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
	void LayoutLine(const EditModel &model, int line, Surface *surface, const ViewStyle &vstyle,
		LineLayout *ll, int width = LineLayout::wrapWidthInfinite);

	Point LocationFromPosition(Surface *surface, const EditModel &model, SelectionPosition pos, int topLine, const ViewStyle &vs);
	SelectionPosition SPositionFromLocation(Surface *surface, const EditModel &model, Point pt, bool canReturnInvalid,
		bool charPosition, bool virtualSpace, const ViewStyle &vs);
	SelectionPosition SPositionFromLineX(Surface *surface, const EditModel &model, int lineDoc, int x, const ViewStyle &vs);
	int DisplayFromPosition(Surface *surface, const EditModel &model, int pos, const ViewStyle &vs);
	int StartEndDisplayLine(Surface *surface, const EditModel &model, int pos, bool start, const ViewStyle &vs);

	void DrawIndentGuide(Surface *surface, int lineVisible, int lineHeight, int start, PRectangle rcSegment, bool highlight);
	void DrawEOL(Surface *surface, const EditModel &model, const ViewStyle &vsDraw, const LineLayout *ll, PRectangle rcLine,
		int line, int lineEnd, int xStart, int subLine, XYACCUMULATOR subLineStart,
		ColourOptional background);
	void DrawAnnotation(Surface *surface, const EditModel &model, const ViewStyle &vsDraw, const LineLayout *ll,
		int line, int xStart, PRectangle rcLine, int subLine);
	void DrawCarets(Surface *surface, const EditModel &model, const ViewStyle &vsDraw, const LineLayout *ll, int line,
		int xStart, PRectangle rcLine, int subLine) const;
	void DrawBackground(Surface *surface, const EditModel &model, const ViewStyle &vsDraw, const LineLayout *ll, PRectangle rcLine,
		Range lineRange, int posLineStart, int xStart,
		int subLine, ColourOptional background);
	void DrawForeground(Surface *surface, const EditModel &model, const ViewStyle &vsDraw, const LineLayout *ll, int lineVisible,
		PRectangle rcLine, Range lineRange, int posLineStart, int xStart,
		int subLine, ColourOptional background);
	void DrawIndentGuidesOverEmpty(Surface *surface, const EditModel &model, const ViewStyle &vsDraw, const LineLayout *ll,
		int line, int lineVisible, PRectangle rcLine, int xStart, int subLine);
	void DrawLine(Surface *surface, const EditModel &model, const ViewStyle &vsDraw, const LineLayout *ll, int line, 
		int lineVisible, int xStart, PRectangle rcLine, int subLine);
	void PaintText(Surface *surfaceWindow, const EditModel &model, PRectangle rcArea, PRectangle rcClient,
		const ViewStyle &vsDraw);
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
