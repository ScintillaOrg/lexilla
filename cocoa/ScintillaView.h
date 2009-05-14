
/**
 * Declaration of the native Cocoa View that serves as container for the scintilla parts.
 *
 * Created by Mike Lischke.
 *
 * Copyright 2009 Sun Microsystems, Inc. All rights reserved.
 * This file is dual licensed under LGPL v2.1 and the Scintilla license (http://www.scintilla.org/License.txt).
 */

#import <Cocoa/Cocoa.h>

#import "ScintillaCocoa.h"

@class ScintillaView;

/**
 * InnerView is the Cocoa interface to the Scintilla backend. It handles text input and
 * provides a canvas for painting the output.
 */
@interface InnerView : NSView <NSTextInput>
{
@private
  Scintilla::ScintillaView* mOwner;
  NSCursor* mCurrentCursor;
  NSTrackingRectTag mCurrentTrackingRect;

  // Set when we are in composition mode and partial input is displayed.
  NSRange mMarkedTextRange;
  
  // Caret position when a drag operation started.
  int mLastPosition;
}

- (void) removeMarkedText;
- (void) setCursor: (Scintilla::Window::Cursor) cursor;

@property (retain) ScintillaView* owner;
@end

@interface ScintillaView : NSView
{
@private
  // The back end is kind of a controller and model in one.
  // It uses the content view for display.
  Scintilla::ScintillaCocoa* mBackend;
  
  // This is the actual content to which the backend renders itself.
  InnerView* mContent;
  
  NSScroller* mHorizontalScroller;
  NSScroller* mVerticalScroller;
}

- (void) dealloc;
- (void) layout;

- (void) sendNotification: (NSString*) notificationName;

// Scroller handling
- (BOOL) setVerticalScrollRange: (int) range page: (int) page;
- (void) setVerticalScrollPosition: (float) position;
- (BOOL) setHorizontalScrollRange: (int) range page: (int) page;
- (void) setHorizontalScrollPosition: (float) position;

- (void) scrollerAction: (id) sender;
- (InnerView*) content;

// NSTextView compatibility layer.
- (NSString*) string;
- (void) setString: (NSString*) aString;
- (void) setEditable: (BOOL) editable;

// Back end properties getters and setters.
- (void) setGeneralProperty: (int) property parameter: (long) parameter value: (long) value;
- (long) getGeneralProperty: (int) property parameter: (long) parameter;
- (void) setColorProperty: (int) property parameter: (long) parameter value: (NSColor*) value;
- (void) setColorProperty: (int) property parameter: (long) parameter fromHTML: (NSString*) fromHTML;
- (NSColor*) getColorProperty: (int) property parameter: (long) parameter;
- (void) setReferenceProperty: (int) property parameter: (long) parameter value: (const void*) value;
- (const void*) getReferenceProperty: (int) property parameter: (long) parameter;
- (void) setStringProperty: (int) property parameter: (long) parameter value: (NSString*) value;
- (NSString*) getStringProperty: (int) property parameter: (long) parameter;
- (void) setLexerProperty: (NSString*) name value: (NSString*) value;
- (NSString*) getLexerProperty: (NSString*) name;

@property Scintilla::ScintillaCocoa* backend;

@end
