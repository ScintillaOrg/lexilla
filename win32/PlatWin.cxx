// Scintilla source code edit control
/** @file PlatWin.cxx
 ** Implementation of platform facilities on Windows.
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <vector>
#include <map>

#undef _WIN32_WINNT
#define _WIN32_WINNT  0x0500
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <windowsx.h>
#include <d2d1.h>
#include <dwrite.h>

#include "Platform.h"
#include "UniConversion.h"
#include "XPM.h"
#include "FontQuality.h"

// We want to use multi monitor functions, but via LoadLibrary etc
// Luckily microsoft has done the heavy lifting for us, so we'll just use their stub functions!
#if defined(_MSC_VER) && (_MSC_VER > 1200)
#define COMPILE_MULTIMON_STUBS
#include <MultiMon.h>
#endif

#ifndef IDC_HAND
#define IDC_HAND MAKEINTRESOURCE(32649)
#endif

// Take care of 32/64 bit pointers
#ifdef GetWindowLongPtr
static void *PointerFromWindow(HWND hWnd) {
	return reinterpret_cast<void *>(::GetWindowLongPtr(hWnd, 0));
}
static void SetWindowPointer(HWND hWnd, void *ptr) {
	::SetWindowLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(ptr));
}
#else
static void *PointerFromWindow(HWND hWnd) {
	return reinterpret_cast<void *>(::GetWindowLong(hWnd, 0));
}
static void SetWindowPointer(HWND hWnd, void *ptr) {
	::SetWindowLong(hWnd, 0, reinterpret_cast<LONG>(ptr));
}

#ifndef GWLP_USERDATA
#define GWLP_USERDATA GWL_USERDATA
#endif

#ifndef GWLP_WNDPROC
#define GWLP_WNDPROC GWL_WNDPROC
#endif

#ifndef LONG_PTR
#define LONG_PTR LONG
#endif

static LONG_PTR SetWindowLongPtr(HWND hWnd, int nIndex, LONG_PTR dwNewLong) {
	return ::SetWindowLong(hWnd, nIndex, dwNewLong);
}

static LONG_PTR GetWindowLongPtr(HWND hWnd, int nIndex) {
	return ::GetWindowLong(hWnd, nIndex);
}
#endif

typedef BOOL (WINAPI *AlphaBlendSig)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);

static CRITICAL_SECTION crPlatformLock;
static HINSTANCE hinstPlatformRes = 0;
static bool onNT = false;
static HMODULE hDLLImage = 0;
static AlphaBlendSig AlphaBlendFn = 0;
static HCURSOR reverseArrowCursor = NULL;

bool IsNT() {
	return onNT;
}

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

Point Point::FromLong(long lpoint) {
	return Point(static_cast<short>(LOWORD(lpoint)), static_cast<short>(HIWORD(lpoint)));
}

static RECT RectFromPRectangle(PRectangle prc) {
	RECT rc = {prc.left, prc.top, prc.right, prc.bottom};
	return rc;
}

Palette::Palette() {
	used = 0;
	allowRealization = false;
	hpal = 0;
	size = 100;
	entries = new ColourPair[size];
}

Palette::~Palette() {
	Release();
	delete []entries;
	entries = 0;
}

void Palette::Release() {
	used = 0;
	if (hpal)
		::DeleteObject(hpal);
	hpal = 0;
	delete []entries;
	size = 100;
	entries = new ColourPair[size];
}

/**
 * This method either adds a colour to the list of wanted colours (want==true)
 * or retrieves the allocated colour back to the ColourPair.
 * This is one method to make it easier to keep the code for wanting and retrieving in sync.
 */
void Palette::WantFind(ColourPair &cp, bool want) {
	if (want) {
		for (int i=0; i < used; i++) {
			if (entries[i].desired == cp.desired)
				return;
		}

		if (used >= size) {
			int sizeNew = size * 2;
			ColourPair *entriesNew = new ColourPair[sizeNew];
			for (int j=0; j<size; j++) {
				entriesNew[j] = entries[j];
			}
			delete []entries;
			entries = entriesNew;
			size = sizeNew;
		}

		entries[used].desired = cp.desired;
		entries[used].allocated.Set(cp.desired.AsLong());
		used++;
	} else {
		for (int i=0; i < used; i++) {
			if (entries[i].desired == cp.desired) {
				cp.allocated = entries[i].allocated;
				return;
			}
		}
		cp.allocated.Set(cp.desired.AsLong());
	}
}

void Palette::Allocate(Window &) {
	if (hpal)
		::DeleteObject(hpal);
	hpal = 0;

	if (allowRealization) {
		char *pal = new char[sizeof(LOGPALETTE) + (used-1) * sizeof(PALETTEENTRY)];
		LOGPALETTE *logpal = reinterpret_cast<LOGPALETTE *>(pal);
		logpal->palVersion = 0x300;
		logpal->palNumEntries = static_cast<WORD>(used);
		for (int iPal=0;iPal<used;iPal++) {
			ColourDesired desired = entries[iPal].desired;
			logpal->palPalEntry[iPal].peRed   = static_cast<BYTE>(desired.GetRed());
			logpal->palPalEntry[iPal].peGreen = static_cast<BYTE>(desired.GetGreen());
			logpal->palPalEntry[iPal].peBlue  = static_cast<BYTE>(desired.GetBlue());
			entries[iPal].allocated.Set(
				PALETTERGB(desired.GetRed(), desired.GetGreen(), desired.GetBlue()));
			// PC_NOCOLLAPSE means exact colours allocated even when in background this means other windows
			// are less likely to get their colours and also flashes more when switching windows
			logpal->palPalEntry[iPal].peFlags = PC_NOCOLLAPSE;
			// 0 allows approximate colours when in background, yielding moe colours to other windows
			//logpal->palPalEntry[iPal].peFlags = 0;
		}
		hpal = ::CreatePalette(logpal);
		delete []pal;
	}
}

IDWriteFactory *pIDWriteFactory = 0;
ID2D1Factory *pD2DFactory = 0;

bool LoadD2D() {
	static bool triedLoadingD2D = false;
	static HMODULE hDLLD2D = 0;
	static HMODULE hDLLDWrite = 0;
	if (!triedLoadingD2D) {
		typedef HRESULT (WINAPI *D2D1CFSig)(D2D1_FACTORY_TYPE factoryType, REFIID riid,
			CONST D2D1_FACTORY_OPTIONS *pFactoryOptions, IUnknown **factory);
		typedef HRESULT (WINAPI *DWriteCFSig)(DWRITE_FACTORY_TYPE factoryType, REFIID iid,
			IUnknown **factory);

		hDLLD2D = ::LoadLibrary(TEXT("D2D1.DLL"));
		if (hDLLD2D) {
			D2D1CFSig fnD2DCF = (D2D1CFSig)::GetProcAddress(hDLLD2D, "D2D1CreateFactory");
			if (fnD2DCF) {
				// A single threaded factory as Scintilla always draw on the GUI thread
				fnD2DCF(D2D1_FACTORY_TYPE_SINGLE_THREADED,
					__uuidof(ID2D1Factory),
					0,
					reinterpret_cast<IUnknown**>(&pD2DFactory));
			}
		}
		hDLLDWrite = ::LoadLibrary(TEXT("DWRITE.DLL"));
		if (hDLLDWrite) {
			DWriteCFSig fnDWCF = (DWriteCFSig)::GetProcAddress(hDLLDWrite, "DWriteCreateFactory");
			if (fnDWCF) {
				fnDWCF(DWRITE_FACTORY_TYPE_SHARED,
					__uuidof(IDWriteFactory),
					reinterpret_cast<IUnknown**>(&pIDWriteFactory));
			}
		}
	}
	triedLoadingD2D = true;
	return pIDWriteFactory && pD2DFactory;
}

struct FormatAndBaseline {
	IDWriteTextFormat *pTextFormat;
	FLOAT baseline;
	FormatAndBaseline(IDWriteTextFormat *pTextFormat_, FLOAT baseline_) : 
		pTextFormat(pTextFormat_), baseline(baseline_) {
	}
	~FormatAndBaseline() {
		pTextFormat->Release();
		pTextFormat = 0;
		baseline = 1;
	}
	HFONT HFont();
};

HFONT FormatAndBaseline::HFont() {
	LOGFONTW lf;
	memset(&lf, 0, sizeof(lf));
	const int familySize = 200;
	WCHAR fontFamilyName[familySize];

	HRESULT hr = pTextFormat->GetFontFamilyName(fontFamilyName, familySize);
	if (SUCCEEDED(hr)) {
		lf.lfWeight = pTextFormat->GetFontWeight();
		lf.lfItalic = pTextFormat->GetFontStyle() == DWRITE_FONT_STYLE_ITALIC;
		lf.lfHeight = -static_cast<int>(pTextFormat->GetFontSize());
		return ::CreateFontIndirectW(&lf);
	}
	return 0;
}

#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY 5
#endif

static BYTE Win32MapFontQuality(int extraFontFlag) {
	switch (extraFontFlag & SC_EFF_QUALITY_MASK) {

		case SC_EFF_QUALITY_NON_ANTIALIASED:
			return NONANTIALIASED_QUALITY;

		case SC_EFF_QUALITY_ANTIALIASED:
			return ANTIALIASED_QUALITY;

		case SC_EFF_QUALITY_LCD_OPTIMIZED:
			return CLEARTYPE_QUALITY;

		default:
			return SC_EFF_QUALITY_DEFAULT;
	}
}

static void SetLogFont(LOGFONTA &lf, const char *faceName, int characterSet, float size, int weight, bool italic, int extraFontFlag) {
	memset(&lf, 0, sizeof(lf));
	// The negative is to allow for leading
	lf.lfHeight = -(abs(static_cast<int>(size)));
	lf.lfWeight = weight;
	lf.lfItalic = static_cast<BYTE>(italic ? 1 : 0);
	lf.lfCharSet = static_cast<BYTE>(characterSet);
	lf.lfQuality = Win32MapFontQuality(extraFontFlag);
	strncpy(lf.lfFaceName, faceName, sizeof(lf.lfFaceName));
}

/**
 * Create a hash from the parameters for a font to allow easy checking for identity.
 * If one font is the same as another, its hash will be the same, but if the hash is the
 * same then they may still be different.
 */
static int HashFont(const char *faceName, int characterSet, float size, int weight, bool italic, int extraFontFlag) {
	return
		static_cast<int>(size) ^
		(characterSet << 10) ^
		((extraFontFlag & SC_EFF_QUALITY_MASK) << 9) ^
		((weight/100) << 12) ^
		(italic ? 0x20000000 : 0) ^
		faceName[0];
}

class FontCached : Font {
	FontCached *next;
	int usage;
	float size;
	LOGFONTA lf;
	int hash;
	FontCached(const char *faceName_, int characterSet_, float size_, int weight_, bool italic_, int extraFontFlag_);
	~FontCached() {}
	bool SameAs(const char *faceName_, int characterSet_, float size_, int weight_, bool italic_, int extraFontFlag_);
	virtual void Release();

	static FontCached *first;
public:
	static FontID FindOrCreate(const char *faceName_, int characterSet_, float size_, int weight_, bool italic_, int extraFontFlag_);
	static void ReleaseId(FontID fid_);
};

FontCached *FontCached::first = 0;

