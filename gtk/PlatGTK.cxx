// Scintilla source code edit control
// PlatGTK.cxx - implementation of platform facilities on GTK+/Linux
// Copyright 1998-2002 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "Platform.h"

#include "Scintilla.h"
#include "ScintillaWidget.h"

/* Use fast way of getting char data on win32 to work around problems
   with gdk_string_extents. */
#ifdef G_OS_WIN32
#define FAST_WAY
#endif

#ifdef _MSC_VER
// Ignore unreferenced local functions in GTK+ headers
#pragma warning(disable: 4505)
#endif

static GdkFont *PFont(Font &f)  {
	return reinterpret_cast<GdkFont *>(f.GetID());
}

static GtkWidget *PWidget(WindowID id) {
	return reinterpret_cast<GtkWidget *>(id);
}

static GtkWidget *PWidget(Window &w) {
	return PWidget(w.GetID());
}

Point Point::FromLong(long lpoint) {
	return Point(
	           Platform::LowShortFromLong(lpoint),
	           Platform::HighShortFromLong(lpoint));
}

Palette::Palette() {
	used = 0;
	allowRealization = false;
	allocatedPalette = 0;
	allocatedLen = 0;
}

Palette::~Palette() {
	Release();
}

void Palette::Release() {
	used = 0;
	delete [](reinterpret_cast<GdkColor *>(allocatedPalette));
	allocatedPalette = 0;
	allocatedLen = 0;
}

// This method either adds a colour to the list of wanted colours (want==true)
// or retrieves the allocated colour back to the ColourPair.
// This is one method to make it easier to keep the code for wanting and retrieving in sync.
void Palette::WantFind(ColourPair &cp, bool want) {
	if (want) {
		for (int i = 0; i < used; i++) {
			if (entries[i].desired == cp.desired)
				return ;
		}

		if (used < numEntries) {
			entries[used].desired = cp.desired;
			entries[used].allocated.Set(cp.desired.AsLong());
			used++;
		}
	} else {
		for (int i = 0; i < used; i++) {
			if (entries[i].desired == cp.desired) {
				cp.allocated = entries[i].allocated;
				return ;
			}
		}
		cp.allocated.Set(cp.desired.AsLong());
	}
}

void Palette::Allocate(Window &w) {
	if (allocatedPalette) {
		gdk_colormap_free_colors(gtk_widget_get_colormap(PWidget(w)),
		        reinterpret_cast<GdkColor *>(allocatedPalette),
			allocatedLen);
		delete [](reinterpret_cast<GdkColor *>(allocatedPalette));
		allocatedPalette = 0;
		allocatedLen = 0;
	}
	GdkColor *paletteNew = new GdkColor[used];
	allocatedPalette = paletteNew;
	gboolean *successPalette = new gboolean[used];
	if (paletteNew) {
		allocatedLen = used;
		int iPal = 0;
		for (iPal = 0; iPal < used; iPal++) {
			paletteNew[iPal].red = entries[iPal].desired.GetRed() * (65535 / 255);
			paletteNew[iPal].green = entries[iPal].desired.GetGreen() * (65535 / 255);
			paletteNew[iPal].blue = entries[iPal].desired.GetBlue() * (65535 / 255);
			paletteNew[iPal].pixel = entries[iPal].desired.AsLong();
		}
		gdk_colormap_alloc_colors(gtk_widget_get_colormap(PWidget(w)),
		                          paletteNew, allocatedLen, FALSE, TRUE,
		                          successPalette);
		for (iPal = 0; iPal < used; iPal++) {
			entries[iPal].allocated.Set(paletteNew[iPal].pixel);
		}
	}
	delete []successPalette;
}

Font::Font() : id(0) {}

Font::~Font() {}

