// Scintilla source code edit control
/** @file LineMarker.cxx
 ** Defines the look of a line marker in the margin .
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>

#include "Platform.h"

#include "Scintilla.h"
#include "LineMarker.h"

static const char *NextField(const char *s) {
	while (*s && *s != ' ') {
		s++;
	}
	while (*s && *s == ' ') {
		s++;
	}
	return s;
}

void XPM::Init(const char * const *linesForm) {
	height = 1;
	width = 1;
	nColours = 1;
	data = NULL;
	codeTransparent = ' ';
	codes = NULL;
	colours = NULL;
	if (!linesForm)
		return;

	const char *line0 = linesForm[0];
	width = atoi(line0);
	line0 = NextField(line0);
	height = atoi(line0);
	line0 = NextField(line0);
	nColours = atoi(line0);
	data = new char[height * width];
	codes = new char[nColours];
	colours = new ColourPair[nColours];

	for (int c=0; c<nColours; c++) {
		const char *colourDef = linesForm[c+1];
		codes[c] = colourDef[0];
		colourDef += 4;
		if (*colourDef == '#') {
			colours[c].desired.Set(colourDef);
		} else {
			colours[c].desired = ColourDesired(0xff, 0xff, 0xff);
			codeTransparent = codes[c];
		}
	}
	int datapos = 0;
	for (int l=0; l<height; l++) {
		for (int x=0; x<width; x++) {
			data[datapos++] = linesForm[1+nColours+l][x];
		}
	}
}

ColourAllocated XPM::ColourFromCode(int ch) {
	for (int i=0;i<nColours;i++) {
		if (codes[i] == ch) {
			return colours[i].allocated;
		}
	}
	return colours[0].allocated;
}

void XPM::FillRun(Surface *surface, int code, int startX, int y, int x) {
	if (code != codeTransparent) {
		PRectangle rc(startX, y, x, y+1);
		surface->FillRectangle(rc, ColourFromCode(code));
	}
}

XPM::XPM(const char *textForm) {
	int countQuotes = 0;
	for (int i=0; textForm[i]; i++) {
		if (textForm[i] == '\"') {
			countQuotes++;
		}
	}
	const char **linesForm = new const char *[countQuotes/2];
	countQuotes = 0;
	if (linesForm) {
		for (int j=0; textForm[j]; j++) {
			if (textForm[j] == '\"') {
				if ((countQuotes & 1) == 0) {
					linesForm[countQuotes / 2] = textForm + j + 1;
				}
				countQuotes++;
			}
		}
	}
	Init(linesForm);
	delete []linesForm;
}

XPM::XPM(const char * const *linesForm) {
	Init(linesForm);
}

XPM::~XPM() {
	delete []data;
	delete []codes;
	delete []colours;
}

void XPM::RefreshColourPalette(Palette &pal, bool want) {
	if (!data || !codes || !colours) {
		return;
	}
	for (int i=0;i<nColours;i++) {
		pal.WantFind(colours[i], want);
	}
}

void XPM::Draw(Surface *surface, PRectangle &rc) {
	if (!data || !codes || !colours) {
		return;
	}
	// Centre the pixmap
	int startY = rc.top + (rc.Height() - height) / 2;
	int startX = rc.left + (rc.Width() - width) / 2;
	for (int y=0;y<height;y++) {
		int prevCode = 0;
		int xStartRun = 0;
		for (int x=0; x<width; x++) {
			int code = data[y*width + x];
			if (code != prevCode) {
				FillRun(surface, prevCode, startX + xStartRun, startY + y, startX + x);
				xStartRun = x;
				prevCode = code;
			}
		}
		FillRun(surface, prevCode, startX + xStartRun, startY + y, startX + width);
	}
}

void LineMarker::RefreshColourPalette(Palette &pal, bool want) {
	pal.WantFind(fore, want);
	pal.WantFind(back, want);
	if (pxpm) {
		pxpm->RefreshColourPalette(pal, want);
	}
}

void LineMarker::SetXPM(const char *textForm) {
	delete pxpm;
	pxpm = new XPM(textForm);
	markType = SC_MARK_PIXMAP;
}

void LineMarker::SetXPM(const char * const *linesForm) {
	delete pxpm;
	pxpm = new XPM(linesForm);
	markType = SC_MARK_PIXMAP;
}

static void DrawBox(Surface *surface, int centreX, int centreY, int armSize, ColourAllocated fore, ColourAllocated back) {
	PRectangle rc;
	rc.left = centreX - armSize;
	rc.top = centreY - armSize;
	rc.right = centreX + armSize + 1;
	rc.bottom = centreY + armSize + 1;
	surface->RectangleDraw(rc, back, fore);
}

static void DrawCircle(Surface *surface, int centreX, int centreY, int armSize, ColourAllocated fore, ColourAllocated back) {
	PRectangle rcCircle;
	rcCircle.left = centreX - armSize;
	rcCircle.top = centreY - armSize;
	rcCircle.right = centreX + armSize + 1;
	rcCircle.bottom = centreY + armSize + 1;
	surface->Ellipse(rcCircle, back, fore);
}

static void DrawPlus(Surface *surface, int centreX, int centreY, int armSize, ColourAllocated fore) {
	PRectangle rcV(centreX, centreY - armSize + 2, centreX + 1, centreY + armSize - 2 + 1);
	surface->FillRectangle(rcV, fore);
	PRectangle rcH(centreX - armSize + 2, centreY, centreX + armSize - 2 + 1, centreY+1);
	surface->FillRectangle(rcH, fore);
}

static void DrawMinus(Surface *surface, int centreX, int centreY, int armSize, ColourAllocated fore) {
	PRectangle rcH(centreX - armSize + 2, centreY, centreX + armSize - 2 + 1, centreY+1);
	surface->FillRectangle(rcH, fore);
}

void LineMarker::Draw(Surface *surface, PRectangle &rcWhole, Font &fontForCharacter) {
	if ((markType == SC_MARK_PIXMAP) && (pxpm)) {
		pxpm->Draw(surface, rcWhole);
		return;
	}
	// Restrict most shapes a bit
	PRectangle rc = rcWhole;
	rc.top++;
	rc.bottom--;
	int minDim = Platform::Minimum(rc.Width(), rc.Height());
	minDim--;	// Ensure does not go beyond edge
	int centreX = (rc.right + rc.left) / 2;
	int centreY = (rc.bottom + rc.top) / 2;
	int dimOn2 = minDim / 2;
	int dimOn4 = minDim / 4;
	int blobSize = dimOn2-1;
	int armSize = dimOn2-2;
	if (rc.Width() > (rc.Height() * 2)) {
		// Wide column is line number so move to left to try to avoid overlapping number
		centreX = rc.left + dimOn2 + 1;
	}
	if (markType == SC_MARK_ROUNDRECT) {
		PRectangle rcRounded = rc;
		rcRounded.left = rc.left + 1;
		rcRounded.right = rc.right - 1;
		surface->RoundedRectangle(rcRounded, fore.allocated, back.allocated);
	} else if (markType == SC_MARK_CIRCLE) {
		PRectangle rcCircle;
		rcCircle.left = centreX - dimOn2;
		rcCircle.top = centreY - dimOn2;
		rcCircle.right = centreX + dimOn2;
		rcCircle.bottom = centreY + dimOn2;
		surface->Ellipse(rcCircle, fore.allocated, back.allocated);
	} else if (markType == SC_MARK_ARROW) {
		Point pts[] = {
    		Point(centreX - dimOn4, centreY - dimOn2),
    		Point(centreX - dimOn4, centreY + dimOn2),
    		Point(centreX + dimOn2 - dimOn4, centreY),
		};
		surface->Polygon(pts, sizeof(pts) / sizeof(pts[0]),
                 		fore.allocated, back.allocated);

	} else if (markType == SC_MARK_ARROWDOWN) {
		Point pts[] = {
    		Point(centreX - dimOn2, centreY - dimOn4),
    		Point(centreX + dimOn2, centreY - dimOn4),
    		Point(centreX, centreY + dimOn2 - dimOn4),
		};
		surface->Polygon(pts, sizeof(pts) / sizeof(pts[0]),
                 		fore.allocated, back.allocated);

	} else if (markType == SC_MARK_PLUS) {
		Point pts[] = {
    		Point(centreX - armSize, centreY - 1),
    		Point(centreX - 1, centreY - 1),
    		Point(centreX - 1, centreY - armSize),
    		Point(centreX + 1, centreY - armSize),
    		Point(centreX + 1, centreY - 1),
    		Point(centreX + armSize, centreY -1),
    		Point(centreX + armSize, centreY +1),
    		Point(centreX + 1, centreY + 1),
    		Point(centreX + 1, centreY + armSize),
    		Point(centreX - 1, centreY + armSize),
    		Point(centreX - 1, centreY + 1),
    		Point(centreX - armSize, centreY + 1),
		};
		surface->Polygon(pts, sizeof(pts) / sizeof(pts[0]),
                 		fore.allocated, back.allocated);

	} else if (markType == SC_MARK_MINUS) {
		Point pts[] = {
    		Point(centreX - armSize, centreY - 1),
    		Point(centreX + armSize, centreY -1),
    		Point(centreX + armSize, centreY +1),
    		Point(centreX - armSize, centreY + 1),
		};
		surface->Polygon(pts, sizeof(pts) / sizeof(pts[0]),
                 		fore.allocated, back.allocated);

	} else if (markType == SC_MARK_SMALLRECT) {
		PRectangle rcSmall;
		rcSmall.left = rc.left + 1;
		rcSmall.top = rc.top + 2;
		rcSmall.right = rc.right - 1;
		rcSmall.bottom = rc.bottom - 2;
		surface->RectangleDraw(rcSmall, fore.allocated, back.allocated);

	} else if (markType == SC_MARK_EMPTY || markType == SC_MARK_BACKGROUND) {
		// An invisible marker so don't draw anything

	} else if (markType == SC_MARK_VLINE) {
		surface->PenColour(back.allocated);
		surface->MoveTo(centreX, rcWhole.top);
		surface->LineTo(centreX, rcWhole.bottom);

	} else if (markType == SC_MARK_LCORNER) {
		surface->PenColour(back.allocated);
		surface->MoveTo(centreX, rcWhole.top);
		surface->LineTo(centreX, rc.top + dimOn2);
		surface->LineTo(rc.right - 2, rc.top + dimOn2);

	} else if (markType == SC_MARK_TCORNER) {
		surface->PenColour(back.allocated);
		surface->MoveTo(centreX, rcWhole.top);
		surface->LineTo(centreX, rcWhole.bottom);
		surface->MoveTo(centreX, rc.top + dimOn2);
		surface->LineTo(rc.right - 2, rc.top + dimOn2);

	} else if (markType == SC_MARK_LCORNERCURVE) {
		surface->PenColour(back.allocated);
		surface->MoveTo(centreX, rcWhole.top);
		surface->LineTo(centreX, rc.top + dimOn2-3);
		surface->LineTo(centreX+3, rc.top + dimOn2);
		surface->LineTo(rc.right - 1, rc.top + dimOn2);

	} else if (markType == SC_MARK_TCORNERCURVE) {
		surface->PenColour(back.allocated);
		surface->MoveTo(centreX, rcWhole.top);
		surface->LineTo(centreX, rcWhole.bottom);

		surface->MoveTo(centreX, rc.top + dimOn2-3);
		surface->LineTo(centreX+3, rc.top + dimOn2);
		surface->LineTo(rc.right - 1, rc.top + dimOn2);

	} else if (markType == SC_MARK_BOXPLUS) {
		surface->PenColour(back.allocated);
		DrawBox(surface, centreX, centreY, blobSize, fore.allocated, back.allocated);
		DrawPlus(surface, centreX, centreY, blobSize, back.allocated);

	} else if (markType == SC_MARK_BOXPLUSCONNECTED) {
		surface->PenColour(back.allocated);
		DrawBox(surface, centreX, centreY, blobSize, fore.allocated, back.allocated);
		DrawPlus(surface, centreX, centreY, blobSize, back.allocated);

		surface->MoveTo(centreX, centreY + blobSize);
		surface->LineTo(centreX, rcWhole.bottom);

		surface->MoveTo(centreX, rcWhole.top);
		surface->LineTo(centreX, centreY - blobSize);

	} else if (markType == SC_MARK_BOXMINUS) {
		surface->PenColour(back.allocated);
		DrawBox(surface, centreX, centreY, blobSize, fore.allocated, back.allocated);
		DrawMinus(surface, centreX, centreY, blobSize, back.allocated);

		surface->MoveTo(centreX, centreY + blobSize);
		surface->LineTo(centreX, rcWhole.bottom);

	} else if (markType == SC_MARK_BOXMINUSCONNECTED) {
		surface->PenColour(back.allocated);
		DrawBox(surface, centreX, centreY, blobSize, fore.allocated, back.allocated);
		DrawMinus(surface, centreX, centreY, blobSize, back.allocated);

		surface->MoveTo(centreX, centreY + blobSize);
		surface->LineTo(centreX, rcWhole.bottom);

		surface->MoveTo(centreX, rcWhole.top);
		surface->LineTo(centreX, centreY - blobSize);

	} else if (markType == SC_MARK_CIRCLEPLUS) {
		DrawCircle(surface, centreX, centreY, blobSize, fore.allocated, back.allocated);
		surface->PenColour(back.allocated);
		DrawPlus(surface, centreX, centreY, blobSize, back.allocated);

	} else if (markType == SC_MARK_CIRCLEPLUSCONNECTED) {
		DrawCircle(surface, centreX, centreY, blobSize, fore.allocated, back.allocated);
		surface->PenColour(back.allocated);
		DrawPlus(surface, centreX, centreY, blobSize, back.allocated);

		surface->MoveTo(centreX, centreY + blobSize);
		surface->LineTo(centreX, rcWhole.bottom);

		surface->MoveTo(centreX, rcWhole.top);
		surface->LineTo(centreX, centreY - blobSize);

	} else if (markType == SC_MARK_CIRCLEMINUS) {
		DrawCircle(surface, centreX, centreY, blobSize, fore.allocated, back.allocated);
		surface->PenColour(back.allocated);
		DrawMinus(surface, centreX, centreY, blobSize, back.allocated);

		surface->MoveTo(centreX, centreY + blobSize);
		surface->LineTo(centreX, rcWhole.bottom);

	} else if (markType == SC_MARK_CIRCLEMINUSCONNECTED) {
		DrawCircle(surface, centreX, centreY, blobSize, fore.allocated, back.allocated);
		surface->PenColour(back.allocated);
		DrawMinus(surface, centreX, centreY, blobSize, back.allocated);

		surface->MoveTo(centreX, centreY + blobSize);
		surface->LineTo(centreX, rcWhole.bottom);

		surface->MoveTo(centreX, rcWhole.top);
		surface->LineTo(centreX, centreY - blobSize);

	} else if (markType >= SC_MARK_CHARACTER) {
		char character[1];
		character[0] = static_cast<char>(markType - SC_MARK_CHARACTER);
		int width = surface->WidthText(fontForCharacter, character, 1);
		rc.left += (rc.Width() - width) / 2;
		rc.right = rc.left + width;
		surface->DrawTextClipped(rc, fontForCharacter, rc.bottom - 2,
			character, 1, fore.allocated, back.allocated);

	} else if (markType == SC_MARK_DOTDOTDOT) {
		int right = centreX - 6;
		for (int b=0; b<3; b++) {
			PRectangle rcBlob(right, rc.bottom - 4, right + 2, rc.bottom-2);
			surface->FillRectangle(rcBlob, fore.allocated);
			right += 5;
		}
	} else if (markType == SC_MARK_ARROWS) {
		surface->PenColour(fore.allocated);
		int right = centreX - 2;
		for (int b=0; b<3; b++) {
			surface->MoveTo(right - 4, centreY - 4);
			surface->LineTo(right, centreY);
			surface->LineTo(right - 5, centreY + 5);
			right += 4;
		}
	} else { // SC_MARK_SHORTARROW
		Point pts[] = {
			Point(centreX, centreY + dimOn2),
			Point(centreX + dimOn2, centreY),
			Point(centreX, centreY - dimOn2),
			Point(centreX, centreY - dimOn4),
			Point(centreX - dimOn4, centreY - dimOn4),
			Point(centreX - dimOn4, centreY + dimOn4),
			Point(centreX, centreY + dimOn4),
			Point(centreX, centreY + dimOn2),
		};
		surface->Polygon(pts, sizeof(pts) / sizeof(pts[0]),
				fore.allocated, back.allocated);
	}
}