FontCached::FontCached(const char *faceName_, int characterSet_, float size_, int weight_, bool italic_, int extraFontFlag_) :
	next(0), usage(0), size(1.0), hash(0) {
	SetLogFont(lf, faceName_, characterSet_, size_, weight_, italic_, extraFontFlag_);
	hash = HashFont(faceName_, characterSet_, size_, weight_, italic_, extraFontFlag_);
	fid = 0;
	if (pIDWriteFactory) {
#ifdef OLD_CODE
		HFONT fontSave = static_cast<HFONT>(::SelectObject(hdc, font_.GetID()));
		DWORD sizeOLTM = ::GetOutlineTextMetrics(hdc, NULL, NULL);
		std::vector<char> vOLTM(sizeOLTM);
		LPOUTLINETEXTMETRIC potm = reinterpret_cast<LPOUTLINETEXTMETRIC>(&vOLTM[0]);
		DWORD worked = ::GetOutlineTextMetrics(hdc, sizeOLTM, potm);
		::SelectObject(hdc, fontSave);
		if (!worked)
			return;
		const WCHAR *pwcFamily = reinterpret_cast<WCHAR *>(&vOLTM[reinterpret_cast<size_t>(potm->otmpFamilyName)]);
		//const WCHAR *pwcFace = reinterpret_cast<WCHAR *>(&vOLTM[reinterpret_cast<size_t>(potm->otmpFaceName)]);
		FLOAT fHeight = potm->otmTextMetrics.tmHeight * 72.0f / 96.0f;
		bool italics = potm->otmTextMetrics.tmItalic != 0;
		bool bold = potm->otmTextMetrics.tmWeight >= FW_BOLD;
#endif
		IDWriteTextFormat *pTextFormat;
		const int faceSize = 200;
		WCHAR wszFace[faceSize];
		UTF16FromUTF8(faceName_, strlen(faceName_)+1, wszFace, faceSize);
		FLOAT fHeight = size_;
		HRESULT hr = pIDWriteFactory->CreateTextFormat(wszFace, NULL,
			static_cast<DWRITE_FONT_WEIGHT>(weight_),
			italic_ ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL, fHeight, L"en-us", &pTextFormat);
		if (SUCCEEDED(hr)) {
			const int maxLines = 2;
			DWRITE_LINE_METRICS lineMetrics[maxLines];
			UINT32 lineCount = 0;
			FLOAT baseline = 1.0f;
			IDWriteTextLayout *pTextLayout = 0;
			hr = pIDWriteFactory->CreateTextLayout(L"X", 1, pTextFormat,
					100.0f, 100.0f, &pTextLayout);
			if (SUCCEEDED(hr)) {
				hr = pTextLayout->GetLineMetrics(lineMetrics, maxLines, &lineCount);
				if (SUCCEEDED(hr)) {
					baseline = lineMetrics[0].baseline;
				}
				pTextLayout->Release();
			}
			fid = reinterpret_cast<void *>(new FormatAndBaseline(pTextFormat, baseline));
		}
	}
	usage = 1;
}

bool FontCached::SameAs(const char *faceName_, int characterSet_, float size_, int weight_, bool italic_, int extraFontFlag_) {
	return
		(size == size_) &&
		(lf.lfWeight == weight_) &&
		(lf.lfItalic == static_cast<BYTE>(italic_ ? 1 : 0)) &&
		(lf.lfCharSet == characterSet_) &&
		(lf.lfQuality == Win32MapFontQuality(extraFontFlag_)) &&
		0 == strcmp(lf.lfFaceName,faceName_);
}

void FontCached::Release() {
	delete reinterpret_cast<FormatAndBaseline *>(fid);
	fid = 0;
}

FontID FontCached::FindOrCreate(const char *faceName_, int characterSet_, float size_, int weight_, bool italic_, int extraFontFlag_) {
	FontID ret = 0;
	::EnterCriticalSection(&crPlatformLock);
	int hashFind = HashFont(faceName_, characterSet_, size_, weight_, italic_, extraFontFlag_);
	for (FontCached *cur=first; cur; cur=cur->next) {
		if ((cur->hash == hashFind) &&
			cur->SameAs(faceName_, characterSet_, size_, weight_, italic_, extraFontFlag_)) {
			cur->usage++;
			ret = cur->fid;
		}
	}
	if (ret == 0) {
		FontCached *fc = new FontCached(faceName_, characterSet_, size_, weight_, italic_, extraFontFlag_);
		if (fc) {
			fc->next = first;
			first = fc;
			ret = fc->fid;
		}
	}
	::LeaveCriticalSection(&crPlatformLock);
	return ret;
}

void FontCached::ReleaseId(FontID fid_) {
	::EnterCriticalSection(&crPlatformLock);
	FontCached **pcur=&first;
	for (FontCached *cur=first; cur; cur=cur->next) {
		if (cur->fid == fid_) {
			cur->usage--;
			if (cur->usage == 0) {
				*pcur = cur->next;
				cur->Release();
				cur->next = 0;
				delete cur;
			}
			break;
		}
		pcur=&cur->next;
	}
	::LeaveCriticalSection(&crPlatformLock);
}

Font::Font() {
	fid = 0;
}

Font::~Font() {
}

#define FONTS_CACHED

void Font::Create(const char *faceName, int characterSet, float size,
	int weight, bool italic, int extraFontFlag) {
	Release();
	if (faceName)
		fid = FontCached::FindOrCreate(faceName, characterSet, size, weight, italic, extraFontFlag);
}

void Font::Release() {
	if (fid)
		FontCached::ReleaseId(fid);
	fid = 0;
}

#ifdef SCI_NAMESPACE
namespace Scintilla {
#endif

class SurfaceImpl : public Surface {
	bool unicodeMode;
	HDC hdc;
	bool hdcOwned;
	int maxWidthMeasure;
	int maxLenText;
	int x, y;

	int codePage;
	// If 9x OS and current code page is same as ANSI code page.
	bool win9xACPSame;

	ID2D1RenderTarget *pRenderTarget;
	bool ownRenderTarget;
	int clipsActive;

	IDWriteTextFormat *pTextFormat;
	FLOAT baseline;
	ID2D1SolidColorBrush *pBrush;
	float dpiScaleX;
	float dpiScaleY;
	bool hasBegun;

	void SetFont(Font &font_);

	// Private so SurfaceImpl objects can not be copied
	SurfaceImpl(const SurfaceImpl &);
	SurfaceImpl &operator=(const SurfaceImpl &);
public:
	SurfaceImpl();
	virtual ~SurfaceImpl();

	void SetDWrite(HDC hdc);
	void Init(WindowID wid);
	void Init(SurfaceID sid, WindowID wid);
	void InitPixMap(int width, int height, Surface *surface_, WindowID wid);

	void Release();
	bool Initialised();

	HRESULT FlushDrawing();

	void PenColour(ColourAllocated fore);
	void D2DPenColour(ColourAllocated fore, int alpha=255);
	int LogPixelsY();
	int DeviceHeightFont(int points);
	void MoveTo(int x_, int y_);
	void LineTo(int x_, int y_);
	void Polygon(Point *pts, int npts, ColourAllocated fore, ColourAllocated back);
	void RectangleDraw(PRectangle rc, ColourAllocated fore, ColourAllocated back);
	void FillRectangle(PRectangle rc, ColourAllocated back);
	void FillRectangle(PRectangle rc, Surface &surfacePattern);
	void RoundedRectangle(PRectangle rc, ColourAllocated fore, ColourAllocated back);
	void AlphaRectangle(PRectangle rc, int cornerSize, ColourAllocated fill, int alphaFill,
		ColourAllocated outline, int alphaOutline, int flags);
	void DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage);
	void Ellipse(PRectangle rc, ColourAllocated fore, ColourAllocated back);
	void Copy(PRectangle rc, Point from, Surface &surfaceSource);

	void DrawTextCommon(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, UINT fuOptions);
	void DrawTextNoClip(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourAllocated fore, ColourAllocated back);
	void DrawTextClipped(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourAllocated fore, ColourAllocated back);
	void DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourAllocated fore);
	void MeasureWidths(Font &font_, const char *s, int len, XYPOSITION *positions);
	XYPOSITION WidthText(Font &font_, const char *s, int len);
	XYPOSITION WidthChar(Font &font_, char ch);
	XYPOSITION Ascent(Font &font_);
	XYPOSITION Descent(Font &font_);
	XYPOSITION InternalLeading(Font &font_);
	XYPOSITION ExternalLeading(Font &font_);
	XYPOSITION Height(Font &font_);
	XYPOSITION AverageCharWidth(Font &font_);

	int SetPalette(Palette *pal, bool inBackGround);
	void SetClip(PRectangle rc);
	void FlushCachedState();

	void SetUnicodeMode(bool unicodeMode_);
	void SetDBCSMode(int codePage_);
};

#ifdef SCI_NAMESPACE
} //namespace Scintilla
#endif

SurfaceImpl::SurfaceImpl() :
	unicodeMode(false),
	hdc(0), hdcOwned(false),
	x(0), y(0) {
	// Windows 9x has only a 16 bit coordinate system so break after 30000 pixels
	maxWidthMeasure = IsNT() ? INT_MAX : 30000;
	// There appears to be a 16 bit string length limit in GDI on NT and a limit of
	// 8192 characters on Windows 95.
	maxLenText = IsNT() ? 65535 : 8192;

	codePage = 0;
	win9xACPSame = false;

	pRenderTarget = NULL;
	ownRenderTarget = false;
	clipsActive = 0;
	pTextFormat = NULL;
	baseline = 1.0f;
	pBrush = NULL;
	dpiScaleX = 1.0;
	dpiScaleY = 1.0;
	hasBegun = false;
}

SurfaceImpl::~SurfaceImpl() {
	Release();
}

void SurfaceImpl::Release() {
	if (hdcOwned) {
		::DeleteDC(reinterpret_cast<HDC>(hdc));
		hdc = 0;
		hdcOwned = false;
	}

	if (pBrush) {
		pBrush->Release();
		pBrush = 0;
	}
	if (pRenderTarget) {
		while (clipsActive) {
			pRenderTarget->PopAxisAlignedClip();
			clipsActive--;
		}
		if (ownRenderTarget) {
			pRenderTarget->Release();
		}
		pRenderTarget = 0;
	}
}

void SurfaceImpl::SetDWrite(HDC hdc) {
	dpiScaleX = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
	dpiScaleY = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;
}

bool SurfaceImpl::Initialised() {
	return pRenderTarget != 0;
}

HRESULT SurfaceImpl::FlushDrawing() {
	return pRenderTarget->Flush();
}

void SurfaceImpl::Init(WindowID wid) {
	Release();
	hdc = ::CreateCompatibleDC(NULL);
	hdcOwned = true;
	::SetTextAlign(reinterpret_cast<HDC>(hdc), TA_BASELINE);
	RECT rc;
	::GetClientRect(reinterpret_cast<HWND>(wid), &rc);
	SetDWrite(hdc);
}

void SurfaceImpl::Init(SurfaceID sid, WindowID) {
	Release();
	hdc = ::CreateCompatibleDC(NULL);
	hdcOwned = true;
	pRenderTarget = reinterpret_cast<ID2D1HwndRenderTarget *>(sid);
	::SetTextAlign(reinterpret_cast<HDC>(hdc), TA_BASELINE);
	SetDWrite(hdc);
}

void SurfaceImpl::InitPixMap(int width, int height, Surface *surface_, WindowID) {
	Release();
	hdc = ::CreateCompatibleDC(NULL);	// Just for measurement
	hdcOwned = true;
	SurfaceImpl *psurfOther = static_cast<SurfaceImpl *>(surface_);
	ID2D1BitmapRenderTarget *pCompatibleRenderTarget = NULL;
	HRESULT hr = psurfOther->pRenderTarget->CreateCompatibleRenderTarget(
		D2D1::SizeF(width, height), &pCompatibleRenderTarget);
	if (SUCCEEDED(hr)) {
		pRenderTarget = pCompatibleRenderTarget;
		pRenderTarget->BeginDraw();
		ownRenderTarget = true;
	}
}

void SurfaceImpl::PenColour(ColourAllocated fore) {
	D2DPenColour(fore);
}