static const char *CharacterSetName(int characterSet) {
	switch (characterSet) {
	case SC_CHARSET_ANSI:
		return "iso8859-*";
	case SC_CHARSET_DEFAULT:
		return "iso8859-*";
	case SC_CHARSET_BALTIC:
		return "*-*";
	case SC_CHARSET_CHINESEBIG5:
		return "*-*";
	case SC_CHARSET_EASTEUROPE:
		return "*-2";
	case SC_CHARSET_GB2312:
		return "gb2312.1980-*";
	case SC_CHARSET_GREEK:
		return "*-7";
	case SC_CHARSET_HANGUL:
		return "ksc5601.1987-*";
	case SC_CHARSET_MAC:
		return "*-*";
	case SC_CHARSET_OEM:
		return "*-*";
	case SC_CHARSET_RUSSIAN:
		return "*-r";
	case SC_CHARSET_SHIFTJIS:
		return "jisx0208.1983-*";
	case SC_CHARSET_SYMBOL:
		return "*-*";
	case SC_CHARSET_TURKISH:
		return "*-9";
	case SC_CHARSET_JOHAB:
		return "*-*";
	case SC_CHARSET_HEBREW:
		return "*-8";
	case SC_CHARSET_ARABIC:
		return "*-6";
	case SC_CHARSET_VIETNAMESE:
		return "*-*";
	case SC_CHARSET_THAI:
		return "*-1";
	default:
		return "*-*";
	}
}

void Font::Create(const char *faceName, int characterSet,
                  int size, bool bold, bool italic) {
	Release();
	// If name of the font begins with a '-', assume, that it is
	// a full fontspec.
	if (faceName[0] == '-') {
		id = gdk_font_load(faceName);
		if (id)
			return ;
	}
	char fontspec[300];
	fontspec[0] = '\0';
	strcat(fontspec, "-*-");
	strcat(fontspec, faceName);
	if (bold)
		strcat(fontspec, "-bold");
	else
		strcat(fontspec, "-medium");
	if (italic)
		strcat(fontspec, "-i");
	else
		strcat(fontspec, "-r");
	strcat(fontspec, "-*-*-*");
	char sizePts[100];
	sprintf(sizePts, "-%0d", size * 10);
	strcat(fontspec, sizePts);
	strcat(fontspec, "-*-*-*-*-");
	strcat(fontspec, CharacterSetName(characterSet));
	id = gdk_font_load(fontspec);
	if (!id) {
		// Font not available so substitute a reasonable code font
		// iso8859 appears to only allow western characters.
		id = gdk_font_load("*-*-*-*-*-*-*-*-*-*-*-*-iso8859-*");
	}
}

void Font::Release() {
	if (id)
		gdk_font_unref(PFont(*this));
	id = 0;
}

class SurfaceImpl : public Surface {
	bool unicodeMode;
	GdkDrawable *drawable;
	GdkGC *gc;
	GdkPixmap *ppixmap;
	int x;
	int y;
	bool inited;
	bool createdGC;
public:
	SurfaceImpl();
	virtual ~SurfaceImpl();

	void Init();
	void Init(SurfaceID sid);
	void InitPixMap(int width, int height, Surface *surface_);

	void Release();
	bool Initialised();
	void PenColour(ColourAllocated fore);
	int LogPixelsY();
	int DeviceHeightFont(int points);
	void MoveTo(int x_, int y_);
	void LineTo(int x_, int y_);
	void Polygon(Point *pts, int npts, ColourAllocated fore, ColourAllocated back);
	void RectangleDraw(PRectangle rc, ColourAllocated fore, ColourAllocated back);
	void FillRectangle(PRectangle rc, ColourAllocated back);
	void FillRectangle(PRectangle rc, Surface &surfacePattern);
	void RoundedRectangle(PRectangle rc, ColourAllocated fore, ColourAllocated back);
	void Ellipse(PRectangle rc, ColourAllocated fore, ColourAllocated back);
	void Copy(PRectangle rc, Point from, Surface &surfaceSource);

	void DrawTextNoClip(PRectangle rc, Font &font_, int ybase, const char *s, int len, ColourAllocated fore, ColourAllocated back);
	void DrawTextClipped(PRectangle rc, Font &font_, int ybase, const char *s, int len, ColourAllocated fore, ColourAllocated back);
	void MeasureWidths(Font &font_, const char *s, int len, int *positions);
	int WidthText(Font &font_, const char *s, int len);
	int WidthChar(Font &font_, char ch);
	int Ascent(Font &font_);
	int Descent(Font &font_);
	int InternalLeading(Font &font_);
	int ExternalLeading(Font &font_);
	int Height(Font &font_);
	int AverageCharWidth(Font &font_);

