// WindowAccessor.h - implementation of BufferAccess and StylingAccess on a Scintilla rapid easy access to contents of a Scintilla
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

class WindowAccessor : public Accessor {
protected:
	// bufferSize is a trade off between time taken to copy the characters and SendMessage overhead
	// slopSize positions the buffer before the desired position in case there is some backtracking
	enum {bufferSize=4000, slopSize=bufferSize/8};
	char buf[bufferSize+1];
	WindowID id;
	PropSet &props;
	int startPos;
	int endPos;
	int lenDoc;
	int codePage;	

	char styleBuf[bufferSize];
	int validLen;
	char chFlags;
	char chWhile;
	unsigned int startSeg;

	bool InternalIsLeadByte(char ch);
	void Fill(int position);
public:
	WindowAccessor(WindowID id_, PropSet &props_) : 
			id(id_), props(props_), startPos(0x7FFFFFFF), endPos(0), 
			lenDoc(-1), codePage(0), validLen(0), chFlags(0) {
	}
	void SetCodePage(int codePage_) { codePage = codePage_; }
	char operator[](int position) {
		if (position < startPos || position >= endPos) {
			Fill(position);
		}
		return buf[position - startPos];
	}
	char SafeGetCharAt(int position, char chDefault=' ') {
		// Safe version of operator[], returning a defined value for invalid position 
		if (position < startPos || position >= endPos) {
			Fill(position);
			if (position < startPos || position >= endPos) {
				// Position is outside range of document 
				return chDefault;
			}
		}
		return buf[position - startPos];
	}
	bool IsLeadByte(char ch) {
		return codePage && InternalIsLeadByte(ch);
	}
	char StyleAt(int position);
	int GetLine(int position);
	int LineStart(int line);
	int LevelAt(int line);
	int Length();
	void Flush();
	int GetLineState(int line);
	int SetLineState(int line, int state);
	PropSet &GetPropSet() { return props; }

	void StartAt(unsigned int start, char chMask=31);
	void SetFlags(char chFlags_, char chWhile_) {chFlags = chFlags_; chWhile = chWhile_; };
	unsigned int GetStartSegment() { return startSeg; }
	void StartSegment(unsigned int pos);
	void ColourTo(unsigned int pos, int chAttr);
	void SetLevel(int line, int level);
	int IndentAmount(int line, int *flags, PFNIsCommentLeader pfnIsCommentLeader = 0);
};