void SurfaceImpl::D2DPenColour(ColourAllocated fore, int alpha) {
	if (pRenderTarget) {
		D2D_COLOR_F col;
		col.r = (fore.AsLong() & 0xff) / 255.0;
		col.g = ((fore.AsLong() & 0xff00) >> 8) / 255.0;
		col.b = (fore.AsLong() >> 16) / 255.0;
		col.a = alpha / 255.0;
		if (pBrush) {
			pBrush->SetColor(col);
		} else {
			HRESULT hr = pRenderTarget->CreateSolidColorBrush(col, &pBrush);
			if (!SUCCEEDED(hr) && pBrush) {						
				pBrush->Release();
				pBrush = 0;
			}
		}
	}
}

void SurfaceImpl::SetFont(Font &font_) {
	FormatAndBaseline *pfabl = reinterpret_cast<FormatAndBaseline *>(font_.GetID());
	pTextFormat = pfabl->pTextFormat;
	baseline = pfabl->baseline;
}

int SurfaceImpl::LogPixelsY() {
	return ::GetDeviceCaps(hdc, LOGPIXELSY);
}

int SurfaceImpl::DeviceHeightFont(int points) {
	return ::MulDiv(points, LogPixelsY(), 72);
}

void SurfaceImpl::MoveTo(int x_, int y_) {
	x = x_;
	y = y_;
}

static int Delta(int difference) {
	if (difference < 0)
		return -1;
	else if (difference > 0)
		return 1;
	else
		return 0;
}

static int RoundFloat(float f) {
	return int(f+0.5);
}

void SurfaceImpl::LineTo(int x_, int y_) {
	if (pRenderTarget) {
		int xDiff = x_ - x;
		int xDelta = Delta(xDiff);
		int yDiff = y_ - y;
		int yDelta = Delta(yDiff);
		if ((xDiff == 0) || (yDiff == 0)) {
			// Horizontal or vertical lines can be more precisely drawn as a filled rectangle
			int xEnd = x_ - xDelta;
			int left = Platform::Minimum(x, xEnd);
			int width = abs(x - xEnd) + 1;
			int yEnd = y_ - yDelta;
			int top = Platform::Minimum(y, yEnd);
			int height = abs(y - yEnd) + 1;
			D2D1_RECT_F rectangle1 = D2D1::RectF(left, top, left+width, top+height);
			pRenderTarget->FillRectangle(&rectangle1, pBrush);
		} else if ((abs(xDiff) == abs(yDiff))) {
			// 45 degree slope
			pRenderTarget->DrawLine(D2D1::Point2F(x + 0.5, y + 0.5), 
				D2D1::Point2F(x_ + 0.5 - xDelta, y_ + 0.5 - yDelta), pBrush);
		} else {
			// Line has a different slope so difficult to avoid last pixel
			pRenderTarget->DrawLine(D2D1::Point2F(x + 0.5, y + 0.5), 
				D2D1::Point2F(x_ + 0.5, y_ + 0.5), pBrush);
		}
		x = x_;
		y = y_;
	}
}

void SurfaceImpl::Polygon(Point *pts, int npts, ColourAllocated fore, ColourAllocated back) {
	if (pRenderTarget) {
		ID2D1Factory *pFactory = 0;
		pRenderTarget->GetFactory(&pFactory);
		ID2D1PathGeometry *geometry=0;
		HRESULT hr = pFactory->CreatePathGeometry(&geometry);
		if (SUCCEEDED(hr)) {
			ID2D1GeometrySink *sink = 0;
			hr = geometry->Open(&sink);
			if (SUCCEEDED(hr)) {
				sink->BeginFigure(D2D1::Point2F(pts[0].x + 0.5f, pts[0].y + 0.5f), D2D1_FIGURE_BEGIN_FILLED);
				for (size_t i=1; i<static_cast<size_t>(npts); i++) {
					sink->AddLine(D2D1::Point2F(pts[i].x + 0.5f, pts[i].y + 0.5f));
				}
				sink->EndFigure(D2D1_FIGURE_END_CLOSED);
				sink->Close();
				sink->Release();

				D2DPenColour(back);
				pRenderTarget->FillGeometry(geometry,pBrush);
				D2DPenColour(fore);
				pRenderTarget->DrawGeometry(geometry,pBrush);
			}

			geometry->Release();
		}
	}
}

void SurfaceImpl::RectangleDraw(PRectangle rc, ColourAllocated fore, ColourAllocated back) {
	if (pRenderTarget) {
		D2DPenColour(back);
		D2D1_RECT_F rectangle1 = D2D1::RectF(RoundFloat(rc.left) + 0.5, rc.top+0.5, RoundFloat(rc.right) - 0.5, rc.bottom-0.5);
		D2DPenColour(back);
		pRenderTarget->FillRectangle(&rectangle1, pBrush);
		D2DPenColour(fore);
		pRenderTarget->DrawRectangle(&rectangle1, pBrush);
	}
}

void SurfaceImpl::FillRectangle(PRectangle rc, ColourAllocated back) {
	if (pRenderTarget) {
		D2DPenColour(back);
        D2D1_RECT_F rectangle1 = D2D1::RectF(RoundFloat(rc.left), rc.top, RoundFloat(rc.right), rc.bottom);
        pRenderTarget->FillRectangle(&rectangle1, pBrush);
	}
}

void SurfaceImpl::FillRectangle(PRectangle rc, Surface &surfacePattern) {
	SurfaceImpl &surfOther = static_cast<SurfaceImpl &>(surfacePattern);
	surfOther.FlushDrawing();
	ID2D1Bitmap *pBitmap = NULL;
	ID2D1BitmapRenderTarget *pCompatibleRenderTarget = reinterpret_cast<ID2D1BitmapRenderTarget *>(
		surfOther.pRenderTarget);
	HRESULT hr = pCompatibleRenderTarget->GetBitmap(&pBitmap);
	if (SUCCEEDED(hr)) {
		ID2D1BitmapBrush *pBitmapBrush = NULL;
		D2D1_BITMAP_BRUSH_PROPERTIES brushProperties =
	        D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP,
			D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		// Create the bitmap brush.
		hr = pRenderTarget->CreateBitmapBrush(pBitmap, brushProperties, &pBitmapBrush);
		pBitmap->Release();
		if (SUCCEEDED(hr)) {
			pRenderTarget->FillRectangle(
				D2D1::RectF(rc.left, rc.top, rc.right, rc.bottom),
				pBitmapBrush);
			pBitmapBrush->Release();
		}
	}
}

void SurfaceImpl::RoundedRectangle(PRectangle rc, ColourAllocated fore, ColourAllocated back) {
	if (pRenderTarget) {
		D2D1_ROUNDED_RECT roundedRectFill = D2D1::RoundedRect(
			D2D1::RectF(rc.left+1.0, rc.top+1.0, rc.right-1.0, rc.bottom-1.0),
			8, 8);
		D2DPenColour(back);
		pRenderTarget->FillRoundedRectangle(roundedRectFill, pBrush);

		D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
			D2D1::RectF(rc.left + 0.5, rc.top+0.5, rc.right - 0.5, rc.bottom-0.5),
			8, 8);
		D2DPenColour(fore);
		pRenderTarget->DrawRoundedRectangle(roundedRect, pBrush);
	}
}

void SurfaceImpl::AlphaRectangle(PRectangle rc, int cornerSize, ColourAllocated fill, int alphaFill,
		ColourAllocated outline, int alphaOutline, int /* flags*/ ) {
	if (pRenderTarget) {
		D2D1_ROUNDED_RECT roundedRectFill = D2D1::RoundedRect(
			D2D1::RectF(rc.left+1.0, rc.top+1.0, rc.right-1.0, rc.bottom-1.0),
			cornerSize, cornerSize);
		D2DPenColour(fill, alphaFill);
		pRenderTarget->FillRoundedRectangle(roundedRectFill, pBrush);

		D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
			D2D1::RectF(rc.left + 0.5, rc.top+0.5, rc.right - 0.5, rc.bottom-0.5),
			cornerSize, cornerSize);
		D2DPenColour(outline, alphaOutline);
		pRenderTarget->DrawRoundedRectangle(roundedRect, pBrush);
	}
}

void SurfaceImpl::DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage) {
	if (pRenderTarget) {
		if (rc.Width() > width)
			rc.left += (rc.Width() - width) / 2;
		rc.right = rc.left + width;
		if (rc.Height() > height)
			rc.top += (rc.Height() - height) / 2;
		rc.bottom = rc.top + height;

		std::vector<unsigned char> image(height * width * 4);
		for (int y=0; y<height; y++) {
			for (int x=0; x<width; x++) {
				unsigned char *pixel = &image[0] + (y*width+x) * 4;
				unsigned char alpha = pixelsImage[3];
				// Input is RGBA, output is BGRA with premultiplied alpha
				pixel[2] = (*pixelsImage++) * alpha / 255;
				pixel[1] = (*pixelsImage++) * alpha / 255;
				pixel[0] = (*pixelsImage++) * alpha / 255;
				pixel[3] = *pixelsImage++;
			}
		}

		ID2D1Bitmap *bitmap = 0;
		D2D1_SIZE_U size = D2D1::SizeU(width, height);
		D2D1_BITMAP_PROPERTIES props = {{DXGI_FORMAT_B8G8R8A8_UNORM,	
		    D2D1_ALPHA_MODE_PREMULTIPLIED}, 72.0, 72.0};
		HRESULT hr = pRenderTarget->CreateBitmap(size, &image[0],
                  width * 4, &props, &bitmap);
		if (SUCCEEDED(hr)) {
			D2D1_RECT_F rcDestination = {rc.left, rc.top, rc.right, rc.bottom};
			pRenderTarget->DrawBitmap(bitmap, rcDestination);
		}
		bitmap->Release();
	}
}

void SurfaceImpl::Ellipse(PRectangle rc, ColourAllocated fore, ColourAllocated back) {
	if (pRenderTarget) {
		FLOAT radius = rc.Width() / 2.0f - 1.0f;
		D2D1_ELLIPSE ellipse = D2D1::Ellipse(
			D2D1::Point2F((rc.left + rc.right) / 2.0f, (rc.top + rc.bottom) / 2.0f),
			radius,radius);

		PenColour(back);
		pRenderTarget->FillEllipse(ellipse, pBrush);
		PenColour(fore);
		pRenderTarget->DrawEllipse(ellipse, pBrush);
	}
}

void SurfaceImpl::Copy(PRectangle rc, Point from, Surface &surfaceSource) {
	SurfaceImpl &surfOther = static_cast<SurfaceImpl &>(surfaceSource);
	surfOther.FlushDrawing();
	ID2D1BitmapRenderTarget *pCompatibleRenderTarget = reinterpret_cast<ID2D1BitmapRenderTarget *>(
		surfOther.pRenderTarget);
	ID2D1Bitmap *pBitmap = NULL;
	HRESULT hr = pCompatibleRenderTarget->GetBitmap(&pBitmap);
	if (SUCCEEDED(hr)) {
		D2D1_RECT_F rcDestination = {rc.left, rc.top, rc.right, rc.bottom};
		D2D1_RECT_F rcSource = {from.x, from.y, from.x + rc.Width(), from.y + rc.Height()};
		pRenderTarget->DrawBitmap(pBitmap, rcDestination, 1.0f, 
			D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, rcSource);
		pRenderTarget->Flush();
		pBitmap->Release();
	}
}

// Buffer to hold strings and string position arrays without always allocating on heap.
// May sometimes have string too long to allocate on stack. So use a fixed stack-allocated buffer
// when less than safe size otherwise allocate on heap and free automatically.
template<typename T, int lengthStandard>
class VarBuffer {
	T bufferStandard[lengthStandard];
public:
	T *buffer;
	VarBuffer(size_t length) : buffer(0) {
		if (length > lengthStandard) {
			buffer = new T[length];
		} else {
			buffer = bufferStandard;
		}
	}
	~VarBuffer() {
		if (buffer != bufferStandard) {
			delete []buffer;
			buffer = 0;
		}
	}
};

