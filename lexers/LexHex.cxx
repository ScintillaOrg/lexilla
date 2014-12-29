// Scintilla source code edit control
/** @file LexHex.cxx
 ** Lexer for Motorola S-Record.
 **
 ** Written by Markus Heidelberg
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

/*
 *  Motorola S-Record file format
 * ===============================
 *
 * Each record (line) is built as follows:
 *
 *    field       size     states
 *
 *  +----------+
 *  | start    |  1 ('S')  SCE_SREC_RECSTART
 *  +----------+
 *  | type     |  1        SCE_SREC_RECTYPE
 *  +----------+
 *  | count    |  2        SCE_SREC_BYTECOUNT, SCE_SREC_BYTECOUNT_WRONG
 *  +----------+
 *  | address  |  4/6/8    SCE_SREC_NOADDRESS, SCE_SREC_DATAADDRESS, SCE_SREC_RECCOUNT, SCE_SREC_STARTADDRESS
 *  +----------+
 *  | data     |  0..500   SCE_SREC_DATA_ODD, SCE_SREC_DATA_EVEN
 *  +----------+
 *  | checksum |  2        SCE_SREC_CHECKSUM, SCE_SREC_CHECKSUM_WRONG
 *  +----------+
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

// prototypes for general helper functions
static inline bool IsNewline(const int ch);
static int GetHexaChar(char hd1, char hd2);
static int GetHexaChar(unsigned int pos, Accessor &styler);
static bool ForwardWithinLine(StyleContext &sc, int nb = 1);

// prototypes for file format specific helper functions
static unsigned int GetSrecRecStartPosition(unsigned int pos, Accessor &styler);
static int GetSrecByteCount(unsigned int recStartPos, Accessor &styler);
static int CountSrecByteCount(unsigned int recStartPos, Accessor &styler);
static int GetSrecAddressFieldSize(unsigned int recStartPos, Accessor &styler);
static int GetSrecAddressFieldType(unsigned int recStartPos, Accessor &styler);
static int GetSrecChecksum(unsigned int recStartPos, Accessor &styler);
static int CalcSrecChecksum(unsigned int recStartPos, Accessor &styler);

static inline bool IsNewline(const int ch)
{
    return (ch == '\n' || ch == '\r');
}

static int GetHexaChar(char hd1, char hd2)
{
	int hexValue = 0;

	if (hd1 >= '0' && hd1 <= '9') {
		hexValue += 16 * (hd1 - '0');
	} else if (hd1 >= 'A' && hd1 <= 'F') {
		hexValue += 16 * (hd1 - 'A' + 10);
	} else if (hd1 >= 'a' && hd1 <= 'f') {
		hexValue += 16 * (hd1 - 'a' + 10);
	} else {
		return -1;
	}

	if (hd2 >= '0' && hd2 <= '9') {
		hexValue += hd2 - '0';
	} else if (hd2 >= 'A' && hd2 <= 'F') {
		hexValue += hd2 - 'A' + 10;
	} else if (hd2 >= 'a' && hd2 <= 'f') {
		hexValue += hd2 - 'a' + 10;
	} else {
		return -1;
	}

	return hexValue;
}

static int GetHexaChar(unsigned int pos, Accessor &styler)
{
	char highNibble, lowNibble;

	highNibble = styler.SafeGetCharAt(pos);
	lowNibble = styler.SafeGetCharAt(pos + 1);

	return GetHexaChar(highNibble, lowNibble);
}

// Forward <nb> characters, but abort (and return false) if hitting the line
// end. Return true if forwarding within the line was possible.
// Avoids influence on highlighting of the subsequent line if the current line
// is malformed (too short).
static bool ForwardWithinLine(StyleContext &sc, int nb)
{
	for (int i = 0; i < nb; i++) {
		if (sc.atLineEnd) {
			// line is too short
			sc.SetState(SCE_SREC_DEFAULT);
			sc.Forward();
			return false;
		} else {
			sc.Forward();
		}
	}

	return true;
}

// Get the position of the record "start" field (first character in line) in
// the record around position <pos>.
static unsigned int GetSrecRecStartPosition(unsigned int pos, Accessor &styler)
{
	while (styler.SafeGetCharAt(pos) != 'S') {
		pos--;
	}

	return pos;
}

// Get the value of the "byte count" field, it counts the number of bytes in
// the subsequent fields ("address", "data" and "checksum" fields).
static int GetSrecByteCount(unsigned int recStartPos, Accessor &styler)
{
	int val;

	val = GetHexaChar(recStartPos + 2, styler);
	if (val < 0) {
	       val = 0;
	}

	return val;
}

// Count the number of digit pairs for the "address", "data" and "checksum"
// fields in this record. Has to be equal to the "byte count" field value.
// If the record is too short, a negative count may be returned.
static int CountSrecByteCount(unsigned int recStartPos, Accessor &styler)
{
	int cnt;
	unsigned int pos;

	pos = recStartPos;

	while (!IsNewline(styler.SafeGetCharAt(pos, '\n'))) {
		pos++;
	}

	// number of digits in this line minus number of digits of uncounted fields
	cnt = static_cast<int>(pos - recStartPos) - 4;

	// Prepare round up if odd (digit pair incomplete), this way the byte
	// count is considered to be valid if the checksum is incomplete.
	if (cnt >= 0) {
		cnt++;
	}

	// digit pairs
	cnt /= 2;

	return cnt;
}

// Get the size of the "address" field.
static int GetSrecAddressFieldSize(unsigned int recStartPos, Accessor &styler)
{
	switch (styler.SafeGetCharAt(recStartPos + 1)) {
		case '0':
		case '1':
		case '5':
		case '9':
			return 2; // 16 bit

		case '2':
		case '6':
		case '8':
			return 3; // 24 bit

		case '3':
		case '7':
			return 4; // 32 bit

		default:
			return 0;
	}
}

// Get the type of the "address" field content.
static int GetSrecAddressFieldType(unsigned int recStartPos, Accessor &styler)
{
	switch (styler.SafeGetCharAt(recStartPos + 1)) {
		case '0':
			return SCE_SREC_NOADDRESS;

		case '1':
		case '2':
		case '3':
			return SCE_SREC_DATAADDRESS;

		case '5':
		case '6':
			return SCE_SREC_RECCOUNT;

		case '7':
		case '8':
		case '9':
			return SCE_SREC_STARTADDRESS;

		default:
			return SCE_SREC_DEFAULT;
	}
}

// Get the value of the "checksum" field.
static int GetSrecChecksum(unsigned int recStartPos, Accessor &styler)
{
	int byteCount;

	byteCount = GetSrecByteCount(recStartPos, styler);

	return GetHexaChar(recStartPos + 2 + byteCount * 2, styler);
}

// Calculate the checksum of the record.
static int CalcSrecChecksum(unsigned int recStartPos, Accessor &styler)
{
	int byteCount;
	unsigned int cs = 0;

	byteCount = GetSrecByteCount(recStartPos, styler);

	// sum over "byte count", "address" and "data" fields
	for (unsigned int pos = recStartPos + 2; pos < recStartPos + 2 + byteCount * 2; pos += 2) {
		int val = GetHexaChar(pos, styler);

		if (val < 0) {
			return val;
		}

		cs += val;
	}

	// low byte of one's complement
	return (~cs) & 0xFF;
}

static void ColouriseSrecDoc(unsigned int startPos, int length, int initStyle, WordList *[], Accessor &styler)
{
	StyleContext sc(startPos, length, initStyle, styler);

	while (sc.More()) {
		unsigned int recStartPos;
		int byteCount, addrFieldSize, addrFieldType, dataFieldSize;
		int cs1, cs2;

		switch (sc.state) {
			case SCE_SREC_DEFAULT:
				if (sc.atLineStart && sc.Match('S')) {
					sc.SetState(SCE_SREC_RECSTART);
				}
				ForwardWithinLine(sc);
				break;

			case SCE_SREC_RECSTART:
				sc.SetState(SCE_SREC_RECTYPE);
				ForwardWithinLine(sc);
				break;

			case SCE_SREC_RECTYPE:
				recStartPos = sc.currentPos - 2;
				byteCount = GetSrecByteCount(recStartPos, styler);

				if (byteCount == CountSrecByteCount(recStartPos, styler)) {
					sc.SetState(SCE_SREC_BYTECOUNT);
				} else {
					sc.SetState(SCE_SREC_BYTECOUNT_WRONG);
				}

				ForwardWithinLine(sc, 2);
				break;

			case SCE_SREC_BYTECOUNT:
			case SCE_SREC_BYTECOUNT_WRONG:
				recStartPos = sc.currentPos - 4;
				addrFieldSize = GetSrecAddressFieldSize(recStartPos, styler);
				addrFieldType = GetSrecAddressFieldType(recStartPos, styler);

				if (addrFieldSize > 0) {
					sc.SetState(addrFieldType);
					ForwardWithinLine(sc, addrFieldSize * 2);
				} else {
					// malformed
					sc.SetState(SCE_SREC_DEFAULT);
					ForwardWithinLine(sc);
				}
				break;

			case SCE_SREC_NOADDRESS:
			case SCE_SREC_DATAADDRESS:
			case SCE_SREC_RECCOUNT:
			case SCE_SREC_STARTADDRESS:
				recStartPos = GetSrecRecStartPosition(sc.currentPos, styler);
				byteCount = GetSrecByteCount(recStartPos, styler);
				addrFieldSize = GetSrecAddressFieldSize(recStartPos, styler);
				dataFieldSize = byteCount - addrFieldSize - 1; // -1 for checksum field

				sc.SetState(SCE_SREC_DATA_ODD);

				for (int i = 0; i < dataFieldSize * 2; i++) {
					if ((i & 0x3) == 0) {
						sc.SetState(SCE_SREC_DATA_ODD);
					} else if ((i & 0x3) == 2) {
						sc.SetState(SCE_SREC_DATA_EVEN);
					}

					if (!ForwardWithinLine(sc)) {
						break;
					}
				}
				break;

			case SCE_SREC_DATA_ODD:
			case SCE_SREC_DATA_EVEN:
				recStartPos = GetSrecRecStartPosition(sc.currentPos, styler);
				cs1 = CalcSrecChecksum(recStartPos, styler);
				cs2 = GetSrecChecksum(recStartPos, styler);

				if (cs1 != cs2 || cs1 < 0 || cs2 < 0) {
					sc.SetState(SCE_SREC_CHECKSUM_WRONG);
				} else {
					sc.SetState(SCE_SREC_CHECKSUM);
				}

				ForwardWithinLine(sc, 2);
				break;

			case SCE_SREC_CHECKSUM:
			case SCE_SREC_CHECKSUM_WRONG:
				// record finished
				sc.SetState(SCE_SREC_DEFAULT);
				ForwardWithinLine(sc);
				break;
		}
	}
	sc.Complete();
}

LexerModule lmSrec(SCLEX_SREC, ColouriseSrecDoc, "srec", 0, NULL);
