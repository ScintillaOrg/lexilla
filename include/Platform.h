// Scintilla source code edit control
/** @file Platform.h
 ** Interface to platform facilities. Also includes some basic utilities.
 ** Implemented in PlatGTK.cxx for GTK+/Linux, PlatWin.cxx for Windows, and PlatWX.cxx for wxWindows.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef PLATFORM_H
#define PLATFORM_H

// PLAT_GTK = GTK+ on Linux or Win32
// PLAT_GTK_WIN32 is defined additionally when running PLAT_GTK under Win32
// PLAT_WIN = Win32 API on Win32 OS
// PLAT_WX is wxWindows on any supported platform

#define PLAT_GTK 0
#define PLAT_GTK_WIN32 0
#define PLAT_WIN 0
#define PLAT_WX  0

#if defined(__WX__)
#undef PLAT_WX
#define PLAT_WX  1

#elif defined(GTK)
#undef PLAT_GTK
#define PLAT_GTK 1

#ifdef _MSC_VER
#undef PLAT_GTK_WIN32
#define PLAT_GTK_WIN32 1
#endif

#else
#undef PLAT_WIN
#define PLAT_WIN 1

#endif


// Underlying the implementation of the platform classes are platform specific types.
// Sometimes these need to be passed around by client code so they are defined here

typedef void* FontID;
typedef void* SurfaceID;
typedef void* WindowID;
typedef void* MenuID;
typedef void* PaletteID;

/**
 * A geometric point class.
 * Point is exactly the same as the Win32 POINT and GTK+ GdkPoint so can be used interchangeably.
 */
class Point {
public:
	int x;
	int y;

	Point(int x_=0, int y_=0) : x(x_), y(y_) {
	}

	// Other automatically defined methods (assignment, copy constructor, destructor) are fine

	static Point FromLong(long lpoint);
};

/**
 * A geometric rectangle class.
 * PRectangle is exactly the same as the Win32 RECT so can be used interchangeably.
 * PRectangles contain their top and left sides, but not their right and bottom sides.
 */
class PRectangle {
public:
	int left;
	int top;
	int right;
	int bottom;

	PRectangle(int left_=0, int top_=0, int right_=0, int bottom_ = 0) :
		left(left_), top(top_), right(right_), bottom(bottom_) {
	}

	// Other automatically defined methods (assignment, copy constructor, destructor) are fine

	bool operator==(PRectangle &rc) {
		return (rc.left == left) && (rc.right == right) &&
			(rc.top == top) && (rc.bottom == bottom);
	}
	bool Contains(Point pt) {
		return (pt.x >= left) && (pt.x <= right) &&
			(pt.y >= top) && (pt.y <= bottom);
	}
	bool Contains(PRectangle rc) {
		return (rc.left >= left) && (rc.right <= right) &&
			(rc.top >= top) && (rc.bottom <= bottom);
	}
	bool Intersects(PRectangle other) {
		return (right >= other.left) && (left <= other.right) &&
			(bottom >= other.top) && (top <= other.bottom);
	}
	int Width() { return right - left; }
	int Height() { return bottom - top; }
};

/**
 * A colour class.
 */
class ColourDesired {
	long co;
public:
	ColourDesired(long lcol=0) {
		co = lcol;
	}
	
	ColourDesired(unsigned int red, unsigned int green, unsigned int blue) {
		co = red | (green << 8) | (blue << 16);
	}
	
	bool operator==(const ColourDesired &other) const {
		return co == other.co;
	}
	
	void Set(long lcol) {
		co = lcol;
	}
	
	long AsLong() const {
		return co;
	}
	
	unsigned int GetRed() {
		return co & 0xff;
	}
	
	unsigned int GetGreen() {
		return (co >> 8) & 0xff;
	}
	
	unsigned int GetBlue() {
		return (co >> 16) & 0xff;
	}
};

class ColourAllocated {
	long coAllocated;
	
public:

	ColourAllocated(long lcol=0) {
		coAllocated = lcol;
	}
	