const int stackBufferLength = 10000;
class TextWide : public VarBuffer<wchar_t, stackBufferLength> {
public:
	int tlen;
	TextWide(const char *s, int len, bool unicodeMode, int codePage=0) :
		VarBuffer<wchar_t, stackBufferLength>(len) {
		if (unicodeMode) {
			tlen = UTF16FromUTF8(s, len, buffer, len);
		} else {
			// Support Asian string display in 9x English
			tlen = ::MultiByteToWideChar(codePage, 0, s, len, buffer, len);
		}
	}
};
typedef VarBuffer<XYPOSITION, stackBufferLength> TextPositions;

void SurfaceImpl::DrawTextCommon(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, UINT) {
	SetFont(font_);
	RECT rcw = RectFromPRectangle(rc);

	// Use Unicode calls
	const TextWide tbuf(s, len, unicodeMode, codePage);
	if (pRenderTarget && pTextFormat && pBrush) {
		
		// Explicitly creating a text layout appears a little faster 
		IDWriteTextLayout *pTextLayout;
		HRESULT hr = pIDWriteFactory->CreateTextLayout(tbuf.buffer, tbuf.tlen, pTextFormat,
				rc.Width()+2, rc.Height(), &pTextLayout);
		if (SUCCEEDED(hr)) {
			D2D1_POINT_2F origin = {rc.left, ybase-baseline};
			pRenderTarget->DrawTextLayout(origin, pTextLayout, pBrush, D2D1_DRAW_TEXT_OPTIONS_NONE);
		} else {
			D2D1_RECT_F layoutRect = D2D1::RectF(
				static_cast<FLOAT>(rcw.left) / dpiScaleX,
				static_cast<FLOAT>(ybase-baseline) / dpiScaleY,
				static_cast<FLOAT>(rcw.right + 1) / dpiScaleX,
				static_cast<FLOAT>(rcw.bottom) / dpiScaleY);
			pRenderTarget->DrawText(tbuf.buffer, tbuf.tlen, pTextFormat, layoutRect, pBrush, D2D1_DRAW_TEXT_OPTIONS_NONE);
		}
	}
}

void SurfaceImpl::DrawTextNoClip(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len,
	ColourAllocated fore, ColourAllocated back) {
	if (pRenderTarget) {
		FillRectangle(rc, back);
		D2DPenColour(fore);
		DrawTextCommon(rc, font_, ybase, s, len, ETO_OPAQUE);
	}
}

void SurfaceImpl::DrawTextClipped(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len,
	ColourAllocated fore, ColourAllocated back) {
	if (pRenderTarget) {
		FillRectangle(rc, back);
		D2DPenColour(fore);
		DrawTextCommon(rc, font_, ybase, s, len, ETO_OPAQUE | ETO_CLIPPED);
	}
}

void SurfaceImpl::DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len,
	ColourAllocated fore) {
	// Avoid drawing spaces in transparent mode
	for (int i=0;i<len;i++) {
		if (s[i] != ' ') {
			if (pRenderTarget) {
				D2DPenColour(fore);
				DrawTextCommon(rc, font_, ybase, s, len, 0);
			}
			return;
		}
	}
}

XYPOSITION SurfaceImpl::WidthText(Font &font_, const char *s, int len) {
	FLOAT width = 1.0;
	SetFont(font_);
	const TextWide tbuf(s, len, unicodeMode, codePage);
	if (pIDWriteFactory && pTextFormat) {
		// Create a layout
		IDWriteTextLayout *pTextLayout = 0;
		HRESULT hr = pIDWriteFactory->CreateTextLayout(tbuf.buffer, tbuf.tlen, pTextFormat, 1000.0, 1000.0, &pTextLayout);
		if (SUCCEEDED(hr)) {
			DWRITE_TEXT_METRICS textMetrics;
			pTextLayout->GetMetrics(&textMetrics);
			width = textMetrics.widthIncludingTrailingWhitespace;
			pTextLayout->Release();
		}
	}
	return int(width + 0.5);
}

void SurfaceImpl::MeasureWidths(Font &font_, const char *s, int len, XYPOSITION *positions) {
	SetFont(font_);
	int fit = 0;
	const TextWide tbuf(s, len, unicodeMode, codePage);
	TextPositions poses(tbuf.tlen);
	fit = tbuf.tlen;
	const int clusters = 1000;
	DWRITE_CLUSTER_METRICS clusterMetrics[clusters];
	UINT32 count = 0;
	if (pIDWriteFactory && pTextFormat) {
		SetFont(font_);
		// Create a layout
		IDWriteTextLayout *pTextLayout = 0;
		HRESULT hr = pIDWriteFactory->CreateTextLayout(tbuf.buffer, tbuf.tlen, pTextFormat, 10000.0, 1000.0, &pTextLayout);
		if (!SUCCEEDED(hr))
			return;
		// For now, assuming WCHAR == cluster
		pTextLayout->GetClusterMetrics(clusterMetrics, clusters, &count);
		FLOAT position = 0.0f;
		size_t ti=0;
		for (size_t ci=0;ci<count;ci++) {
			position += clusterMetrics[ci].width;
			for (size_t inCluster=0; inCluster<clusterMetrics[ci].length; inCluster++) {
				//poses.buffer[ti++] = int(position + 0.5);
				poses.buffer[ti++] = position;
			}
		}
		PLATFORM_ASSERT(ti == static_cast<size_t>(tbuf.tlen));
		pTextLayout->Release();
	}
	if (unicodeMode) {
		// Map the widths given for UTF-16 characters back onto the UTF-8 input string
		int ui=0;
		const unsigned char *us = reinterpret_cast<const unsigned char *>(s);
		int i=0;
		while (ui<fit) {
			unsigned char uch = us[i];
			unsigned int lenChar = 1;
			if (uch >= (0x80 + 0x40 + 0x20 + 0x10)) {
				lenChar = 4;
				ui++;
			} else if (uch >= (0x80 + 0x40 + 0x20)) {
				lenChar = 3;
			} else if (uch >= (0x80)) {
				lenChar = 2;
			}
			for (unsigned int bytePos=0; (bytePos<lenChar) && (i<len); bytePos++) {
				positions[i++] = poses.buffer[ui];
			}
			ui++;
		}
		int lastPos = 0;
		if (i > 0)
			lastPos = positions[i-1];
		while (i<len) {
			positions[i++] = lastPos;
		}
	} else if (codePage == 0) {

		// One character per position
		PLATFORM_ASSERT(len == tbuf.tlen);
		for (size_t kk=0;kk<static_cast<size_t>(len);kk++) {
			positions[kk] = poses.buffer[kk];
		}

	} else {

		// May be more than one byte per position
		int ui = 0;
		for (int i=0;i<len;) {
			if (::IsDBCSLeadByteEx(codePage, s[i])) {
				positions[i] = poses.buffer[ui];
				positions[i+1] = poses.buffer[ui];
				i += 2;
			} else {
				positions[i] = poses.buffer[ui];
				i++;
			}

			ui++;
		}
	}
}

XYPOSITION SurfaceImpl::WidthChar(Font &font_, char ch) {
	FLOAT width = 1.0;
	SetFont(font_);
	if (pIDWriteFactory && pTextFormat) {
		// Create a layout
		IDWriteTextLayout *pTextLayout = 0;
		const WCHAR wch = ch;
		HRESULT hr = pIDWriteFactory->CreateTextLayout(&wch, 1, pTextFormat, 1000.0, 1000.0, &pTextLayout);
		if (SUCCEEDED(hr)) {
			DWRITE_TEXT_METRICS textMetrics;
			pTextLayout->GetMetrics(&textMetrics);
			width = textMetrics.widthIncludingTrailingWhitespace;
			pTextLayout->Release();
		}
	}
	return int(width + 0.5);
}

XYPOSITION SurfaceImpl::Ascent(Font &font_) {
	FLOAT ascent = 1.0;
	SetFont(font_);
	if (pIDWriteFactory && pTextFormat) {
		SetFont(font_);
		// Create a layout
		IDWriteTextLayout *pTextLayout = 0;
		HRESULT hr = pIDWriteFactory->CreateTextLayout(L"X", 1, pTextFormat, 1000.0, 1000.0, &pTextLayout);
		if (SUCCEEDED(hr)) {
			DWRITE_TEXT_METRICS textMetrics;
			pTextLayout->GetMetrics(&textMetrics);
			ascent = textMetrics.layoutHeight;
			const int clusters = 20;
			DWRITE_CLUSTER_METRICS clusterMetrics[clusters];
			UINT32 count = 0;
			pTextLayout->GetClusterMetrics(clusterMetrics, clusters, &count);
			//height = pTextLayout->GetMaxHeight();
			FLOAT minWidth = 0;
			hr = pTextLayout->DetermineMinWidth(&minWidth);
			const int maxLines = 2;
			DWRITE_LINE_METRICS lineMetrics[maxLines];
			UINT32 lineCount = 0;
			hr = pTextLayout->GetLineMetrics(lineMetrics, maxLines, &lineCount);
			if (SUCCEEDED(hr)) {
				ascent = lineMetrics[0].baseline;
			}
			pTextLayout->Release();
		}
	}
	return int(ascent + 0.5);
}

XYPOSITION SurfaceImpl::Descent(Font &font_) {
	FLOAT descent = 1.0;
	SetFont(font_);
	if (pIDWriteFactory && pTextFormat) {
		// Create a layout
		IDWriteTextLayout *pTextLayout = 0;
		HRESULT hr = pIDWriteFactory->CreateTextLayout(L"X", 1, pTextFormat, 1000.0, 1000.0, &pTextLayout);
		if (SUCCEEDED(hr)) {
			const int maxLines = 2;
			DWRITE_LINE_METRICS lineMetrics[maxLines];
			UINT32 lineCount = 0;
			hr = pTextLayout->GetLineMetrics(lineMetrics, maxLines, &lineCount);
			if (SUCCEEDED(hr)) {
				descent = lineMetrics[0].height - lineMetrics[0].baseline;
			}
			pTextLayout->Release();
		}
	}
	return int(descent + 0.5);
}

XYPOSITION SurfaceImpl::InternalLeading(Font &) {
	return 0;
}

XYPOSITION SurfaceImpl::ExternalLeading(Font &) {
	return 1;
}

XYPOSITION SurfaceImpl::Height(Font &font_) {
	FLOAT height = 1.0;
	SetFont(font_);
	if (pIDWriteFactory && pTextFormat) {
		// Create a layout
		IDWriteTextLayout *pTextLayout = 0;
		HRESULT hr = pIDWriteFactory->CreateTextLayout(L"X", 1, pTextFormat, 1000.0, 1000.0, &pTextLayout);
		if (SUCCEEDED(hr)) {
			const int maxLines = 2;
			DWRITE_LINE_METRICS lineMetrics[maxLines];
			UINT32 lineCount = 0;
			hr = pTextLayout->GetLineMetrics(lineMetrics, maxLines, &lineCount);
			if (SUCCEEDED(hr)) {
				height = lineMetrics[0].height;
			}
			pTextLayout->Release();
		}
	}
	// Truncating rather than rounding as otherwise too much space.
	return int(height);
}

XYPOSITION SurfaceImpl::AverageCharWidth(Font &font_) {
	FLOAT width = 1.0;
	SetFont(font_);
	if (pIDWriteFactory && pTextFormat) {
		// Create a layout
		IDWriteTextLayout *pTextLayout = 0;
		const WCHAR wszAllAlpha[] = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		HRESULT hr = pIDWriteFactory->CreateTextLayout(wszAllAlpha, wcslen(wszAllAlpha), pTextFormat, 1000.0, 1000.0, &pTextLayout);
		if (SUCCEEDED(hr)) {
			DWRITE_TEXT_METRICS textMetrics;
			pTextLayout->GetMetrics(&textMetrics);
			width = textMetrics.width / wcslen(wszAllAlpha);
			pTextLayout->Release();
		}
	}
	return int(width + 0.5);
}

