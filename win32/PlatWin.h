// Scintilla source code edit control
/** @file PlatWin.h
 ** Implementation of platform facilities on Windows.
 **/
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

extern bool IsNT();
extern void Platform_Initialise(void *hInstance);
extern void Platform_Finalise();
extern bool LoadD2D();
extern ID2D1Factory *pD2DFactory;
extern IDWriteFactory *pIDWriteFactory;
