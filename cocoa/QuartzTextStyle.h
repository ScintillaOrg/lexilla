/*
 *  QuartzTextStyle.h
 *
 *  Created by Evan Jones on Wed Oct 02 2002.
 *
 */

#ifndef _QUARTZ_TEXT_STYLE_H
#define _QUARTZ_TEXT_STYLE_H

#include "QuartzTextStyleAttribute.h"

class QuartzTextStyle
{
public:
    QuartzTextStyle()
    {
		fontRef = NULL;
		styleDict = CFDictionaryCreateMutable(NULL, 1, NULL, NULL);
		characterSet = 0;
    }

    ~QuartzTextStyle()
    {
		if (styleDict != NULL)
		{
			CFRelease(styleDict);
			styleDict = NULL;
		}
    }
	
	CFMutableDictionaryRef getCTStyle() const
	{
		return styleDict;
	}
	 
	void setCTStyleColor(CGColor* inColor )
	{
		CFDictionarySetValue(styleDict, kCTForegroundColorAttributeName, inColor);
	}
	
	float getAscent() const
	{
		return ::CTFontGetAscent(fontRef);
	}
	
	float getDescent() const
	{
		return ::CTFontGetDescent(fontRef);
	}
	
	float getLeading() const
	{
		return ::CTFontGetLeading(fontRef);
	}
	
	void setFontRef(CTFontRef inRef, int characterSet_)
	{
		fontRef = inRef;
		characterSet = characterSet_;
		
		if (styleDict != NULL)
			CFRelease(styleDict);

		styleDict = CFDictionaryCreateMutable(NULL, 1, NULL, NULL);
		
		CFDictionaryAddValue(styleDict, kCTFontAttributeName, fontRef);
	}
	
	CTFontRef getFontRef()
	{
		return fontRef;
	}
	
	int getCharacterSet()
	{
		return characterSet;
	}
	
private:
	CFMutableDictionaryRef styleDict;
	CTFontRef fontRef;
	int characterSet;
};

#endif