int SurfaceImpl::SetPalette(Palette *, bool) {
	return 0;
}

void SurfaceImpl::SetClip(PRectangle rc) {
	if (pRenderTarget) {
		D2D1_RECT_F rcClip = {rc.left, rc.top, rc.right, rc.bottom};
		pRenderTarget->PushAxisAlignedClip(rcClip, D2D1_ANTIALIAS_MODE_ALIASED);
		clipsActive++;
	}
}

void SurfaceImpl::FlushCachedState() {
}

void SurfaceImpl::SetUnicodeMode(bool unicodeMode_) {
	unicodeMode=unicodeMode_;
}

void SurfaceImpl::SetDBCSMode(int codePage_) {
	// No action on window as automatically handled by system.
	codePage = codePage_;
	win9xACPSame = !IsNT() && ((unsigned int)codePage == ::GetACP());
}

Surface *Surface::Allocate() {
	return new SurfaceImpl;
}

Window::~Window() {
}

void Window::Destroy() {
	if (wid)
		::DestroyWindow(reinterpret_cast<HWND>(wid));
	wid = 0;
}

bool Window::HasFocus() {
	return ::GetFocus() == wid;
}

PRectangle Window::GetPosition() {
	RECT rc;
	::GetWindowRect(reinterpret_cast<HWND>(wid), &rc);
	return PRectangle(rc.left, rc.top, rc.right, rc.bottom);
}

void Window::SetPosition(PRectangle rc) {
	::SetWindowPos(reinterpret_cast<HWND>(wid),
		0, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER|SWP_NOACTIVATE);
}

void Window::SetPositionRelative(PRectangle rc, Window w) {
	LONG style = ::GetWindowLong(reinterpret_cast<HWND>(wid), GWL_STYLE);
	if (style & WS_POPUP) {
		POINT ptOther = {0, 0};
		::ClientToScreen(reinterpret_cast<HWND>(w.GetID()), &ptOther);
		rc.Move(ptOther.x, ptOther.y);

		// This #ifdef is for VC 98 which has problems with MultiMon.h under some conditions.
#ifdef MONITOR_DEFAULTTONULL
		// We're using the stub functionality of MultiMon.h to decay gracefully on machines
		// (ie, pre Win2000, Win95) that do not support the newer functions.
		RECT rcMonitor = {rc.left, rc.top, rc.right, rc.bottom};
		MONITORINFO mi = {0};
		mi.cbSize = sizeof(mi);

		HMONITOR hMonitor = ::MonitorFromRect(&rcMonitor, MONITOR_DEFAULTTONEAREST);
		// If hMonitor is NULL, that's just the main screen anyways.
		::GetMonitorInfo(hMonitor, &mi);

		// Now clamp our desired rectangle to fit inside the work area
		// This way, the menu will fit wholly on one screen. An improvement even
		// if you don't have a second monitor on the left... Menu's appears half on
		// one screen and half on the other are just U.G.L.Y.!
		if (rc.right > mi.rcWork.right)
			rc.Move(mi.rcWork.right - rc.right, 0);
		if (rc.bottom > mi.rcWork.bottom)
			rc.Move(0, mi.rcWork.bottom - rc.bottom);
		if (rc.left < mi.rcWork.left)
			rc.Move(mi.rcWork.left - rc.left, 0);
		if (rc.top < mi.rcWork.top)
			rc.Move(0, mi.rcWork.top - rc.top);
#endif
	}
	SetPosition(rc);
}

PRectangle Window::GetClientPosition() {
	RECT rc={0,0,0,0};
	if (wid)
		::GetClientRect(reinterpret_cast<HWND>(wid), &rc);
	return  PRectangle(rc.left, rc.top, rc.right, rc.bottom);
}

void Window::Show(bool show) {
	if (show)
		::ShowWindow(reinterpret_cast<HWND>(wid), SW_SHOWNOACTIVATE);
	else
		::ShowWindow(reinterpret_cast<HWND>(wid), SW_HIDE);
}

void Window::InvalidateAll() {
	::InvalidateRect(reinterpret_cast<HWND>(wid), NULL, FALSE);
}

void Window::InvalidateRectangle(PRectangle rc) {
	RECT rcw = RectFromPRectangle(rc);
	::InvalidateRect(reinterpret_cast<HWND>(wid), &rcw, FALSE);
}

static LRESULT Window_SendMessage(Window *w, UINT msg, WPARAM wParam=0, LPARAM lParam=0) {
	return ::SendMessage(reinterpret_cast<HWND>(w->GetID()), msg, wParam, lParam);
}

void Window::SetFont(Font &font) {
	Window_SendMessage(this, WM_SETFONT,
		reinterpret_cast<WPARAM>(font.GetID()), 0);
}

static void FlipBitmap(HBITMAP bitmap, int width, int height) {
	HDC hdc = ::CreateCompatibleDC(NULL);
	if (hdc != NULL) {
		HGDIOBJ prevBmp = ::SelectObject(hdc, bitmap);
		::StretchBlt(hdc, width - 1, 0, -width, height, hdc, 0, 0, width, height, SRCCOPY);
		::SelectObject(hdc, prevBmp);
		::DeleteDC(hdc);
	}
}

static HCURSOR GetReverseArrowCursor() {
	if (reverseArrowCursor != NULL)
		return reverseArrowCursor;

	::EnterCriticalSection(&crPlatformLock);
	HCURSOR cursor = reverseArrowCursor;
	if (cursor == NULL) {
		cursor = ::LoadCursor(NULL, IDC_ARROW);
		ICONINFO info;
		if (::GetIconInfo(cursor, &info)) {
			BITMAP bmp;
			if (::GetObject(info.hbmMask, sizeof(bmp), &bmp)) {
				FlipBitmap(info.hbmMask, bmp.bmWidth, bmp.bmHeight);
				if (info.hbmColor != NULL)
					FlipBitmap(info.hbmColor, bmp.bmWidth, bmp.bmHeight);
				info.xHotspot = (DWORD)bmp.bmWidth - 1 - info.xHotspot;

				reverseArrowCursor = ::CreateIconIndirect(&info);
				if (reverseArrowCursor != NULL)
					cursor = reverseArrowCursor;
			}

			::DeleteObject(info.hbmMask);
			if (info.hbmColor != NULL)
				::DeleteObject(info.hbmColor);
		}
	}
	::LeaveCriticalSection(&crPlatformLock);
	return cursor;
}

void Window::SetCursor(Cursor curs) {
	switch (curs) {
	case cursorText:
		::SetCursor(::LoadCursor(NULL,IDC_IBEAM));
		break;
	case cursorUp:
		::SetCursor(::LoadCursor(NULL,IDC_UPARROW));
		break;
	case cursorWait:
		::SetCursor(::LoadCursor(NULL,IDC_WAIT));
		break;
	case cursorHoriz:
		::SetCursor(::LoadCursor(NULL,IDC_SIZEWE));
		break;
	case cursorVert:
		::SetCursor(::LoadCursor(NULL,IDC_SIZENS));
		break;
	case cursorHand:
		::SetCursor(::LoadCursor(NULL,IDC_HAND));
		break;
	case cursorReverseArrow:
		::SetCursor(GetReverseArrowCursor());
		break;
	case cursorArrow:
	case cursorInvalid:	// Should not occur, but just in case.
		::SetCursor(::LoadCursor(NULL,IDC_ARROW));
		break;
	}
}

void Window::SetTitle(const char *s) {
	::SetWindowTextA(reinterpret_cast<HWND>(wid), s);
}

/* Returns rectangle of monitor pt is on, both rect and pt are in Window's
   coordinates */