	void Set(long lcol) {
		coAllocated = lcol;
	}
	
	long AsLong() const {
		return coAllocated;
	}
};

/**
 * Colour pairs hold a desired colour and the colour that the graphics engine
 * allocates to approximate the desired colour.
 * To make palette management more automatic, ColourPairs could register at
 * construction time with a palette management object.
 */
struct ColourPair {
	ColourDesired desired;
	ColourAllocated allocated;

	ColourPair(ColourDesired desired_=ColourDesired(0,0,0)) {
		desired = desired_;
		allocated.Set(desired.AsLong());
	}
};

class Window;	// Forward declaration for Palette

/**
 * Colour palette management.
 */
class Palette {
	int used;
	enum {numEntries = 100};
	ColourPair entries[numEntries];
#if PLAT_GTK
	void *allocatedPalette; // GdkColor *
	int allocatedLen;
#elif PLAT_WIN
	void *hpal;
#elif PLAT_WX
	// wxPalette* pal;  // **** Is this needed?
#endif
public:
	bool allowRealization;

	Palette();
	~Palette();

	void Release();

	/**
	 * This method either adds a colour to the list of wanted colours (want==true)
	 * or retrieves the allocated colour back to the ColourPair.
	 * This is one method to make it easier to keep the code for wanting and retrieving in sync.
	 */
	void WantFind(ColourPair &cp, bool want);

	void Allocate(Window &w);

	friend class Surface;
};

/**
 * Font management.
 */
class Font {
protected:
	FontID id;
#if PLAT_WX
	int ascent;
#endif
	// Private so Font objects can not be copied
	Font(const Font &) {}
	Font &operator=(const Font &) { id=0; return *this; }
public:
	Font();
	virtual ~Font();

	virtual void Create(const char *faceName, int characterSet, int size, bool bold, bool italic);
	virtual void Release();

	FontID GetID() { return id; }
	// Alias another font - caller guarantees not to Release
	void SetID(FontID id_) { id = id_; }
	friend class Surface;
};

/**
 * A surface abstracts a place to draw.
 */
class Surface {
private:
	bool unicodeMode;
#if PLAT_GTK
	void *drawable;	// GdkDrawable *drawable;
	void *gc;			// GdkGC *gc;
	void *ppixmap;	// GdkPixmap *ppixmap;
	int x;
	int y;
	bool inited;
	bool createdGC;
#elif PLAT_WIN
	void *hdc; // HDC 
	bool hdcOwned;
	void *pen;	// HPEN
	void *penOld;	// HPEN
	void *brush;	// HBRUSH
	void *brushOld; // HBRUSH
	void *font;	// HFONT 
	void *fontOld;	// HFONT 
	void *bitmap;	// HBITMAP 
	void *bitmapOld;	// HBITMAP 
	void *paletteOld;	// HPALETTE 
#elif PLAT_WX
	void *hdc;	// wxDC*
	bool hdcOwned;
	void *bitmap;	// wxBitmap*
	int x;
	int y;
#endif

	// Private so Surface objects can not be copied
	Surface(const Surface &) {}
	Surface &operator=(const Surface &) { return *this; }
#if PLAT_WIN || PLAT_WX
	void BrushColor(ColourAllocated back);
	void SetFont(Font &font_);
#endif
public:
	Surface();
	~Surface();

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

	void DrawText(PRectangle rc, Font &font_, int ybase, const char *s, int len, ColourAllocated fore, ColourAllocated back);
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

	void SetUnicodeMode(bool unicodeMode_) {
		unicodeMode=unicodeMode_;
	}
};

/**
 * A simple callback action passing one piece of untyped user data.
 */
typedef void (*CallBackAction)(void*);

/**
 * Class to hide the details of window manipulation.
 * Does not own the window which will normally have a longer life than this object.
 */
