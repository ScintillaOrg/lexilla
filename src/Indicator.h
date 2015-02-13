// Scintilla source code edit control
/** @file Indicator.h
 ** Defines the style of indicators which are text decorations such as underlining.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef INDICATOR_H
#define INDICATOR_H

#ifdef SCI_NAMESPACE
namespace Scintilla {
#endif

struct StyleAndColour {
	int style;
	ColourDesired fore;
	StyleAndColour() : style(INDIC_PLAIN), fore(0, 0, 0) {
	}
	StyleAndColour(int style_, ColourDesired fore_ = ColourDesired(0, 0, 0)) : style(style_), fore(fore_) {
	}
	bool operator==(const StyleAndColour &other) const {
		return (style == other.style) && (fore == other.fore);
	}
};

/**
 */
class Indicator {
public:
	enum DrawState { drawNormal, drawHover };
	StyleAndColour sacNormal;
	StyleAndColour sacHover;
	bool under;
	int fillAlpha;
	int outlineAlpha;
	Indicator() : under(false), fillAlpha(30), outlineAlpha(50) {
	}
	Indicator(int style_, ColourDesired fore_=ColourDesired(0,0,0), bool under_=false, int fillAlpha_=30, int outlineAlpha_=50) :
		sacNormal(style_, fore_), sacHover(style_, fore_), under(under_), fillAlpha(fillAlpha_), outlineAlpha(outlineAlpha_) {
	}
	void Draw(Surface *surface, const PRectangle &rc, const PRectangle &rcLine, DrawState drawState) const;
	bool IsDynamic() const {
		return !(sacNormal == sacHover);
	}
};

#ifdef SCI_NAMESPACE
}
#endif

#endif