PRectangle Window::GetMonitorRect(Point pt) {
#ifdef MONITOR_DEFAULTTONULL
	// MonitorFromPoint and GetMonitorInfo are not available on Windows 95 so are not used.
	// There could be conditional code and dynamic loading in a future version
	// so this would work on those platforms where they are available.
	PRectangle rcPosition = GetPosition();
	POINT ptDesktop = {pt.x + rcPosition.left, pt.y + rcPosition.top};
	HMONITOR hMonitor = ::MonitorFromPoint(ptDesktop, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = {0};
	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	if (::GetMonitorInfo(hMonitor, &mi)) {
		PRectangle rcMonitor(
			mi.rcWork.left - rcPosition.left,
			mi.rcWork.top - rcPosition.top,
			mi.rcWork.right - rcPosition.left,
			mi.rcWork.bottom - rcPosition.top);
		return rcMonitor;
	} else {
		return PRectangle();
	}
#else
	return PRectangle();
#endif
}

struct ListItemData {
	const char *text;
	int pixId;
};

#define _ROUND2(n,pow2) \
	( ( (n) + (pow2) - 1) & ~((pow2) - 1) )

class LineToItem {
	char *words;
	int wordsCount;
	int wordsSize;

	ListItemData *data;
	int len;
	int count;

private:
	void FreeWords() {
		delete []words;
		words = NULL;
		wordsCount = 0;
		wordsSize = 0;
	}
	char *AllocWord(const char *word);

public:
	LineToItem() : words(NULL), wordsCount(0), wordsSize(0), data(NULL), len(0), count(0) {
	}
	~LineToItem() {
		Clear();
	}
	void Clear() {
		FreeWords();
		delete []data;
		data = NULL;
		len = 0;
		count = 0;
	}

	ListItemData *Append(const char *text, int value);

	ListItemData Get(int index) const {
		if (index >= 0 && index < count) {
			return data[index];
		} else {
			ListItemData missing = {"", -1};
			return missing;
		}
	}
	int Count() const {
		return count;
	}

	ListItemData *AllocItem();

	void SetWords(char *s) {
		words = s;	// N.B. will be deleted on destruction
	}
};

char *LineToItem::AllocWord(const char *text) {
	int chars = static_cast<int>(strlen(text) + 1);
	int newCount = wordsCount + chars;
	if (newCount > wordsSize) {
		wordsSize = _ROUND2(newCount * 2, 8192);
		char *wordsNew = new char[wordsSize];
		memcpy(wordsNew, words, wordsCount);
		int offset = wordsNew - words;
		for (int i=0; i<count; i++)
			data[i].text += offset;
		delete []words;
		words = wordsNew;
	}
	char *s = &words[wordsCount];
	wordsCount = newCount;
	strncpy(s, text, chars);
	return s;
}

ListItemData *LineToItem::AllocItem() {
	if (count >= len) {
		int lenNew = _ROUND2((count+1) * 2, 1024);
		ListItemData *dataNew = new ListItemData[lenNew];
		memcpy(dataNew, data, count * sizeof(ListItemData));
		delete []data;
		data = dataNew;
		len = lenNew;
	}
	ListItemData *item = &data[count];
	count++;
	return item;
}

ListItemData *LineToItem::Append(const char *text, int imageIndex) {
	ListItemData *item = AllocItem();
	item->text = AllocWord(text);
	item->pixId = imageIndex;
	return item;
}

const TCHAR ListBoxX_ClassName[] = TEXT("ListBoxX");

ListBox::ListBox() {
}

ListBox::~ListBox() {
}

class ListBoxX : public ListBox {
	int lineHeight;
	FontID fontCopy;
	RGBAImageSet images;
	LineToItem lti;
	HWND lb;
	bool unicodeMode;
	int desiredVisibleRows;
	unsigned int maxItemCharacters;
	unsigned int aveCharWidth;
	Window *parent;
	int ctrlID;
	CallBackAction doubleClickAction;
	void *doubleClickActionData;
	const char *widestItem;
	unsigned int maxCharWidth;
	int resizeHit;
	PRectangle rcPreSize;
	Point dragOffset;
	Point location;	// Caret location at which the list is opened

	HWND GetHWND() const;
	void AppendListItem(const char *startword, const char *numword);
	void AdjustWindowRect(PRectangle *rc) const;
	int ItemHeight() const;
	int MinClientWidth() const;
	int TextOffset() const;
	Point GetClientExtent() const;
	POINT MinTrackSize() const;
	POINT MaxTrackSize() const;
	void SetRedraw(bool on);
	void OnDoubleClick();
	void ResizeToCursor();
	void StartResize(WPARAM);
	int NcHitTest(WPARAM, LPARAM) const;
	void CentreItem(int);
	void Paint(HDC);
	static LRESULT PASCAL ControlWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	static const Point ItemInset;	// Padding around whole item
	static const Point TextInset;	// Padding around text
	static const Point ImageInset;	// Padding around image

public:
	ListBoxX() : lineHeight(10), fontCopy(0), lb(0), unicodeMode(false),
		desiredVisibleRows(5), maxItemCharacters(0), aveCharWidth(8),
		parent(NULL), ctrlID(0), doubleClickAction(NULL), doubleClickActionData(NULL),
		widestItem(NULL), maxCharWidth(1), resizeHit(0) {
	}
	virtual ~ListBoxX() {
		if (fontCopy) {
			::DeleteObject(fontCopy);
			fontCopy = 0;
		}
	}
	virtual void SetFont(Font &font);
	virtual void Create(Window &parent, int ctrlID, Point location_, int lineHeight_, bool unicodeMode_);
	virtual void SetAverageCharWidth(int width);
	virtual void SetVisibleRows(int rows);
	virtual int GetVisibleRows() const;
	virtual PRectangle GetDesiredRect();
	virtual int CaretFromEdge();
	virtual void Clear();
	virtual void Append(char *s, int type = -1);
	virtual int Length();
	virtual void Select(int n);
	virtual int GetSelection();
	virtual int Find(const char *prefix);
	virtual void GetValue(int n, char *value, int len);
	virtual void RegisterImage(int type, const char *xpm_data);
	virtual void RegisterRGBAImage(int type, int width, int height, const unsigned char *pixelsImage);
	virtual void ClearRegisteredImages();
	virtual void SetDoubleClickAction(CallBackAction action, void *data) {
		doubleClickAction = action;
		doubleClickActionData = data;
	}
	virtual void SetList(const char *list, char separator, char typesep);
	void Draw(DRAWITEMSTRUCT *pDrawItem);
	LRESULT WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
	static LRESULT PASCAL StaticWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
};

const Point ListBoxX::ItemInset(0, 0);
const Point ListBoxX::TextInset(2, 0);
const Point ListBoxX::ImageInset(1, 0);

ListBox *ListBox::Allocate() {
	ListBoxX *lb = new ListBoxX();
	return lb;
}

void ListBoxX::Create(Window &parent_, int ctrlID_, Point location_, int lineHeight_, bool unicodeMode_) {
	parent = &parent_;
	ctrlID = ctrlID_;
	location = location_;
	lineHeight = lineHeight_;
	unicodeMode = unicodeMode_;
	HWND hwndParent = reinterpret_cast<HWND>(parent->GetID());
	HINSTANCE hinstanceParent = GetWindowInstance(hwndParent);
	// Window created as popup so not clipped within parent client area
	wid = ::CreateWindowEx(
		WS_EX_WINDOWEDGE, ListBoxX_ClassName, TEXT(""),
		WS_POPUP | WS_THICKFRAME,
		100,100, 150,80, hwndParent,
		NULL,
		hinstanceParent,
		this);

	POINT locationw = {location.x, location.y};
	::MapWindowPoints(hwndParent, NULL, &locationw, 1);
	location = Point(locationw.x, locationw.y);
}

void ListBoxX::SetFont(Font &font) {
	if (font.GetID()) {
		if (fontCopy) {
			::DeleteObject(fontCopy);
			fontCopy = 0;
		}
		FormatAndBaseline *pfabl = reinterpret_cast<FormatAndBaseline *>(font.GetID());
		fontCopy = pfabl->HFont();
		::SendMessage(lb, WM_SETFONT, reinterpret_cast<WPARAM>(fontCopy), 0);
	}
}

void ListBoxX::SetAverageCharWidth(int width) {
	aveCharWidth = width;
}

void ListBoxX::SetVisibleRows(int rows) {
	desiredVisibleRows = rows;
}

int ListBoxX::GetVisibleRows() const {
	return desiredVisibleRows;
}

HWND ListBoxX::GetHWND() const {
	return reinterpret_cast<HWND>(GetID());
}

PRectangle ListBoxX::GetDesiredRect() {
	PRectangle rcDesired = GetPosition();

	int rows = Length();
	if ((rows == 0) || (rows > desiredVisibleRows))
		rows = desiredVisibleRows;
	rcDesired.bottom = rcDesired.top + ItemHeight() * rows;

	int width = MinClientWidth();
	HDC hdc = ::GetDC(lb);
	HFONT oldFont = SelectFont(hdc, fontCopy);
	SIZE textSize = {0, 0};
	int len = static_cast<int>(widestItem ? strlen(widestItem) : 0);
	if (unicodeMode) {
		const TextWide tbuf(widestItem, len, unicodeMode);
		::GetTextExtentPoint32W(hdc, tbuf.buffer, tbuf.tlen, &textSize);
	} else {
		::GetTextExtentPoint32A(hdc, widestItem, len, &textSize);
	}
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	maxCharWidth = tm.tmMaxCharWidth;
	SelectFont(hdc, oldFont);
	::ReleaseDC(lb, hdc);

	int widthDesired = Platform::Maximum(textSize.cx, (len + 1) * tm.tmAveCharWidth);
	if (width < widthDesired)
		width = widthDesired;

	rcDesired.right = rcDesired.left + TextOffset() + width + (TextInset.x * 2);
	if (Length() > rows)
		rcDesired.right += ::GetSystemMetrics(SM_CXVSCROLL);

	AdjustWindowRect(&rcDesired);
	return rcDesired;
}

int ListBoxX::TextOffset() const {
	int pixWidth = images.GetWidth();
	return pixWidth == 0 ? ItemInset.x : ItemInset.x + pixWidth + (ImageInset.x * 2);
}

int ListBoxX::CaretFromEdge() {
	PRectangle rc;
	AdjustWindowRect(&rc);
	return TextOffset() + TextInset.x + (0 - rc.left) - 1;
}

void ListBoxX::Clear() {
	::SendMessage(lb, LB_RESETCONTENT, 0, 0);
	maxItemCharacters = 0;
	widestItem = NULL;
	lti.Clear();
}

void ListBoxX::Append(char *s, int type) {
	int index = ::SendMessage(lb, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
	if (index < 0)
		return;
	ListItemData *newItem = lti.Append(s, type);
	unsigned int len = static_cast<unsigned int>(strlen(s));
	if (maxItemCharacters < len) {
		maxItemCharacters = len;
		widestItem = newItem->text;
	}
}

int ListBoxX::Length() {
	return lti.Count();
}

void ListBoxX::Select(int n) {
	// We are going to scroll to centre on the new selection and then select it, so disable
	// redraw to avoid flicker caused by a painting new selection twice in unselected and then
	// selected states
	SetRedraw(false);
	CentreItem(n);
	::SendMessage(lb, LB_SETCURSEL, n, 0);
	SetRedraw(true);
}

int ListBoxX::GetSelection() {
	return ::SendMessage(lb, LB_GETCURSEL, 0, 0);
}

// This is not actually called at present
int ListBoxX::Find(const char *) {
	return LB_ERR;
}

void ListBoxX::GetValue(int n, char *value, int len) {
	ListItemData item = lti.Get(n);
	strncpy(value, item.text, len);
	value[len-1] = '\0';
}

void ListBoxX::RegisterImage(int type, const char *xpm_data) {
	XPM xpmImage(xpm_data);
	images.Add(type, new RGBAImage(xpmImage));
}

void ListBoxX::RegisterRGBAImage(int type, int width, int height, const unsigned char *pixelsImage) {
	images.Add(type, new RGBAImage(width, height, pixelsImage));
}

void ListBoxX::ClearRegisteredImages() {
	images.Clear();
}

void ListBoxX::Draw(DRAWITEMSTRUCT *pDrawItem) {
	if ((pDrawItem->itemAction == ODA_SELECT) || (pDrawItem->itemAction == ODA_DRAWENTIRE)) {
		RECT rcBox = pDrawItem->rcItem;
		rcBox.left += TextOffset();
		if (pDrawItem->itemState & ODS_SELECTED) {
			RECT rcImage = pDrawItem->rcItem;
			rcImage.right = rcBox.left;
			// The image is not highlighted
			::FillRect(pDrawItem->hDC, &rcImage, reinterpret_cast<HBRUSH>(COLOR_WINDOW+1));
			::FillRect(pDrawItem->hDC, &rcBox, reinterpret_cast<HBRUSH>(COLOR_HIGHLIGHT+1));
			::SetBkColor(pDrawItem->hDC, ::GetSysColor(COLOR_HIGHLIGHT));
			::SetTextColor(pDrawItem->hDC, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
		} else {
			::FillRect(pDrawItem->hDC, &pDrawItem->rcItem, reinterpret_cast<HBRUSH>(COLOR_WINDOW+1));
			::SetBkColor(pDrawItem->hDC, ::GetSysColor(COLOR_WINDOW));
			::SetTextColor(pDrawItem->hDC, ::GetSysColor(COLOR_WINDOWTEXT));
		}

		ListItemData item = lti.Get(pDrawItem->itemID);
		int pixId = item.pixId;
		const char *text = item.text;
		int len = static_cast<int>(strlen(text));

		RECT rcText = rcBox;
		::InsetRect(&rcText, TextInset.x, TextInset.y);

		if (unicodeMode) {
			const TextWide tbuf(text, len, unicodeMode);
			::DrawTextW(pDrawItem->hDC, tbuf.buffer, tbuf.tlen, &rcText, DT_NOPREFIX|DT_END_ELLIPSIS|DT_SINGLELINE|DT_NOCLIP);
		} else {
			::DrawTextA(pDrawItem->hDC, text, len, &rcText, DT_NOPREFIX|DT_END_ELLIPSIS|DT_SINGLELINE|DT_NOCLIP);
		}
		if (pDrawItem->itemState & ODS_SELECTED) {
			::DrawFocusRect(pDrawItem->hDC, &rcBox);
		}

		// Draw the image, if any
		RGBAImage *pimage = images.Get(pixId);
		if (pimage) {
			Surface *surfaceItem = Surface::Allocate();
			if (surfaceItem) {
				if (pD2DFactory) {
					D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
						D2D1_RENDER_TARGET_TYPE_DEFAULT,
						D2D1::PixelFormat(
							DXGI_FORMAT_B8G8R8A8_UNORM,
							D2D1_ALPHA_MODE_IGNORE),
						0,
						0,
						D2D1_RENDER_TARGET_USAGE_NONE,
						D2D1_FEATURE_LEVEL_DEFAULT
						);
					ID2D1DCRenderTarget *pDCRT = 0;
					HRESULT hr = pD2DFactory->CreateDCRenderTarget(&props, &pDCRT);
					RECT rcWindow;
					GetClientRect(pDrawItem->hwndItem, &rcWindow);
					hr = pDCRT->BindDC(pDrawItem->hDC, &rcWindow);
					if (SUCCEEDED(hr)) {
						surfaceItem->Init(pDCRT, pDrawItem->hwndItem);
						pDCRT->BeginDraw();
						int left = pDrawItem->rcItem.left + ItemInset.x + ImageInset.x;
						PRectangle rcImage(left, pDrawItem->rcItem.top,
							left + images.GetWidth(), pDrawItem->rcItem.bottom);
						surfaceItem->DrawRGBAImage(rcImage,
							pimage->GetWidth(), pimage->GetHeight(), pimage->Pixels());
						delete surfaceItem;
						::SetTextAlign(pDrawItem->hDC, TA_TOP);
						pDCRT->EndDraw();
						pDCRT->Release();
					}
				}
			}
		}
	}
}

void ListBoxX::AppendListItem(const char *startword, const char *numword) {
	ListItemData *item = lti.AllocItem();
	item->text = startword;
	if (numword) {
		int pixId = 0;
		char ch;
		while ((ch = *++numword) != '\0') {
			pixId = 10 * pixId + (ch - '0');
		}
		item->pixId = pixId;
	} else {
		item->pixId = -1;
	}

	unsigned int len = static_cast<unsigned int>(strlen(item->text));
	if (maxItemCharacters < len) {
		maxItemCharacters = len;
		widestItem = item->text;
	}
}

void ListBoxX::SetList(const char *list, char separator, char typesep) {
	// Turn off redraw while populating the list - this has a significant effect, even if
	// the listbox is not visible.
	SetRedraw(false);
	Clear();
	size_t size = strlen(list) + 1;
	char *words = new char[size];
	lti.SetWords(words);
	memcpy(words, list, size);
	char *startword = words;
	char *numword = NULL;
	int i = 0;
	for (; words[i]; i++) {
		if (words[i] == separator) {
			words[i] = '\0';
			if (numword)
				*numword = '\0';
			AppendListItem(startword, numword);
			startword = words + i + 1;
			numword = NULL;
		} else if (words[i] == typesep) {
			numword = words + i;
		}
	}
	if (startword) {
		if (numword)
			*numword = '\0';
		AppendListItem(startword, numword);
	}

	// Finally populate the listbox itself with the correct number of items
	int count = lti.Count();
	::SendMessage(lb, LB_INITSTORAGE, count, 0);
	for (int j=0; j<count; j++) {
		::SendMessage(lb, LB_ADDSTRING, 0, j+1);
	}
	SetRedraw(true);
}

void ListBoxX::AdjustWindowRect(PRectangle *rc) const {
	RECT rcw = {rc->left, rc->top, rc->right, rc->bottom };
	::AdjustWindowRectEx(&rcw, WS_THICKFRAME, false, WS_EX_WINDOWEDGE);
	*rc = PRectangle(rcw.left, rcw.top, rcw.right, rcw.bottom);
}

int ListBoxX::ItemHeight() const {
	int itemHeight = lineHeight + (TextInset.y * 2);
	int pixHeight = images.GetHeight() + (ImageInset.y * 2);
	if (itemHeight < pixHeight) {
		itemHeight = pixHeight;
	}
	return itemHeight;
}

int ListBoxX::MinClientWidth() const {
	return 12 * (aveCharWidth+aveCharWidth/3);
}

POINT ListBoxX::MinTrackSize() const {
	PRectangle rc(0, 0, MinClientWidth(), ItemHeight());
	AdjustWindowRect(&rc);
	POINT ret = {rc.Width(), rc.Height()};
	return ret;
}

POINT ListBoxX::MaxTrackSize() const {
	PRectangle rc(0, 0,
		maxCharWidth * maxItemCharacters + TextInset.x * 2 +
		 TextOffset() + ::GetSystemMetrics(SM_CXVSCROLL),
		ItemHeight() * lti.Count());
	AdjustWindowRect(&rc);
	POINT ret = {rc.Width(), rc.Height()};
	return ret;
}

void ListBoxX::SetRedraw(bool on) {
	::SendMessage(lb, WM_SETREDRAW, static_cast<BOOL>(on), 0);
	if (on)
		::InvalidateRect(lb, NULL, TRUE);
}

void ListBoxX::ResizeToCursor() {
	PRectangle rc = GetPosition();
	POINT ptw;
	::GetCursorPos(&ptw);
	Point pt(ptw.x, ptw.y);
	pt.x += dragOffset.x;
	pt.y += dragOffset.y;

	switch (resizeHit) {
		case HTLEFT:
			rc.left = pt.x;
			break;
		case HTRIGHT:
			rc.right = pt.x;
			break;
		case HTTOP:
			rc.top = pt.y;
			break;
		case HTTOPLEFT:
			rc.top = pt.y;
			rc.left = pt.x;
			break;
		case HTTOPRIGHT:
			rc.top = pt.y;
			rc.right = pt.x;
			break;
		case HTBOTTOM:
			rc.bottom = pt.y;
			break;
		case HTBOTTOMLEFT:
			rc.bottom = pt.y;
			rc.left = pt.x;
			break;
		case HTBOTTOMRIGHT:
			rc.bottom = pt.y;
			rc.right = pt.x;
			break;
	}

	POINT ptMin = MinTrackSize();
	POINT ptMax = MaxTrackSize();
	// We don't allow the left edge to move at present, but just in case
	rc.left = Platform::Maximum(Platform::Minimum(rc.left, rcPreSize.right - ptMin.x), rcPreSize.right - ptMax.x);
	rc.top = Platform::Maximum(Platform::Minimum(rc.top, rcPreSize.bottom - ptMin.y), rcPreSize.bottom - ptMax.y);
	rc.right = Platform::Maximum(Platform::Minimum(rc.right, rcPreSize.left + ptMax.x), rcPreSize.left + ptMin.x);
	rc.bottom = Platform::Maximum(Platform::Minimum(rc.bottom, rcPreSize.top + ptMax.y), rcPreSize.top + ptMin.y);

	SetPosition(rc);
}

void ListBoxX::StartResize(WPARAM hitCode) {
	rcPreSize = GetPosition();
	POINT cursorPos;
	::GetCursorPos(&cursorPos);

	switch (hitCode) {
		case HTRIGHT:
		case HTBOTTOM:
		case HTBOTTOMRIGHT:
			dragOffset.x = rcPreSize.right - cursorPos.x;
			dragOffset.y = rcPreSize.bottom - cursorPos.y;
			break;

		case HTTOPRIGHT:
			dragOffset.x = rcPreSize.right - cursorPos.x;
			dragOffset.y = rcPreSize.top - cursorPos.y;
			break;

		// Note that the current hit test code prevents the left edge cases ever firing
		// as we don't want the left edge to be moveable
		case HTLEFT:
		case HTTOP:
		case HTTOPLEFT:
			dragOffset.x = rcPreSize.left - cursorPos.x;
			dragOffset.y = rcPreSize.top - cursorPos.y;
			break;
		case HTBOTTOMLEFT:
			dragOffset.x = rcPreSize.left - cursorPos.x;
			dragOffset.y = rcPreSize.bottom - cursorPos.y;
			break;

		default:
			return;
	}

	::SetCapture(GetHWND());
	resizeHit = hitCode;
}

int ListBoxX::NcHitTest(WPARAM wParam, LPARAM lParam) const {
	int hit = ::DefWindowProc(GetHWND(), WM_NCHITTEST, wParam, lParam);
	// There is an apparent bug in the DefWindowProc hit test code whereby it will
	// return HTTOPXXX if the window in question is shorter than the default
	// window caption height + frame, even if one is hovering over the bottom edge of
	// the frame, so workaround that here
	if (hit >= HTTOP && hit <= HTTOPRIGHT) {
		int minHeight = GetSystemMetrics(SM_CYMINTRACK);
		PRectangle rc = const_cast<ListBoxX*>(this)->GetPosition();
		int yPos = GET_Y_LPARAM(lParam);
		if ((rc.Height() < minHeight) && (yPos > ((rc.top + rc.bottom)/2))) {
			hit += HTBOTTOM - HTTOP;
		}
	}

	// Nerver permit resizing that moves the left edge. Allow movement of top or bottom edge
	// depending on whether the list is above or below the caret
	switch (hit) {
		case HTLEFT:
		case HTTOPLEFT:
		case HTBOTTOMLEFT:
			hit = HTERROR;
			break;

		case HTTOP:
		case HTTOPRIGHT: {
				PRectangle rc = const_cast<ListBoxX*>(this)->GetPosition();
				// Valid only if caret below list
				if (location.y < rc.top)
					hit = HTERROR;
			}
			break;

		case HTBOTTOM:
		case HTBOTTOMRIGHT: {
				PRectangle rc = const_cast<ListBoxX*>(this)->GetPosition();
				// Valid only if caret above list
				if (rc.bottom < location.y)
					hit = HTERROR;
			}
			break;
	}

	return hit;
}

void ListBoxX::OnDoubleClick() {

	if (doubleClickAction != NULL) {
		doubleClickAction(doubleClickActionData);
	}
}

Point ListBoxX::GetClientExtent() const {
	PRectangle rc = const_cast<ListBoxX*>(this)->GetClientPosition();
	return Point(rc.Width(), rc.Height());
}

void ListBoxX::CentreItem(int n) {
	// If below mid point, scroll up to centre, but with more items below if uneven
	if (n >= 0) {
		Point extent = GetClientExtent();
		int visible = extent.y/ItemHeight();
		if (visible < Length()) {
			int top = ::SendMessage(lb, LB_GETTOPINDEX, 0, 0);
			int half = (visible - 1) / 2;
			if (n > (top + half))
				::SendMessage(lb, LB_SETTOPINDEX, n - half , 0);
		}
	}
}

// Performs a double-buffered paint operation to avoid flicker
void ListBoxX::Paint(HDC hDC) {
	Point extent = GetClientExtent();
	HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, extent.x, extent.y);
	HDC bitmapDC = ::CreateCompatibleDC(hDC);
	HBITMAP hBitmapOld = SelectBitmap(bitmapDC, hBitmap);
	// The list background is mainly erased during painting, but can be a small
	// unpainted area when at the end of a non-integrally sized list with a
	// vertical scroll bar
	RECT rc = { 0, 0, extent.x, extent.y };
	::FillRect(bitmapDC, &rc, reinterpret_cast<HBRUSH>(COLOR_WINDOW+1));
	// Paint the entire client area and vertical scrollbar
	::SendMessage(lb, WM_PRINT, reinterpret_cast<WPARAM>(bitmapDC), PRF_CLIENT|PRF_NONCLIENT);
	::BitBlt(hDC, 0, 0, extent.x, extent.y, bitmapDC, 0, 0, SRCCOPY);
	// Select a stock brush to prevent warnings from BoundsChecker
	::SelectObject(bitmapDC, GetStockFont(WHITE_BRUSH));
	SelectBitmap(bitmapDC, hBitmapOld);
	::DeleteDC(bitmapDC);
	::DeleteObject(hBitmap);
}

LRESULT PASCAL ListBoxX::ControlWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	try {
		switch (uMsg) {
		case WM_ERASEBKGND:
			return TRUE;

		case WM_PAINT: {
				PAINTSTRUCT ps;
				HDC hDC = ::BeginPaint(hWnd, &ps);
				ListBoxX *lbx = reinterpret_cast<ListBoxX *>(PointerFromWindow(::GetParent(hWnd)));
				if (lbx)
					lbx->Paint(hDC);
				::EndPaint(hWnd, &ps);
			}
			return 0;

		case WM_MOUSEACTIVATE:
			// This prevents the view activating when the scrollbar is clicked
			return MA_NOACTIVATE;

		case WM_LBUTTONDOWN: {
				// We must take control of selection to prevent the ListBox activating
				// the popup
				LRESULT lResult = ::SendMessage(hWnd, LB_ITEMFROMPOINT, 0, lParam);
				int item = LOWORD(lResult);
				if (HIWORD(lResult) == 0 && item >= 0) {
					::SendMessage(hWnd, LB_SETCURSEL, item, 0);
				}
			}
			return 0;

		case WM_LBUTTONUP:
			return 0;

		case WM_LBUTTONDBLCLK: {
				ListBoxX *lbx = reinterpret_cast<ListBoxX *>(PointerFromWindow(::GetParent(hWnd)));
				if (lbx) {
					lbx->OnDoubleClick();
				}
			}
			return 0;
		}

		WNDPROC prevWndProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (prevWndProc) {
			return ::CallWindowProc(prevWndProc, hWnd, uMsg, wParam, lParam);
		} else {
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	} catch (...) {
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT ListBoxX::WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case WM_CREATE: {
			HINSTANCE hinstanceParent = GetWindowInstance(reinterpret_cast<HWND>(parent->GetID()));
			// Note that LBS_NOINTEGRALHEIGHT is specified to fix cosmetic issue when resizing the list
			// but has useful side effect of speeding up list population significantly
			lb = ::CreateWindowEx(
				0, TEXT("listbox"), TEXT(""),
				WS_CHILD | WS_VSCROLL | WS_VISIBLE |
				LBS_OWNERDRAWFIXED | LBS_NODATA | LBS_NOINTEGRALHEIGHT,
				0, 0, 150,80, hWnd,
				reinterpret_cast<HMENU>(ctrlID),
				hinstanceParent,
				0);
			WNDPROC prevWndProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(lb, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ControlWndProc)));
			::SetWindowLongPtr(lb, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(prevWndProc));
		}
		break;

	case WM_SIZE:
		if (lb) {
			SetRedraw(false);
			::SetWindowPos(lb, 0, 0,0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);
			// Ensure the selection remains visible
			CentreItem(GetSelection());
			SetRedraw(true);
		}
		break;

	case WM_PAINT: {
			PAINTSTRUCT ps;
			::BeginPaint(hWnd, &ps);
			::EndPaint(hWnd, &ps);
		}
		break;

	case WM_COMMAND:
		// This is not actually needed now - the registered double click action is used
		// directly to action a choice from the list.
		::SendMessage(reinterpret_cast<HWND>(parent->GetID()), iMessage, wParam, lParam);
		break;

	case WM_MEASUREITEM: {
			MEASUREITEMSTRUCT *pMeasureItem = reinterpret_cast<MEASUREITEMSTRUCT *>(lParam);
			pMeasureItem->itemHeight = static_cast<unsigned int>(ItemHeight());
		}
		break;

	case WM_DRAWITEM:
		Draw(reinterpret_cast<DRAWITEMSTRUCT *>(lParam));
		break;

	case WM_DESTROY:
		lb = 0;
		::SetWindowLong(hWnd, 0, 0);
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);

	case WM_ERASEBKGND:
		// To reduce flicker we can elide background erasure since this window is
		// completely covered by its child.
		return TRUE;

	case WM_GETMINMAXINFO: {
			MINMAXINFO *minMax = reinterpret_cast<MINMAXINFO*>(lParam);
			minMax->ptMaxTrackSize = MaxTrackSize();
			minMax->ptMinTrackSize = MinTrackSize();
		}
		break;

	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;

	case WM_NCHITTEST:
		return NcHitTest(wParam, lParam);

	case WM_NCLBUTTONDOWN:
		// We have to implement our own window resizing because the DefWindowProc
		// implementation insists on activating the resized window
		StartResize(wParam);
		return 0;

	case WM_MOUSEMOVE: {
			if (resizeHit == 0) {
				return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
			} else {
				ResizeToCursor();
			}
		}
		break;

	case WM_LBUTTONUP:
	case WM_CANCELMODE:
		if (resizeHit != 0) {
			resizeHit = 0;
			::ReleaseCapture();
		}
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);

	default:
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
	}

	return 0;
}

