// SciTE - Scintilla based Text Editor
// Accessor.h - rapid easy access to contents of a Scintilla
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

enum { wsSpace = 1, wsTab = 2, wsSpaceTab = 4, wsInconsistent=8};

class Accessor;

typedef bool (*PFNIsCommentLeader)(Accessor &styler, int pos, int len);

// Interface to data in a Scintilla
class Accessor {
public:
	virtual void SetCodePage(int codePage_)=0;
	virtual char operator[](int position)=0;
	virtual char SafeGetCharAt(int position, char chDefault=' ')=0;
	virtual bool IsLeadByte(char ch)=0;
	virtual char StyleAt(int position)=0;
	virtual int GetLine(int position)=0;
	virtual int LineStart(int line)=0;
	virtual int LevelAt(int line)=0;
	virtual int Length()=0;
	virtual void Flush()=0;
	virtual int GetLineState(int line)=0;
	virtual int SetLineState(int line, int state)=0;
	virtual PropSet &GetPropSet()=0;

	// Style setting
	virtual void StartAt(unsigned int start, char chMask=31)=0;
	virtual void SetFlags(char chFlags_, char chWhile_)=0;
	virtual unsigned int GetStartSegment()=0;
	virtual void StartSegment(unsigned int pos)=0;
	virtual void ColourTo(unsigned int pos, int chAttr)=0;
	virtual void SetLevel(int line, int level)=0;
	virtual int IndentAmount(int line, int *flags, PFNIsCommentLeader pfnIsCommentLeader = 0)=0;
};

