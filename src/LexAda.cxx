#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "Accessor.h"
#include "StyleContext.h"
#include "PropSet.h"
#include "KeyWords.h"
#include "SciLexer.h"
#include "SString.h"

/*
 * Interface
 */

static void ColouriseDocument(
	unsigned int startPos,
	int length,
	int initStyle,
	WordList *keywordlists[],
	Accessor &styler);

LexerModule lmAda(SCLEX_ADA, ColouriseDocument, "ada");

/*
 * Implementation
 */

static void ColouriseCharacter  (StyleContext& sc);
static void ColouriseContext    (StyleContext& sc, char chEnd, int stateEOL);
static void ColouriseComment    (StyleContext& sc);
static void ColouriseDelimiter  (StyleContext& sc);
static void ColouriseLabel      (StyleContext& sc, WordList& keywords);
static void ColouriseNumber     (StyleContext& sc);
static void ColouriseString     (StyleContext& sc);
static void ColouriseWhiteSpace (StyleContext& sc);
static void ColouriseWord       (StyleContext& sc, WordList& keywords);

static inline bool isDelimiterCharacter (int ch);
static inline bool isNumberStartCharacter (int ch);
static inline bool isNumberCharacter (int ch);
static inline bool isSeparatorOrDelimiter (int ch);
static        bool isValidIdentifier (const SString& identifier);
static        bool isValidNumber (const SString& number);
static inline bool isWordStartCharacter (int ch);
static inline bool isWordCharacter (int ch);

static void ColouriseCharacter (StyleContext& sc)
{
	sc.SetState (SCE_ADA_CHARACTER);
	
	// Skip the apostrophe and one more character (so that '' is shown as non-terminated and '''
	// is handled correctly)
	sc.Forward ();
	sc.Forward ();

	ColouriseContext (sc, '\'', SCE_ADA_CHARACTEREOL);
}

static void ColouriseContext (StyleContext& sc, char chEnd, int stateEOL)
{
	while (!sc.atLineEnd && !sc.Match (chEnd))
	{
		sc.Forward ();
	}

	if (!sc.atLineEnd)
	{
		sc.ForwardSetState (SCE_ADA_DEFAULT);
	}
	else
	{
		sc.ChangeState (stateEOL);
	}
}

static void ColouriseComment (StyleContext& sc)
{
	sc.SetState (SCE_ADA_COMMENTLINE);

	while (!sc.atLineEnd)
	{
		sc.Forward ();
	}
}

static void ColouriseDelimiter (StyleContext& sc)
{
	sc.SetState (SCE_ADA_DELIMITER);
	sc.ForwardSetState (SCE_ADA_DEFAULT);
}

static void ColouriseLabel (StyleContext& sc, WordList& keywords)
{
	sc.SetState (SCE_ADA_LABEL);

	// Skip "<<"
	sc.Forward ();
	sc.Forward ();

	SString identifier;

	while (!sc.atLineEnd && !isSeparatorOrDelimiter (sc.ch))
	{
		identifier += (char) tolower (sc.ch);
		sc.Forward ();
	}

	// Skip ">>"
	if (sc.Match ('>', '>'))
	{
		sc.Forward ();
		sc.Forward ();
	}
	else
	{
		sc.ChangeState (SCE_ADA_BADLABEL);
	}

	if (!isValidIdentifier (identifier) || keywords.InList (identifier.c_str ()))
	{
		sc.ChangeState (SCE_ADA_BADLABEL);
	}

	sc.SetState (SCE_ADA_DEFAULT);
}

static void ColouriseNumber (StyleContext& sc)
{
	SString number;
	sc.SetState (SCE_ADA_NUMBER);
	
	while (!isSeparatorOrDelimiter (sc.ch))
	{
		number += (char) sc.ch;
		sc.Forward ();
	}

	// Special case: exponent with sign
	if (tolower (sc.chPrev) == 'e'
		&& (sc.Match ('+') || sc.Match ('-'))
		&& isdigit (sc.chNext))
	{
		number += (char) sc.ch;
		sc.Forward ();
		
		while (!isSeparatorOrDelimiter (sc.ch))
		{
			number += (char) sc.ch;
			sc.Forward ();
		}
	}
	
	if (!isValidNumber (number))
	{
		sc.ChangeState (SCE_ADA_BADNUMBER);
	}
	
	sc.SetState (SCE_ADA_DEFAULT);
}