LRESULT PASCAL ListBoxX::StaticWndProc(
    HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	if (iMessage == WM_CREATE) {
		CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
		SetWindowPointer(hWnd, pCreate->lpCreateParams);
	}
	// Find C++ object associated with window.
	ListBoxX *lbx = reinterpret_cast<ListBoxX *>(PointerFromWindow(hWnd));
	if (lbx) {
		return lbx->WndProc(hWnd, iMessage, wParam, lParam);
	} else {
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
	}
}

static bool ListBoxX_Register() {
	WNDCLASSEX wndclassc;
	wndclassc.cbSize = sizeof(wndclassc);
	// We need CS_HREDRAW and CS_VREDRAW because of the ellipsis that might be drawn for
	// truncated items in the list and the appearance/disappearance of the vertical scroll bar.
	// The list repaint is double-buffered to avoid the flicker this would otherwise cause.
	wndclassc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wndclassc.cbClsExtra = 0;
	wndclassc.cbWndExtra = sizeof(ListBoxX *);
	wndclassc.hInstance = hinstPlatformRes;
	wndclassc.hIcon = NULL;
	wndclassc.hbrBackground = NULL;
	wndclassc.lpszMenuName = NULL;
	wndclassc.lpfnWndProc = ListBoxX::StaticWndProc;
	wndclassc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wndclassc.lpszClassName = ListBoxX_ClassName;
	wndclassc.hIconSm = 0;

	return ::RegisterClassEx(&wndclassc) != 0;
}

