// Scintilla source code edit control
// PlatGTK.cxx - implementation of platform facilities on GTK+/Linux
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "Platform.h"

#include "Scintilla.h"
#include "ScintillaWidget.h"

/* Use fast way of getting char data on win32 to work around problems
   with gdk_string_extents. */
#ifdef G_OS_WIN32
#define FAST_WAY
#endif

Point Point::FromLong(long lpoint) {
	return Point(
	           Platform::LowShortFromLong(lpoint),
	           Platform::HighShortFromLong(lpoint));
}

static GdkColor ColourfromRGB(unsigned int red, unsigned int green, unsigned int blue) {
	GdkColor co;
	co.red = red * (65535 / 255);
	co.green = green * (65535 / 255);
	co.blue = blue * (65535 / 255);
	// the pixel value indicates the index in the colourmap of the colour.
	// it is simply a combination of the RGB values we set earlier
	co.pixel = (gulong)(red * 65536 + green * 256 + blue);
	return co;
}

Colour::Colour(long lcol) {
	unsigned int red = lcol & 0xff;
	unsigned int green = (lcol >> 8) & 0xff;
	unsigned int blue = lcol >> 16;
	co = ColourfromRGB(red, green, blue);
}

Colour::Colour(unsigned int red, unsigned int green, unsigned int blue) {
	co = ColourfromRGB(red, green, blue);
}

bool Colour::operator==(const Colour &other) const {
	return
	    co.red == other.co.red &&
	    co.green == other.co.green &&
	    co.blue == other.co.blue;
}

unsigned int Colour::GetRed() {
	return co.red;
}

unsigned int Colour::GetGreen() {
	return co.green;
}

unsigned int Colour::GetBlue() {
	return co.blue;
}

long Colour::AsLong() const {
	unsigned int red = co.red * 255 / 65535;
	unsigned int green = co.green * 255 / 65535;
	unsigned int blue = co.blue * 255 / 65535;
	return (red + green*256 + blue*65536);
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
	delete []allocatedPalette;
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
			entries[used].allocated = cp.desired;
			used++;
		}
	} else {
		for (int i = 0; i < used; i++) {
			if (entries[i].desired == cp.desired) {
				cp.allocated = entries[i].allocated;
				return ;
			}
		}
		cp.allocated = cp.desired;
	}
}