static void ColouriseString (StyleContext& sc)
{
	sc.SetState (SCE_ADA_STRING);
	sc.Forward ();

	ColouriseContext (sc, '"', SCE_ADA_STRINGEOL);
}

static void ColouriseWhiteSpace (StyleContext& sc)
{
	sc.SetState (SCE_ADA_DEFAULT);
	sc.ForwardSetState (SCE_ADA_DEFAULT);
}

static void ColouriseWord (StyleContext& sc, WordList& keywords)
{
	SString identifier;
	sc.SetState (SCE_ADA_IDENTIFIER);

	while (!sc.atLineEnd && isWordCharacter (sc.ch))
	{
		identifier += (char) tolower (sc.ch);
		sc.Forward ();
	}

	if (!isValidIdentifier (identifier))
	{
		sc.ChangeState (SCE_ADA_BADIDENTIFIER);
	}
	else if (keywords.InList (identifier.c_str ()))
	{
		sc.ChangeState (SCE_ADA_WORD);
	}
	
	sc.SetState (SCE_ADA_DEFAULT);
}

//
// ColouriseDocument
//

static void ColouriseDocument(
	unsigned int startPos,
	int length,
	int initStyle,
	WordList *keywordlists[],
	Accessor &styler)
{
	WordList &keywords = *keywordlists[0];

	StyleContext sc(startPos, length, initStyle, styler);

	// Apostrophe can start either an attribute or a character constant. Which 
	int lineCurrent = styler.GetLine (startPos);
	int lineState   = styler.GetLineState (lineCurrent);
	
	bool apostropheStartsAttribute = (lineState & 1) != 0;

	while (sc.More ())
	{
		if (sc.atLineEnd)
		{
			sc.Forward ();
			lineCurrent++;

			// Remember the apostrophe setting
			styler.SetLineState (lineCurrent, apostropheStartsAttribute ? 1 : 0);

			// Don't leak any styles onto the next line
			sc.SetState (SCE_ADA_DEFAULT);
		}

		// Comments
		if (sc.Match ('-', '-'))
		{
			ColouriseComment (sc);
		}
		
		// Strings
		else if (sc.Match ('"'))
		{
			apostropheStartsAttribute = false;
			ColouriseString (sc);
		}

		// Characters
		else if (sc.Match ('\'') && !apostropheStartsAttribute)
		{
			apostropheStartsAttribute = false;
			ColouriseCharacter (sc);
		}

		// Labels
		else if (sc.Match ('<', '<'))
		{
			apostropheStartsAttribute = false;
			ColouriseLabel (sc, keywords);
		}

		// Whitespace
		else if (isspace (sc.ch))
		{
			ColouriseWhiteSpace (sc);
		}

		// Delimiters
		else if (isDelimiterCharacter (sc.ch))
		{
			apostropheStartsAttribute = sc.ch == ')';
			ColouriseDelimiter (sc);
		}

		// Numbers
		else if (isdigit (sc.ch))
		{
			apostropheStartsAttribute = false;
			ColouriseNumber (sc);
		}

		// Keywords or identifiers
		else
		{
			apostropheStartsAttribute = true;
			ColouriseWord (sc, keywords);
		}
	}

	sc.Complete ();
}

static inline bool isDelimiterCharacter (int ch)
{
	switch (ch)
	{
	case '&':
	case '\'':
	case '(':
	case ')':
	case '*':
	case '+':
	case ',':
	case '-':
	case '.':
	case '/':
	case ':':
	case ';':
	case '<':
	case '=':
	case '>':
	case '|':
		return true;
	default:
		return false;
	}
}

static inline bool isNumberCharacter (int ch)
{
	return isNumberStartCharacter (ch)
		|| ch == '_'
		|| ch == '.'
		|| ch == '#'
		|| (ch >= 'a' && ch <= 'f')
		|| (ch >= 'A' && ch <= 'F');
}

static inline bool isNumberStartCharacter (int ch)
{
	return isdigit (ch) != 0;
}

