// Scintilla source code edit control
/** @file LineMarker.h
 ** Defines the look of a line marker in the margin .
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef LINEMARKER_H
#define LINEMARKER_H

class XPM {
	int height;
	int width;
	int nColours;
	char *data;
	char codeTransparent;
	char *codes;
	ColourPair *colours;
	void Init(const char * const *linesForm);
	ColourAllocated ColourFromCode(int ch);
	void FillRun(Surface *surface, int code, int startX, int y, int x);
public:
	XPM(const char *textForm);
	XPM(const char * const *linesForm);
	~XPM();
	// Similar to same named method in ViewStyle:
	void RefreshColourPalette(Palette &pal, bool want);
	// Decompose image into runs and use FillRectangle for each run:
	void Draw(Surface *surface, PRectangle &rc);
};

/**
 */
class LineMarker {
public:
	int markType;
	ColourPair fore;
	ColourPair back;
	XPM *pxpm;
	LineMarker() {
		markType = SC_MARK_CIRCLE;
		fore = ColourDesired(0,0,0);
		back = ColourDesired(0xff,0xff,0xff);
		pxpm = NULL;
	}
	~LineMarker() {
		delete pxpm;
	}
	void RefreshColourPalette(Palette &pal, bool want);
	void SetXPM(const char *textForm);
	void SetXPM(const char * const *linesForm);
	void Draw(Surface *surface, PRectangle &rc, Font &fontForCharacter);
};

#endif