bool ListBoxX_Unregister() {
	return ::UnregisterClass(ListBoxX_ClassName, hinstPlatformRes) != 0;
}

Menu::Menu() : mid(0) {
}

void Menu::CreatePopUp() {
	Destroy();
	mid = ::CreatePopupMenu();
}

void Menu::Destroy() {
	if (mid)
		::DestroyMenu(reinterpret_cast<HMENU>(mid));
	mid = 0;
}

void Menu::Show(Point pt, Window &w) {
	::TrackPopupMenu(reinterpret_cast<HMENU>(mid),
		0, pt.x - 4, pt.y, 0,
		reinterpret_cast<HWND>(w.GetID()), NULL);
	Destroy();
}

static bool initialisedET = false;
static bool usePerformanceCounter = false;
static LARGE_INTEGER frequency;

ElapsedTime::ElapsedTime() {
	if (!initialisedET) {
		usePerformanceCounter = ::QueryPerformanceFrequency(&frequency) != 0;
		initialisedET = true;
	}
	if (usePerformanceCounter) {
		LARGE_INTEGER timeVal;
		::QueryPerformanceCounter(&timeVal);
		bigBit = timeVal.HighPart;
		littleBit = timeVal.LowPart;
	} else {
		bigBit = clock();
	}
}

double ElapsedTime::Duration(bool reset) {
	double result;
	long endBigBit;
	long endLittleBit;

	if (usePerformanceCounter) {
		LARGE_INTEGER lEnd;
		::QueryPerformanceCounter(&lEnd);
		endBigBit = lEnd.HighPart;
		endLittleBit = lEnd.LowPart;
		LARGE_INTEGER lBegin;
		lBegin.HighPart = bigBit;
		lBegin.LowPart = littleBit;
		double elapsed = lEnd.QuadPart - lBegin.QuadPart;
		result = elapsed / static_cast<double>(frequency.QuadPart);
	} else {
		endBigBit = clock();
		endLittleBit = 0;
		double elapsed = endBigBit - bigBit;
		result = elapsed / CLOCKS_PER_SEC;
	}
	if (reset) {
		bigBit = endBigBit;
		littleBit = endLittleBit;
	}
	return result;
}

class DynamicLibraryImpl : public DynamicLibrary {
protected:
	HMODULE h;
public:
	DynamicLibraryImpl(const char *modulePath) {
		h = ::LoadLibraryA(modulePath);
	}

	virtual ~DynamicLibraryImpl() {
		if (h != NULL)
			::FreeLibrary(h);
	}

	// Use GetProcAddress to get a pointer to the relevant function.
	virtual Function FindFunction(const char *name) {
		if (h != NULL) {
			// C++ standard doesn't like casts betwen function pointers and void pointers so use a union
			union {
				FARPROC fp;
				Function f;
			} fnConv;
			fnConv.fp = ::GetProcAddress(h, name);
			return fnConv.f;
		} else
			return NULL;
	}

	virtual bool IsValid() {
		return h != NULL;
	}
};

DynamicLibrary *DynamicLibrary::Load(const char *modulePath) {
	return static_cast<DynamicLibrary *>(new DynamicLibraryImpl(modulePath));
}

ColourDesired Platform::Chrome() {
	return ::GetSysColor(COLOR_3DFACE);
}

ColourDesired Platform::ChromeHighlight() {
	return ::GetSysColor(COLOR_3DHIGHLIGHT);
}

const char *Platform::DefaultFont() {
	return "Verdana";
}

int Platform::DefaultFontSize() {
	return 8;
}

unsigned int Platform::DoubleClickTime() {
	return ::GetDoubleClickTime();
}

bool Platform::MouseButtonBounce() {
	return false;
}

void Platform::DebugDisplay(const char *s) {
	::OutputDebugStringA(s);
}

bool Platform::IsKeyDown(int key) {
	return (::GetKeyState(key) & 0x80000000) != 0;
}

long Platform::SendScintilla(WindowID w, unsigned int msg, unsigned long wParam, long lParam) {
	return ::SendMessage(reinterpret_cast<HWND>(w), msg, wParam, lParam);
}

long Platform::SendScintillaPointer(WindowID w, unsigned int msg, unsigned long wParam, void *lParam) {
	return ::SendMessage(reinterpret_cast<HWND>(w), msg, wParam,
		reinterpret_cast<LPARAM>(lParam));
}

bool Platform::IsDBCSLeadByte(int codePage, char ch) {
	return ::IsDBCSLeadByteEx(codePage, ch) != 0;
}

int Platform::DBCSCharLength(int codePage, const char *s) {
	return (::IsDBCSLeadByteEx(codePage, s[0]) != 0) ? 2 : 1;
}

int Platform::DBCSCharMaxLength() {
	return 2;
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
	vsprintf(buffer,format,pArguments);
	va_end(pArguments);
	Platform::DebugDisplay(buffer);
}
#else
void Platform::DebugPrintf(const char *, ...) {
}
#endif

static bool assertionPopUps = true;

bool Platform::ShowAssertionPopUps(bool assertionPopUps_) {
	bool ret = assertionPopUps;
	assertionPopUps = assertionPopUps_;
	return ret;
}

void Platform::Assert(const char *c, const char *file, int line) {
	char buffer[2000];
	sprintf(buffer, "Assertion [%s] failed at %s %d", c, file, line);
	if (assertionPopUps) {
		int idButton = ::MessageBoxA(0, buffer, "Assertion failure",
			MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);
		if (idButton == IDRETRY) {
			::DebugBreak();
		} else if (idButton == IDIGNORE) {
			// all OK
		} else {
			abort();
		}
	} else {
		strcat(buffer, "\r\n");
		Platform::DebugDisplay(buffer);
		::DebugBreak();
		abort();
	}
}

int Platform::Clamp(int val, int minVal, int maxVal) {
	if (val > maxVal)
		val = maxVal;
	if (val < minVal)
		val = minVal;
	return val;
}

void Platform_Initialise(void *hInstance) {
	OSVERSIONINFO osv = {sizeof(OSVERSIONINFO),0,0,0,0,TEXT("")};
	::GetVersionEx(&osv);
	onNT = osv.dwPlatformId == VER_PLATFORM_WIN32_NT;
	::InitializeCriticalSection(&crPlatformLock);
	hinstPlatformRes = reinterpret_cast<HINSTANCE>(hInstance);
	// This may be called from DllMain, in which case the call to LoadLibrary
	// is bad because it can upset the DLL load order.
	if (!hDLLImage) {
		hDLLImage = ::LoadLibrary(TEXT("Msimg32"));
	}
	if (hDLLImage) {
		AlphaBlendFn = (AlphaBlendSig)::GetProcAddress(hDLLImage, "AlphaBlend");
	}
	ListBoxX_Register();
}

void Platform_Finalise() {
	if (reverseArrowCursor != NULL)
		::DestroyCursor(reverseArrowCursor);
	ListBoxX_Unregister();
	::DeleteCriticalSection(&crPlatformLock);
}