	int SetPalette(Palette *pal, bool inBackGround);
	void SetClip(PRectangle rc);
	void FlushCachedState();

	void SetUnicodeMode(bool unicodeMode_);
};

SurfaceImpl::SurfaceImpl() : unicodeMode(false), drawable(0), gc(0), ppixmap(0),
x(0), y(0), inited(false), createdGC(false) {}

SurfaceImpl::~SurfaceImpl() {
	Release();
}

void SurfaceImpl::Release() {
	drawable = 0;
	if (createdGC) {
		createdGC = false;
		gdk_gc_unref(gc);
	}
	gc = 0;
	if (ppixmap)
		gdk_pixmap_unref(ppixmap);
	ppixmap = 0;
	x = 0;
	y = 0;
	inited = false;
	createdGC = false;
}

bool SurfaceImpl::Initialised() {
	return inited;
}

void SurfaceImpl::Init() {
	Release();
	inited = true;
}

void SurfaceImpl::Init(SurfaceID sid) {
	GdkDrawable *drawable_ = reinterpret_cast<GdkDrawable *>(sid);
	Release();
	drawable = drawable_;
	gc = gdk_gc_new(drawable_);
	//gdk_gc_set_line_attributes(gc, 1,
	//	GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_BEVEL);
	createdGC = true;
	inited = true;
}

void SurfaceImpl::InitPixMap(int width, int height, Surface *surface_) {
	Release();
	if (height > 0 && width > 0)
		ppixmap = gdk_pixmap_new(static_cast<SurfaceImpl *>(surface_)->drawable, width, height, -1);
	drawable = ppixmap;
	gc = gdk_gc_new(static_cast<SurfaceImpl *>(surface_)->drawable);
	//gdk_gc_set_line_attributes(gc, 1,
	//	GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_BEVEL);
	createdGC = true;
	inited = true;
}

void SurfaceImpl::PenColour(ColourAllocated fore) {
	if (gc) {
		GdkColor co;
		co.pixel = fore.AsLong();
		gdk_gc_set_foreground(gc, &co);
	}
}

int SurfaceImpl::LogPixelsY() {
	return 72;
}

int SurfaceImpl::DeviceHeightFont(int points) {
	int logPix = LogPixelsY();
	return (points * logPix + logPix / 2) / 72;
}

void SurfaceImpl::MoveTo(int x_, int y_) {
	x = x_;
	y = y_;
}

void SurfaceImpl::LineTo(int x_, int y_) {
	gdk_draw_line(drawable, gc,
	              x, y,
	              x_, y_);
	x = x_;
	y = y_;
}

void SurfaceImpl::Polygon(Point *pts, int npts, ColourAllocated fore,
                      ColourAllocated back) {
	GdkPoint gpts[20];
	if (npts < static_cast<int>((sizeof(gpts) / sizeof(gpts[0])))) {
		for (int i = 0;i < npts;i++) {
			gpts[i].x = pts[i].x;
			gpts[i].y = pts[i].y;
		}
		PenColour(back);
		gdk_draw_polygon(drawable, gc, 1, gpts, npts);
		PenColour(fore);
		gdk_draw_polygon(drawable, gc, 0, gpts, npts);
	}
}

void SurfaceImpl::RectangleDraw(PRectangle rc, ColourAllocated fore, ColourAllocated back) {
	if (gc && drawable) {
		PenColour(back);
		gdk_draw_rectangle(drawable, gc, 1,
		                   rc.left + 1, rc.top + 1,
		                   rc.right - rc.left - 2, rc.bottom - rc.top - 2);

		PenColour(fore);
		// The subtraction of 1 off the width and height here shouldn't be needed but
		// otherwise a different rectangle is drawn than would be done if the fill parameter == 1
		gdk_draw_rectangle(drawable, gc, 0,
		                   rc.left, rc.top,
		                   rc.right - rc.left - 1, rc.bottom - rc.top - 1);
	}
}