class Window {
	friend class ListBox;
protected:
	WindowID id;
public:
	Window() : id(0) {}
	Window(const Window &source) : id(source.id) {}
	virtual ~Window();
	Window &operator=(WindowID id_) {
		id = id_;
		return *this;
	}
	WindowID GetID() { return id; }
	bool Created() { return id != 0; }
	void Destroy();
	bool HasFocus();
	PRectangle GetPosition();
	void SetPosition(PRectangle rc);
	void SetPositionRelative(PRectangle rc, Window relativeTo);
	PRectangle GetClientPosition();
	void Show(bool show=true);
	void InvalidateAll();
	void InvalidateRectangle(PRectangle rc);
	virtual void SetFont(Font &font);
	enum Cursor { cursorText, cursorArrow, cursorUp, cursorWait, cursorHoriz, cursorVert, cursorReverseArrow };
	void SetCursor(Cursor curs);
	void SetTitle(const char *s);
};

/**
 * Listbox management.
 */

class ListBox : public Window {
private:
#if PLAT_GTK
	WindowID list;
	WindowID scroller;
	int current;
#endif
	int desiredVisibleRows;
	unsigned int maxItemCharacters;
	unsigned int aveCharWidth;
public:
	CallBackAction doubleClickAction;
	void *doubleClickActionData;
public:
	ListBox();
	virtual ~ListBox();
	void Create(Window &parent, int ctrlID);
	virtual void SetFont(Font &font);
	void SetAverageCharWidth(int width);
	void SetVisibleRows(int rows);
	PRectangle GetDesiredRect();
	void Clear();
	void Append(char *s);
	int Length();
	void Select(int n);
	int GetSelection();
	int Find(const char *prefix);
	void GetValue(int n, char *value, int len);
	void Sort();
	void SetDoubleClickAction(CallBackAction action, void *data) {
		doubleClickAction = action;
		doubleClickActionData = data;
	}
};

/**
 * Menu management.
 */
class Menu {
	MenuID id;
public:
	Menu();
	MenuID GetID() { return id; }
	void CreatePopUp();
	void Destroy();
	void Show(Point pt, Window &w);
};

/**
 * Platform class used to retrieve system wide parameters such as double click speed
 * and chrome colour. Not a creatable object, more of a module with several functions.
 */
class Platform {
	// Private so Platform objects can not be copied
	Platform(const Platform &) {}
	Platform &operator=(const Platform &) { return *this; }
public:
	// Should be private because no new Platforms are ever created
	// but gcc warns about this
	Platform() {}
	~Platform() {}
	static ColourDesired Chrome();
	static ColourDesired ChromeHighlight();
	static const char *DefaultFont();
	static int DefaultFontSize();
	static unsigned int DoubleClickTime();
	static void DebugDisplay(const char *s);
	static bool IsKeyDown(int key);
	static long SendScintilla(
		WindowID w, unsigned int msg, unsigned long wParam=0, long lParam=0);
	static bool IsDBCSLeadByte(int codePage, char ch);

	// These are utility functions not really tied to a platform
	static int Minimum(int a, int b);
	static int Maximum(int a, int b);
	// Next three assume 16 bit shorts and 32 bit longs
	static long LongFromTwoShorts(short a,short b) {
		return (a) | ((b) << 16);
	}
	static short HighShortFromLong(long x) {
		return static_cast<short>(x >> 16);
	}
	static short LowShortFromLong(long x) {
		return static_cast<short>(x & 0xffff);
	}
	static void DebugPrintf(const char *format, ...);
	static bool ShowAssertionPopUps(bool assertionPopUps_);
	static void Assert(const char *c, const char *file, int line);
	static int Clamp(int val, int minVal, int maxVal);
};

#ifdef  NDEBUG
#define PLATFORM_ASSERT(c) ((void)0)
#else
#define PLATFORM_ASSERT(c) ((c) ? (void)(0) : Platform::Assert(#c, __FILE__, __LINE__))
#endif

// Shut up annoying Visual C++ warnings:
#ifdef _MSC_VER
#pragma warning(disable: 4244 4309 4514 4710 4800)
#endif

#endif