void Palette::Allocate(Window &w) {
	if (allocatedPalette) {
		gdk_colormap_free_colors(gtk_widget_get_colormap(w.GetID()),
		                         allocatedPalette, allocatedLen);
		delete []allocatedPalette;
		allocatedPalette = 0;
		allocatedLen = 0;
	}
	allocatedPalette = new GdkColor[used];
	gboolean *successPalette = new gboolean[used];
	if (allocatedPalette) {
		allocatedLen = used;
		int iPal = 0;
		for (iPal = 0; iPal < used; iPal++) {
			allocatedPalette[iPal] = entries[iPal].desired.co;
		}
		gdk_colormap_alloc_colors(gtk_widget_get_colormap(w.GetID()),
		                          allocatedPalette, allocatedLen, FALSE, TRUE,
		                          successPalette);
		for (iPal = 0; iPal < used; iPal++) {
			entries[iPal].allocated.co = allocatedPalette[iPal];
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
		return "*-*";
	case SC_CHARSET_JOHAB:
		return "*-*";
	case SC_CHARSET_HEBREW:
		return "*-8";
	case SC_CHARSET_ARABIC:
		return "*-6";
	case SC_CHARSET_VIETNAMESE:
		return "*-*";
	case SC_CHARSET_THAI:
		return "*-*";
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
		gdk_font_unref(id);
	id = 0;
}

Surface::Surface() : unicodeMode(false), drawable(0), gc(0), ppixmap(0),
x(0), y(0), inited(false), createdGC(false) {}

Surface::~Surface() {
	Release();
}

void Surface::Release() {
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

bool Surface::Initialised() {
	return inited;
}

void Surface::Init() {
	Release();
	inited = true;
}

void Surface::Init(GdkDrawable *drawable_) {
	Release();
	drawable = drawable_;
	gc = gdk_gc_new(drawable_);
	//gdk_gc_set_line_attributes(gc, 1, 
	//	GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_BEVEL);
	createdGC = true;
	inited = true;
}

void Surface::InitPixMap(int width, int height, Surface *surface_) {
	Release();
	if (height > 0 && width > 0)
		ppixmap = gdk_pixmap_new(surface_->drawable, width, height, -1);
	drawable = ppixmap;
	gc = gdk_gc_new(surface_->drawable);
	//gdk_gc_set_line_attributes(gc, 1, 
	//	GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_BEVEL);
	createdGC = true;
	inited = true;
}

void Surface::PenColour(Colour fore) {
	if (gc)
		gdk_gc_set_foreground(gc, &fore.co);
}

int Surface::LogPixelsY() {
	return 72;
}

int Surface::DeviceHeightFont(int points) {
	int logPix = LogPixelsY();
	return (points * logPix + logPix / 2) / 72;
}

void Surface::MoveTo(int x_, int y_) {
	x = x_;
	y = y_;
}

void Surface::LineTo(int x_, int y_) {
	gdk_draw_line(drawable, gc,
	              x, y,
	              x_, y_);
	x = x_;
	y = y_;
}

void Surface::Polygon(Point *pts, int npts, Colour fore,
                      Colour back) {
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

void Surface::RectangleDraw(PRectangle rc, Colour fore, Colour back) {
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

void Surface::FillRectangle(PRectangle rc, Colour back) {
	PenColour(back);
	if (drawable) {
		gdk_draw_rectangle(drawable, gc, 1,
		                   rc.left, rc.top,
		                   rc.right - rc.left, rc.bottom - rc.top);
	}
}

void Surface::FillRectangle(PRectangle rc, Surface &surfacePattern) {
	if (surfacePattern.drawable) {
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
				                surfacePattern.drawable,
				                0, 0,
				                xTile, yTile,
				                widthx, heighty);
			}
		}
	} else {
		// Something is wrong so try to show anyway
		// Shows up black because colour not allocated
		FillRectangle(rc, Colour(0xff, 0, 0));
	}
}

void Surface::RoundedRectangle(PRectangle rc, Colour fore, Colour back) {
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

void Surface::Ellipse(PRectangle rc, Colour fore, Colour back) {
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

void Surface::Copy(PRectangle rc, Point from, Surface &surfaceSource) {
	if (surfaceSource.drawable) {
		gdk_draw_pixmap(drawable,
		                gc,
		                surfaceSource.drawable,
		                from.x, from.y,
		                rc.left, rc.top,
		                rc.right - rc.left, rc.bottom - rc.top);
	}
}

void Surface::DrawText(PRectangle rc, Font &font_, int ybase, const char *s, int len,
                       Colour fore, Colour back) {
	FillRectangle(rc, back);
	PenColour(fore);
	if (gc && drawable)
		gdk_draw_text(drawable, font_.id, gc, rc.left, ybase, s, len);
}

// On GTK+, exactly same as DrawText
void Surface::DrawTextClipped(PRectangle rc, Font &font_, int ybase, const char *s, int len,
                              Colour fore, Colour back) {
	FillRectangle(rc, back);
	PenColour(fore);
	if (gc && drawable)
		gdk_draw_text(drawable, font_.id, gc, rc.left, ybase, s, len);
}

void Surface::MeasureWidths(Font &font_, const char *s, int len, int *positions) {
	int totalWidth = 0;
	for (int i = 0;i < len;i++) {
		if (font_.id) {
			int width = gdk_char_width(font_.id, s[i]);
			totalWidth += width;
		} else {
			totalWidth++;
		}
		positions[i] = totalWidth;
	}
}

int Surface::WidthText(Font &font_, const char *s, int len) {
	if (font_.id)
		return gdk_text_width(font_.id, s, len);
	else
		return 1;
}

int Surface::WidthChar(Font &font_, char ch) {
	if (font_.id)
		return gdk_char_width(font_.id, ch);
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

int Surface::Ascent(Font &font_) {
	if (!font_.id)
		return 1;
#ifdef FAST_WAY
	return font_.id->ascent;
#else
	gint lbearing;
	gint rbearing;
	gint width;
	gint ascent;
	gint descent;

	gdk_string_extents(font_.id, sizeString,
	                   &lbearing, &rbearing, &width, &ascent, &descent);
	return ascent;
#endif
}

int Surface::Descent(Font &font_) {
	if (!font_.id)
		return 1;
#ifdef FAST_WAY
	return font_.id->descent;
#else
	gint lbearing;
	gint rbearing;
	gint width;
	gint ascent;
	gint descent;

	gdk_string_extents(font_.id, sizeString,
	                   &lbearing, &rbearing, &width, &ascent, &descent);
	return descent;
#endif
}

int Surface::InternalLeading(Font &) {
	return 0;
}

int Surface::ExternalLeading(Font &) {
	return 0;
}

int Surface::Height(Font &font_) {
	return Ascent(font_) + Descent(font_);
}

int Surface::AverageCharWidth(Font &font_) {
	if (font_.id)
		return gdk_char_width(font_.id, 'n');
	else
		return 1;
}

int Surface::SetPalette(Palette *, bool) {
	// Handled in palette allocation for GTK so this does nothing
	return 0;
}

void Surface::SetClip(PRectangle rc) {
	GdkRectangle area = {rc.left, rc.top,
	                     rc.right - rc.left, rc.bottom - rc.top};
	gdk_gc_set_clip_rectangle(gc, &area);
}

void Surface::FlushCachedState() {}

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
		rc.left = id->allocation.x;
		rc.top = id->allocation.y;
		if (id->allocation.width > 20) {
			rc.right = rc.left + id->allocation.width;
			rc.bottom = rc.top + id->allocation.height;
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
	gtk_widget_size_allocate(id, &alloc);
#else
	gtk_widget_set_uposition(id, rc.left, rc.top);
	gtk_widget_set_usize(id, rc.right - rc.left, rc.bottom - rc.top);
#endif
}

void Window::SetPositionRelative(PRectangle rc, Window relativeTo) {
	int ox = 0;
	int oy = 0;
	gdk_window_get_origin(relativeTo.id->window, &ox, &oy);

	gtk_widget_set_uposition(id, rc.left + ox, rc.top + oy);
#if 0
	GtkAllocation alloc;
	alloc.x = rc.left + ox;
	alloc.y = rc.top + oy;
	alloc.width = rc.right - rc.left;
	alloc.height = rc.bottom - rc.top;
	gtk_widget_size_allocate(id, &alloc);
#endif
	gtk_widget_set_usize(id, rc.right - rc.left, rc.bottom - rc.top);
}

PRectangle Window::GetClientPosition() {
	// On GTK+, the client position is the window position
	return GetPosition();
}

void Window::Show(bool show) {
	if (show)
		gtk_widget_show(id);
}

void Window::InvalidateAll() {
	if (id) {
		gtk_widget_queue_draw(id);
	}
}

void Window::InvalidateRectangle(PRectangle rc) {
	if (id) {
		gtk_widget_queue_draw_area(id,
		                           rc.left, rc.top,
		                           rc.right - rc.left, rc.bottom - rc.top);
	}
}

void Window::SetFont(Font &) {
	// TODO
}

void Window::SetCursor(Cursor curs) {
	switch (curs) {
	case cursorText:
		gdk_window_set_cursor(id->window, gdk_cursor_new(GDK_XTERM));
		break;
	case cursorArrow:
		gdk_window_set_cursor(id->window, gdk_cursor_new(GDK_ARROW));
		break;
	case cursorUp:
		gdk_window_set_cursor(id->window, gdk_cursor_new(GDK_CENTER_PTR));
		break;
	case cursorWait:
		gdk_window_set_cursor(id->window, gdk_cursor_new(GDK_WATCH));
		break;
	case cursorReverseArrow:
		gdk_window_set_cursor(id->window, gdk_cursor_new(GDK_TOP_LEFT_ARROW));
		break;
	default:
		gdk_window_set_cursor(id->window, gdk_cursor_new(GDK_ARROW));
		break;
	}
}

void Window::SetTitle(const char *s) {
	gtk_window_set_title(GTK_WINDOW(id), s);
}

ListBox::ListBox() : list(0), current(0), desiredVisibleRows(5), maxItemCharacters(0) {}

ListBox::~ListBox() {}

static void SelectionAC(GtkWidget *, gint row, gint,
                        GdkEventButton *, gpointer p) {
	int *pi = reinterpret_cast<int *>(p);
	*pi = row;
}

void ListBox::Create(Window &, int) {
	id = gtk_window_new(GTK_WINDOW_POPUP);

	GtkWidget* frame = gtk_frame_new(NULL);
	gtk_widget_show (frame);
	gtk_container_add(GTK_CONTAINER(GetID()), frame);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 0);

	scroller = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_set_border_width(GTK_CONTAINER(scroller), 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller),
	                               GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(frame), scroller);
	gtk_widget_show(scroller);

	list = gtk_clist_new(1);
	gtk_widget_show(list);
	gtk_container_add(GTK_CONTAINER(scroller), list);
	gtk_clist_set_column_auto_resize(GTK_CLIST(list), 0, TRUE);
	gtk_clist_set_selection_mode(GTK_CLIST(list), GTK_SELECTION_BROWSE);
	gtk_signal_connect(GTK_OBJECT(list), "select_row",
	                   GTK_SIGNAL_FUNC(SelectionAC), &current);
	gtk_clist_set_shadow_type(GTK_CLIST(list), GTK_SHADOW_NONE);

	gtk_widget_realize(id);
}

void ListBox::SetFont(Font &scint_font) {
	GtkStyle* style;

	style = gtk_widget_get_style(GTK_WIDGET(list));
	if (!gdk_font_equal(style->font, scint_font.GetID())) {
		style = gtk_style_copy(style);
		gdk_font_unref(style->font);
		style->font = scint_font.GetID();
		gdk_font_ref(style->font);
		gtk_widget_set_style(GTK_WIDGET(list), style);
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
		          + 2 * (list->style->klass->ythickness
		                 + GTK_CONTAINER(list)->border_width));
		gtk_widget_set_usize(GTK_WIDGET(list), -1, height);

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
	return GTK_CLIST(list)->rows;
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
	gtk_item_factory_popup(id, pt.x - 4, pt.y, 3, 0);
}

Colour Platform::Chrome() {
	return Colour(0xe0, 0xe0, 0xe0);
}

Colour Platform::ChromeHighlight() {
	return Colour(0xff, 0xff, 0xff);
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