static inline bool isSeparatorOrDelimiter (int ch)
{
	return isspace (ch) || isDelimiterCharacter (ch);
}

static bool isValidIdentifier (const SString& identifier)
{
	// First character can't be '_', so we set the variable to true initially
	bool lastWasUnderscore = true;

	int length = identifier.length ();
	
	// Zero-length identifiers are not valid (these can occur inside labels)
	if (length == 0) return false;

	if (!isWordStartCharacter (identifier[0])) return false;

	// Check for only valid characters and no double underscores
	for (int i = 0; i < length; i++)
	{
		if (!isWordCharacter (identifier[i])) return false;
		else if (identifier[i] == '_' && lastWasUnderscore) return false;
		lastWasUnderscore = identifier[i] == '_';
	}

	// Check for underscore at the end
	if (lastWasUnderscore == true) return false;

	// All checks passed
	return true;
}

static bool isValidNumber (const SString& number)
{
	int hashPos = number.search ("#");
	bool seenDot = false;
	
	int i = 0;
	int length = number.length ();

	if (length == 0) return false; // Just in case

	// Decimal number
	if (hashPos == -1)
	{
		bool canBeSpecial = false;

		for (; i < length; i++)
		{
			if (number[i] == '_')
			{
				if (!canBeSpecial) return false;
				canBeSpecial = false;
			}
			else if (number[i] == '.')
			{
				if (!canBeSpecial || seenDot) return false;
				canBeSpecial = false;
				seenDot = true;
			}
			else if (isdigit (number[i]))
			{
				canBeSpecial = true;
			}
			else if (number[i] != 'e' && number[i] != 'E')
			{
				return false;
			}
		}
		
		if (!canBeSpecial) return false;
	}
	
	// Based number
	else
	{
		bool canBeSpecial = false;
		int base = 0;

		// Parse base
		for (; i < length; i++)
		{
			int ch = number[i];
			if (ch == '_')
			{
				if (!canBeSpecial) return false;
				canBeSpecial = false;
			}
			else if (isdigit (ch))
			{
				base = base * 10 + (ch - '0');
				if (base > 16) return false;
				canBeSpecial = true;
			}
			else if (ch == '#' && canBeSpecial)
			{
				break;
			}
			else
			{
				return false;
			}
		}

		if (base < 2) return false;
		if (i == length) return false;
		
		i++; // Skip over '#'

		// Parse number
		canBeSpecial = false;

		for (; i < length; i++)
		{
			int ch = tolower (number[i]);
			if (ch == '_')
			{
				if (!canBeSpecial) return false;
				canBeSpecial = false;
			}
			else if (ch == '.')
			{
				if (!canBeSpecial || seenDot) return false;
				canBeSpecial = false;
				seenDot = true;
			}
			else if (isdigit (ch))
			{
				if (ch - '0' >= base) return false;
				canBeSpecial = true;
			}
			else if (ch >= 'a' && ch <= 'f')
			{
				if (ch - 'a' + 10 >= base) return false;
				canBeSpecial = true;
			}
			else if (ch == '#' && canBeSpecial)
			{
				break;
			}
			else
			{
				return false;
			}
		}
		
		if (i == length) return false;
		i++;
	}

	// Exponent (optional)
	if (i < length)
	{
		if (number[i] != 'e' && number[i] != 'E') return false;

		i++;
		
		if (i == length) return false;
		
		if (number[i] == '+')
			i++;
		else if (number[i] == '-')
		{
			if (seenDot)
				i++;
			else
				return false; // Integer literals should not have negative exponents
		}
		
		if (i == length) return false;

		bool canBeSpecial = false;

		for (; i < length; i++)
		{
			if (number[i] == '_')
			{
				if (!canBeSpecial) return false;
				canBeSpecial = false;
			}
			else if (isdigit (number[i]))
			{
				canBeSpecial = true;
			}
			else
			{
				return false;
			}
		}
		
		if (!canBeSpecial) return false;
	}

	return i == length; // if i == length, number was parsed successfully.
}

static inline bool isWordCharacter (int ch)
{
	return isWordStartCharacter (ch) || isdigit (ch);
}

static inline bool isWordStartCharacter (int ch)
{
	return isalpha (ch) || ch == '_';
}