void SurfaceImpl::FillRectangle(PRectangle rc, ColourAllocated back) {
	PenColour(back);
	if (drawable && (rc.left < 32000)) {	// Protect against out of range
		gdk_draw_rectangle(drawable, gc, 1,
		                   rc.left, rc.top,
		                   rc.right - rc.left, rc.bottom - rc.top);
	}
}

void SurfaceImpl::FillRectangle(PRectangle rc, Surface &surfacePattern) {
	if (static_cast<SurfaceImpl &>(surfacePattern).drawable) {
		// Tile pattern over rectangle
		// Currently assumes 8x8 pattern
		int widthPat = 8;
		int heightPat = 8;
		for (int xTile = rc.left; xTile < rc.right; xTile += widthPat) {
			int widthx = (xTile + widthPat > rc.right) ? rc.right - xTile : widthPat;
			for (int yTile = rc.top; yTile < rc.bottom; yTile += heightPat) {
				int heighty = (yTile + heightPat > rc.bottom) ? rc.bottom - yTile : heightPat;
				gdk_draw_pixmap(drawable,
				                gc,
				                static_cast<SurfaceImpl &>(surfacePattern).drawable,
				                0, 0,
				                xTile, yTile,
				                widthx, heighty);
			}
		}
	} else {
		// Something is wrong so try to show anyway
		// Shows up black because colour not allocated
		FillRectangle(rc, ColourAllocated(0));
	}
}

void SurfaceImpl::RoundedRectangle(PRectangle rc, ColourAllocated fore, ColourAllocated back) {
	if (((rc.right - rc.left) > 4) && ((rc.bottom - rc.top) > 4)) {
		// Approximate a round rect with some cut off corners
		Point pts[] = {
		    Point(rc.left + 2, rc.top),
		    Point(rc.right - 2, rc.top),
		    Point(rc.right, rc.top + 2),
		    Point(rc.right, rc.bottom - 2),
		    Point(rc.right - 2, rc.bottom),
		    Point(rc.left + 2, rc.bottom),
		    Point(rc.left, rc.bottom - 2),
		    Point(rc.left, rc.top + 2),
		};
		Polygon(pts, sizeof(pts) / sizeof(pts[0]), fore, back);
	} else {
		RectangleDraw(rc, fore, back);
	}
}

void SurfaceImpl::Ellipse(PRectangle rc, ColourAllocated fore, ColourAllocated back) {
	PenColour(back);
	gdk_draw_arc(drawable, gc, 1,
	             rc.left + 1, rc.top + 1,
	             rc.right - rc.left - 2, rc.bottom - rc.top - 2,
	             0, 32767);

	// The subtraction of 1 here is similar to the case for RectangleDraw
	PenColour(fore);
	gdk_draw_arc(drawable, gc, 0,
	             rc.left, rc.top,
	             rc.right - rc.left - 1, rc.bottom - rc.top - 1,
	             0, 32767);
}

void SurfaceImpl::Copy(PRectangle rc, Point from, Surface &surfaceSource) {
	if (static_cast<SurfaceImpl &>(surfaceSource).drawable) {
		gdk_draw_pixmap(drawable,
		                gc,
		                static_cast<SurfaceImpl &>(surfaceSource).drawable,
		                from.x, from.y,
		                rc.left, rc.top,
		                rc.right - rc.left, rc.bottom - rc.top);
	}
}

void SurfaceImpl::DrawTextNoClip(PRectangle rc, Font &font_, int ybase, const char *s, int len,
                       ColourAllocated fore, ColourAllocated back) {
	FillRectangle(rc, back);
	PenColour(fore);
	if (gc && drawable)
		gdk_draw_text(drawable, PFont(font_), gc, rc.left, ybase, s, len);
}

// On GTK+, exactly same as DrawTextNoClip
void SurfaceImpl::DrawTextClipped(PRectangle rc, Font &font_, int ybase, const char *s, int len,
                              ColourAllocated fore, ColourAllocated back) {
	FillRectangle(rc, back);
	PenColour(fore);
	if (gc && drawable)
		gdk_draw_text(drawable, PFont(font_), gc, rc.left, ybase, s, len);
}

void SurfaceImpl::MeasureWidths(Font &font_, const char *s, int len, int *positions) {
	int totalWidth = 0;
	for (int i = 0;i < len;i++) {
		if (font_.GetID()) {
			int width = gdk_char_width(PFont(font_), s[i]);
			totalWidth += width;
		} else {
			totalWidth++;
		}
		positions[i] = totalWidth;
	}
}

int SurfaceImpl::WidthText(Font &font_, const char *s, int len) {
	if (font_.GetID())
		return gdk_text_width(PFont(font_), s, len);
	else
		return 1;
}

int SurfaceImpl::WidthChar(Font &font_, char ch) {
	if (font_.GetID())
		return gdk_char_width(PFont(font_), ch);
	else
		return 1;
}

// Three possible strategies for determining ascent and descent of font:
// 1) Call gdk_string_extents with string containing all letters, numbers and punctuation.
// 2) Use the ascent and descent fields of GdkFont.
// 3) Call gdk_string_extents with string as 1 but also including accented capitals.
// Smallest values given by 1 and largest by 3 with 2 in between.
// Techniques 1 and 2 sometimes chop off extreme portions of ascenders and
// descenders but are mostly OK except for accented characters like Å which are
// rarely used in code.

// This string contains a good range of characters to test for size.
const char largeSizeString[] = "ÂÃÅÄ `~!@#$%^&*()-_=+\\|[]{};:\"\'<,>.?/1234567890"
                               "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char sizeString[] = "`~!@#$%^&*()-_=+\\|[]{};:\"\'<,>.?/1234567890"
                          "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

int SurfaceImpl::Ascent(Font &font_) {
	if (!font_.GetID())
		return 1;
#ifdef FAST_WAY
	return PFont(font_)->ascent;
#else
	gint lbearing;
	gint rbearing;
	gint width;
	gint ascent;
	gint descent;

	gdk_string_extents(PFont(font_), sizeString,
	                   &lbearing, &rbearing, &width, &ascent, &descent);
	return ascent;
#endif
}

int SurfaceImpl::Descent(Font &font_) {
	if (!font_.GetID())
		return 1;
#ifdef FAST_WAY
	return PFont(font_)->descent;
#else
	gint lbearing;
	gint rbearing;
	gint width;
	gint ascent;
	gint descent;

	gdk_string_extents(PFont(font_), sizeString,
	                   &lbearing, &rbearing, &width, &ascent, &descent);
	return descent;
#endif
}

int SurfaceImpl::InternalLeading(Font &) {
	return 0;
}

int SurfaceImpl::ExternalLeading(Font &) {
	return 0;
}

int SurfaceImpl::Height(Font &font_) {
	return Ascent(font_) + Descent(font_);
}

int SurfaceImpl::AverageCharWidth(Font &font_) {
	if (font_.GetID())
		return gdk_char_width(PFont(font_), 'n');
	else
		return 1;
}

int SurfaceImpl::SetPalette(Palette *, bool) {
	// Handled in palette allocation for GTK so this does nothing
	return 0;
}

void SurfaceImpl::SetClip(PRectangle rc) {
	GdkRectangle area = {rc.left, rc.top,
	                     rc.right - rc.left, rc.bottom - rc.top};
	gdk_gc_set_clip_rectangle(gc, &area);
}

void SurfaceImpl::FlushCachedState() {}

void SurfaceImpl::SetUnicodeMode(bool unicodeMode_) {
	unicodeMode=unicodeMode_;
}

Surface *Surface::Allocate() {
	return new SurfaceImpl;
}

Window::~Window() {}

void Window::Destroy() {
	if (id)
		gtk_widget_destroy(GTK_WIDGET(id));
	id = 0;
}

bool Window::HasFocus() {
	return GTK_WIDGET_HAS_FOCUS(id);
}

PRectangle Window::GetPosition() {
	// Before any size allocated pretend its 1000 wide so not scrolled
	PRectangle rc(0, 0, 1000, 1000);
	if (id) {
		rc.left = PWidget(id)->allocation.x;
		rc.top = PWidget(id)->allocation.y;
		if (PWidget(id)->allocation.width > 20) {
			rc.right = rc.left + PWidget(id)->allocation.width;
			rc.bottom = rc.top + PWidget(id)->allocation.height;
		}
	}
	return rc;
}

void Window::SetPosition(PRectangle rc) {
#if 1
	//gtk_widget_set_uposition(id, rc.left, rc.top);
	GtkAllocation alloc;
	alloc.x = rc.left;
	alloc.y = rc.top;
	alloc.width = rc.Width();
	alloc.height = rc.Height();
	gtk_widget_size_allocate(PWidget(id), &alloc);
#else
	gtk_widget_set_uposition(id, rc.left, rc.top);
	gtk_widget_set_usize(id, rc.right - rc.left, rc.bottom - rc.top);
#endif
}

void Window::SetPositionRelative(PRectangle rc, Window relativeTo) {
	int ox = 0;
	int oy = 0;
	gdk_window_get_origin(PWidget(relativeTo.id)->window, &ox, &oy);

	gtk_widget_set_uposition(PWidget(id), rc.left + ox, rc.top + oy);
#if 0
	GtkAllocation alloc;
	alloc.x = rc.left + ox;
	alloc.y = rc.top + oy;
	alloc.width = rc.right - rc.left;
	alloc.height = rc.bottom - rc.top;
	gtk_widget_size_allocate(id, &alloc);
#endif
	gtk_widget_set_usize(PWidget(id), rc.right - rc.left, rc.bottom - rc.top);
}

PRectangle Window::GetClientPosition() {
	// On GTK+, the client position is the window position
	return GetPosition();
}

void Window::Show(bool show) {
	if (show)
		gtk_widget_show(PWidget(id));
}

void Window::InvalidateAll() {
	if (id) {
		gtk_widget_queue_draw(PWidget(id));
	}
}

void Window::InvalidateRectangle(PRectangle rc) {
	if (id) {
		gtk_widget_queue_draw_area(PWidget(id),
		                           rc.left, rc.top,
		                           rc.right - rc.left, rc.bottom - rc.top);
	}
}

void Window::SetFont(Font &) {
	// TODO
}

void Window::SetCursor(Cursor curs) {
	GdkCursor *gdkCurs;

	// We don't set the cursor to same value numerous times under gtk because
	// it stores the cursor in the window once it's set
	if (curs == cursorLast)
		return;
	cursorLast = curs;

	switch (curs) {
	case cursorText:
		gdkCurs = gdk_cursor_new(GDK_XTERM);
		break;
	case cursorArrow:
		gdkCurs = gdk_cursor_new(GDK_ARROW);
		break;
	case cursorUp:
		gdkCurs = gdk_cursor_new(GDK_CENTER_PTR);
		break;
	case cursorWait:
		gdkCurs = gdk_cursor_new(GDK_WATCH);
		break;
	case cursorReverseArrow:
		gdkCurs = gdk_cursor_new(GDK_TOP_LEFT_ARROW);
		break;
	default:
		gdkCurs = gdk_cursor_new(GDK_ARROW);
		cursorLast = cursorArrow;
		break;
	}

	gdk_window_set_cursor(PWidget(id)->window, gdkCurs);
	gdk_cursor_destroy(gdkCurs);
}

void Window::SetTitle(const char *s) {
	gtk_window_set_title(GTK_WINDOW(id), s);
}

ListBox::ListBox() : list(0), current(0), desiredVisibleRows(5), maxItemCharacters(0),
	doubleClickAction(NULL), doubleClickActionData(NULL) {}

ListBox::~ListBox() {}

static void SelectionAC(GtkWidget *, gint row, gint,
                        GdkEventButton *, gpointer p) {
	int *pi = reinterpret_cast<int *>(p);
	*pi = row;
}

static gboolean ButtonPress(GtkWidget *, GdkEventButton* ev, gpointer p) {
	ListBox* lb = reinterpret_cast<ListBox*>(p);
	if (ev->type == GDK_2BUTTON_PRESS && lb->doubleClickAction != NULL) {
		lb->doubleClickAction(lb->doubleClickActionData);
		return TRUE;
	}

	return FALSE;
}

void ListBox::Create(Window &, int) {
	id = gtk_window_new(GTK_WINDOW_POPUP);

	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_widget_show (frame);
	gtk_container_add(GTK_CONTAINER(GetID()), frame);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 0);

	scroller = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_set_border_width(GTK_CONTAINER(scroller), 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller),
	                               GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(frame), PWidget(scroller));
	gtk_widget_show(PWidget(scroller));

	list = gtk_clist_new(1);
	gtk_widget_show(PWidget(list));
	gtk_container_add(GTK_CONTAINER(PWidget(scroller)), PWidget(list));
	gtk_clist_set_column_auto_resize(GTK_CLIST(PWidget(list)), 0, TRUE);
	gtk_clist_set_selection_mode(GTK_CLIST(PWidget(list)), GTK_SELECTION_BROWSE);
	gtk_signal_connect(GTK_OBJECT(PWidget(list)), "select_row",
	                   GTK_SIGNAL_FUNC(SelectionAC), &current);
	gtk_signal_connect(GTK_OBJECT(PWidget(list)), "button_press_event",
	                   GTK_SIGNAL_FUNC(ButtonPress), this);
	gtk_clist_set_shadow_type(GTK_CLIST(PWidget(list)), GTK_SHADOW_NONE);

	gtk_widget_realize(PWidget(id));
}

void ListBox::SetFont(Font &scint_font) {
	GtkStyle* style;

	style = gtk_widget_get_style(GTK_WIDGET(PWidget(list)));
	if (!gdk_font_equal(style->font, PFont(scint_font))) {
		style = gtk_style_copy(style);
		gdk_font_unref(style->font);
		style->font = PFont(scint_font);
		gdk_font_ref(style->font);
		gtk_widget_set_style(GTK_WIDGET(PWidget(list)), style);
		gtk_style_unref(style);
	}
}

void ListBox::SetAverageCharWidth(int width) {
	aveCharWidth = width;
}

void ListBox::SetVisibleRows(int rows) {
	desiredVisibleRows = rows;
}

PRectangle ListBox::GetDesiredRect() {
	// Before any size allocated pretend its 100 wide so not scrolled
	PRectangle rc(0, 0, 100, 100);
	if (id) {
		int rows = Length();
		if ((rows == 0) || (rows > desiredVisibleRows))
			rows = desiredVisibleRows;

		GtkRequisition req;
		int height;

		// First calculate height of the clist for our desired visible row count otherwise it tries to expand to the total # of rows
		height = (rows * GTK_CLIST(list)->row_height
		          + rows + 1
		          + 2 * (PWidget(list)->style->klass->ythickness
		                 + GTK_CONTAINER(PWidget(list))->border_width));
		gtk_widget_set_usize(GTK_WIDGET(PWidget(list)), -1, height);

		// Get the size of the scroller because we set usize on the window
		gtk_widget_size_request(GTK_WIDGET(scroller), &req);
		rc.right = req.width;
		rc.bottom = req.height;

		gtk_widget_set_usize(GTK_WIDGET(list), -1, -1);
		int width = maxItemCharacters;
		if (width < 12)
			width = 12;
		rc.right = width * (aveCharWidth + aveCharWidth / 3);
		if (Length() > rows)
			rc.right = rc.right + 16;
	}
	return rc;

}

void ListBox::Clear() {
	gtk_clist_clear(GTK_CLIST(list));
	maxItemCharacters = 0;
}

void ListBox::Append(char *s) {
	char *szs[] = { s, 0};
	gtk_clist_append(GTK_CLIST(list), szs);
	size_t len = strlen(s);
	if (maxItemCharacters < len)
		maxItemCharacters = len;
}

int ListBox::Length() {
	if (id)
		return GTK_CLIST(list)->rows;
	return 0;
}

void ListBox::Select(int n) {
	gtk_clist_select_row(GTK_CLIST(list), n, 0);
	gtk_clist_moveto(GTK_CLIST(list), n, 0, 0.5, 0.5);
}

int ListBox::GetSelection() {
	return current;
}

int ListBox::Find(const char *prefix) {
	int count = Length();
	for (int i = 0; i < count; i++) {
		char *s = 0;
		gtk_clist_get_text(GTK_CLIST(list), i, 0, &s);
		if (s && (0 == strncmp(prefix, s, strlen(prefix)))) {
			return i;
		}
	}
	return - 1;
}

void ListBox::GetValue(int n, char *value, int len) {
	char *text = 0;
	gtk_clist_get_text(GTK_CLIST(list), n, 0, &text);
	if (text && len > 0) {
		strncpy(value, text, len);
		value[len - 1] = '\0';
	} else {
		value[0] = '\0';
	}
}

void ListBox::Sort() {
	gtk_clist_sort(GTK_CLIST(list));
}

Menu::Menu() : id(0) {}

void Menu::CreatePopUp() {
	Destroy();
	id = gtk_item_factory_new(GTK_TYPE_MENU, "<main>", NULL);
}

void Menu::Destroy() {
	if (id)
		gtk_object_unref(GTK_OBJECT(id));
	id = 0;
}

void Menu::Show(Point pt, Window &) {
	gtk_item_factory_popup(reinterpret_cast<GtkItemFactory *>(id), pt.x - 4, pt.y, 3, 0);
}

ElapsedTime::ElapsedTime() {
	GTimeVal curTime;
	g_get_current_time(&curTime);
	bigBit = curTime.tv_sec;
	littleBit = curTime.tv_usec;
}

double ElapsedTime::Duration(bool reset) {
	GTimeVal curTime;
	g_get_current_time(&curTime);
	long endBigBit = curTime.tv_sec;
	long endLittleBit = curTime.tv_usec;
	double result = 1000000.0 * (endBigBit - bigBit);
	result += endLittleBit - littleBit;
	result /= 1000000.0;
	if (reset) {
		bigBit = endBigBit;
		littleBit = endLittleBit;
	}
	return result;
}

ColourDesired Platform::Chrome() {
	return ColourDesired(0xe0, 0xe0, 0xe0);
}

ColourDesired Platform::ChromeHighlight() {
	return ColourDesired(0xff, 0xff, 0xff);
}

const char *Platform::DefaultFont() {
#ifdef G_OS_WIN32
	return "Lucida Console";
#else
	return "lucidatypewriter";
#endif
}

int Platform::DefaultFontSize() {
#ifdef G_OS_WIN32
	return 10;
#else
	return 12;
#endif
}

unsigned int Platform::DoubleClickTime() {
	return 500; 	// Half a second
}

void Platform::DebugDisplay(const char *s) {
	printf("%s", s);
}

bool Platform::IsKeyDown(int) {
	// TODO: discover state of keys in GTK+/X
	return false;
}

long Platform::SendScintilla(
    WindowID w, unsigned int msg, unsigned long wParam, long lParam) {
	return scintilla_send_message(SCINTILLA(w), msg, wParam, lParam);
}

bool Platform::IsDBCSLeadByte(int /*codePage*/, char /*ch*/) {
	return false;
}

// These are utility functions not really tied to a platform

int Platform::Minimum(int a, int b) {
	if (a < b)
		return a;
	else
		return b;
}

int Platform::Maximum(int a, int b) {
	if (a > b)
		return a;
	else
		return b;
}

//#define TRACE

#ifdef TRACE
void Platform::DebugPrintf(const char *format, ...) {
	char buffer[2000];
	va_list pArguments;
	va_start(pArguments, format);
	vsprintf(buffer, format, pArguments);
	va_end(pArguments);
	Platform::DebugDisplay(buffer);
}
#else
void Platform::DebugPrintf(const char *, ...) {}

#endif

// Not supported for GTK+
static bool assertionPopUps = true;

bool Platform::ShowAssertionPopUps(bool assertionPopUps_) {
	bool ret = assertionPopUps;
	assertionPopUps = assertionPopUps_;
	return ret;
}

void Platform::Assert(const char *c, const char *file, int line) {
	char buffer[2000];
	sprintf(buffer, "Assertion [%s] failed at %s %d", c, file, line);
	strcat(buffer, "\r\n");
	Platform::DebugDisplay(buffer);
	abort();
}

int Platform::Clamp(int val, int minVal, int maxVal) {
	if (val > maxVal)
		val = maxVal;
	if (val < minVal)
		val = minVal;
	return val;
}
