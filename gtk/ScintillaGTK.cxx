// Scintilla source code edit control
// ScintillaGTK.cxx - GTK+ specific subclass of ScintillaBase
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include "Platform.h"

#include "ScintillaWidget.h"
#include "Scintilla.h"
#ifdef SCI_LEXER
#include "SciLexer.h"
#include "PropSet.h"
#include "Accessor.h"
#include "KeyWords.h"
#endif
#include "ContractionState.h"
#include "SVector.h"
#include "CellBuffer.h"
#include "CallTip.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "LineMarker.h"
#include "Style.h"
#include "AutoComplete.h"
#include "ViewStyle.h"
#include "Document.h"
#include "Editor.h"
#include "ScintillaBase.h"

#include "gtk/gtksignal.h"

#define USE_XIM 1

class ScintillaGTK : public ScintillaBase {
	_ScintillaObject *sci;
	Window scrollbarv;
	Window scrollbarh;
	GtkObject *adjustmentv;
	GtkObject *adjustmenth;
	char *pasteBuffer;
	bool pasteBufferIsRectangular;
	GdkEventButton evbtn;
	bool capturedMouse;
	bool dragWasDropped;
	char *primarySelectionCopy;

	static GdkAtom clipboard_atom;

	// Input context used for supporting internationalized key entry
	GdkIC     *ic;
	GdkICAttr *ic_attr;

public:
	ScintillaGTK(_ScintillaObject *sci_);
	virtual ~ScintillaGTK();

private:
	virtual void Initialise();
	virtual void Finalise();
	virtual void StartDrag();
public:	// Public for scintilla_send_message
	virtual long WndProc(unsigned int iMessage, unsigned long wParam, long lParam);
private:
	virtual long DefWndProc(unsigned int iMessage, unsigned long wParam, long lParam);
	virtual void SetTicking(bool on);
	virtual void SetMouseCapture(bool on);
	virtual bool HaveMouseCapture();
	void FullPaint();
	void SyncPaint(PRectangle rc);
	virtual void ScrollText(int linesToMove);
	virtual void SetVerticalScrollPos();
	virtual void SetHorizontalScrollPos();
	virtual bool ModifyScrollBars(int nMax, int nPage);
	void ScintillaGTK::ReconfigureScrollBars();
	virtual void NotifyChange();
	virtual void NotifyFocus(bool focus);
	virtual void NotifyParent(SCNotification scn);
	void NotifyKey(int key, int modifiers);
	virtual int KeyDefault(int key, int modifiers);
	virtual void Copy();
	virtual void Paste();
	virtual void CreateCallTipWindow(PRectangle rc);
	virtual void AddToPopUp(const char *label, int cmd=0, bool enabled=true);
	bool OwnPrimarySelection();
	virtual void ClaimSelection();
	void ReceivedSelection(GtkSelectionData *selection_data);
	void ReceivedDrop(GtkSelectionData *selection_data);
	void GetSelection(GtkSelectionData *selection_data, guint info, char *text, bool isRectangular);
	void UnclaimSelection(GdkEventSelection *selection_event);
	void Resize(int width, int height);
	
	// Callback functions
#ifdef USE_XIM
	static gint Realize(GtkWidget *widget, ScintillaGTK *sciThis);
	static gint UnRealize(GtkWidget *widget, ScintillaGTK *sciThis);
	static gint CursorMoved(GtkWidget *widget, int xoffset, int yoffset, ScintillaGTK *sciThis);
#endif
	static gint FocusIn(GtkWidget *widget, GdkEventFocus *event, ScintillaGTK *sciThis);
	static gint FocusOut(GtkWidget *widget, GdkEventFocus *event, ScintillaGTK *sciThis);
	static gint Expose(GtkWidget *widget, GdkEventExpose *ose, ScintillaGTK *sciThis);
	static void ScrollSignal(GtkAdjustment *adj, ScintillaGTK *sciThis);
	static void ScrollHSignal(GtkAdjustment *adj, ScintillaGTK *sciThis);
	static gint MoveResize(GtkWidget *widget, GtkAllocation *allocation, ScintillaGTK *sciThis);
	static gint Press(GtkWidget *widget, GdkEventButton *event, ScintillaGTK *sciThis);
	static gint MouseRelease(GtkWidget *widget, GdkEventButton *event, ScintillaGTK *sciThis);
	static gint Motion(GtkWidget *widget, GdkEventMotion *event, ScintillaGTK *sciThis);
	static gint KeyPress(GtkWidget *widget, GdkEventKey *event, ScintillaGTK *sciThis);
	static gint KeyRelease(GtkWidget *widget, GdkEventKey *event, ScintillaGTK *sciThis);
	static gint DestroyWindow(GtkWidget *widget, ScintillaGTK *sciThis);
	static void SelectionReceived(GtkWidget *widget, GtkSelectionData *selection_data,
		guint time, ScintillaGTK *sciThis);
	static void SelectionGet(GtkWidget *widget, GtkSelectionData *selection_data,
		guint info, guint time, ScintillaGTK *sciThis);
	static void SelectionClearEvent(GtkWidget *widget, GdkEventSelection *selection_event, ScintillaGTK *sciThis);
	static void DragBegin(GtkWidget *widget, GdkDragContext *context, 
		ScintillaGTK *sciThis);
	static gboolean DragMotion(GtkWidget *widget, GdkDragContext *context, 
		gint x, gint y, guint time, ScintillaGTK *sciThis);
	static void DragLeave(GtkWidget *widget, GdkDragContext *context, 
		guint time, ScintillaGTK *sciThis);
	static void DragEnd(GtkWidget *widget, GdkDragContext *context, 
		ScintillaGTK *sciThis);
	static gboolean Drop(GtkWidget *widget, GdkDragContext *context, 
		gint x, gint y, guint time, ScintillaGTK *sciThis);
	static void DragDataReceived(GtkWidget *widget, GdkDragContext *context,
		gint x, gint y, GtkSelectionData *selection_data, guint info, guint time,
		ScintillaGTK *sciThis);
	static void DragDataGet(GtkWidget *widget, GdkDragContext *context,
		GtkSelectionData *selection_data, guint info, guint time, ScintillaGTK *sciThis);
	static gint TimeOut(ScintillaGTK *sciThis);
	static void PopUpCB(ScintillaGTK *sciThis, guint action, GtkWidget *widget);
	static gint ExposeCT(GtkWidget *widget, GdkEventExpose *ose, CallTip *ct);
	static long DirectFunction(ScintillaGTK *sciThis, 
		unsigned int iMessage, unsigned long wParam, long lParam);
};

enum {
    COMMAND_SIGNAL,
    NOTIFY_SIGNAL,
    LAST_SIGNAL
};

static gint scintilla_signals[LAST_SIGNAL] = { 0 };

GdkAtom ScintillaGTK::clipboard_atom = GDK_NONE;

enum {
    TARGET_STRING,
    TARGET_TEXT,
    TARGET_COMPOUND_TEXT
};

ScintillaGTK::ScintillaGTK(_ScintillaObject *sci_) :
	adjustmentv(0), adjustmenth(0), 
	pasteBuffer(0), pasteBufferIsRectangular(false), 
	capturedMouse(false), dragWasDropped(false),
	primarySelectionCopy(0), ic(NULL), ic_attr(NULL) {
	sci = sci_;
	wMain = GTK_WIDGET(sci);
	
	Initialise();
}

ScintillaGTK::~ScintillaGTK() { 
	delete []primarySelectionCopy;
}

#ifdef USE_XIM
gint ScintillaGTK::Realize(GtkWidget *widget, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("ScintillaGTK::realize in %x\n", sciThis);
	if (gdk_im_ready () && (sciThis->ic_attr = gdk_ic_attr_new ()) != NULL) {
		gint width, height;
		GdkColormap *colormap;
		GdkEventMask mask;
		GdkICAttr *attr = sciThis->ic_attr;
		GdkICAttributesType attrmask = GDK_IC_ALL_REQ;
		GdkIMStyle style;
		GdkIMStyle supported_style = (GdkIMStyle) (GDK_IM_PREEDIT_NONE | 
													 GDK_IM_PREEDIT_NOTHING |
													 GDK_IM_PREEDIT_POSITION |
													 GDK_IM_STATUS_NONE |
													 GDK_IM_STATUS_NOTHING);
		  
		if (widget->style && widget->style->font->type != GDK_FONT_FONTSET)
			supported_style = (GdkIMStyle) ((int) supported_style & ~GDK_IM_PREEDIT_POSITION);
		  
		attr->style = style = gdk_im_decide_style (supported_style);
		attr->client_window = widget->window;

		if ((colormap = gtk_widget_get_colormap (widget)) != gtk_widget_get_default_colormap ()) {
			attrmask = (GdkICAttributesType) ((int) attrmask | GDK_IC_PREEDIT_COLORMAP);
			attr->preedit_colormap = colormap;
		}

		switch (style & GDK_IM_PREEDIT_MASK) {
			case GDK_IM_PREEDIT_POSITION:
			if (widget->style && widget->style->font->type != GDK_FONT_FONTSET)	{
				g_warning ("over-the-spot style requires fontset");
				break;
			}

			attrmask = (GdkICAttributesType) ((int) attrmask | GDK_IC_PREEDIT_POSITION_REQ);
			gdk_window_get_size (widget->window, &width, &height);
			attr->spot_location.x = 0;
			attr->spot_location.y = height;
			attr->preedit_area.x = 0;
			attr->preedit_area.y = 0;
			attr->preedit_area.width = width;
			attr->preedit_area.height = height;
			attr->preedit_fontset = widget->style->font;
	  
			break;
		}
		sciThis->ic = gdk_ic_new (attr, attrmask);
		  
		if (sciThis->ic == NULL)
			g_warning ("Can't create input context.");
		else {
			mask = gdk_window_get_events (widget->window);
			mask = (GdkEventMask) ((int) mask | gdk_ic_get_events(sciThis->ic));
			gdk_window_set_events (widget->window, mask);
	  
			if (GTK_WIDGET_HAS_FOCUS (widget))
				gdk_im_begin (sciThis->ic, widget->window);
		}
	}
	return FALSE;
}

gint ScintillaGTK::UnRealize(GtkWidget *, ScintillaGTK *sciThis) {
	if (sciThis->ic) {
		gdk_ic_destroy (sciThis->ic);
		sciThis->ic = NULL;
	}
	if (sciThis->ic_attr) {
		gdk_ic_attr_destroy (sciThis->ic_attr);
		sciThis->ic_attr = NULL;
	}
  return FALSE;
}

gint ScintillaGTK::CursorMoved(GtkWidget *widget, int xoffset, int yoffset, ScintillaGTK *sciThis) {
  if (GTK_WIDGET_HAS_FOCUS(widget) && gdk_im_ready() && sciThis->ic && 
     (gdk_ic_get_style (sciThis->ic) & GDK_IM_PREEDIT_POSITION)) {
	  sciThis->ic_attr->spot_location.x = xoffset;
	  sciThis->ic_attr->spot_location.y = yoffset;
	  gdk_ic_set_attr (sciThis->ic, sciThis->ic_attr, GDK_IC_SPOT_LOCATION);
	}
  return FALSE;
}

#endif

gint ScintillaGTK::FocusIn(GtkWidget *widget, GdkEventFocus * /*event*/, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("ScintillaGTK::focus in %x\n", sciThis);
	GTK_WIDGET_SET_FLAGS(widget, GTK_HAS_FOCUS);
	sciThis->SetFocusState(true);

#ifdef USE_XIM
	if (sciThis->ic)
		gdk_im_begin(sciThis->ic, widget->window);
#endif

	return FALSE;
}

gint ScintillaGTK::FocusOut(GtkWidget *widget, GdkEventFocus * /*event*/, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("ScintillaGTK::focus out %x\n", sciThis);
	GTK_WIDGET_UNSET_FLAGS(widget, GTK_HAS_FOCUS);
	sciThis->SetFocusState(false);
  
#ifdef USE_XIM
	gdk_im_end();
#endif
  
	return FALSE;
}

void ScintillaGTK::Initialise() {
	pasteBuffer = 0;
	pasteBufferIsRectangular = false;

	GTK_WIDGET_SET_FLAGS(wMain.GetID(), GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS(GTK_WIDGET(wMain.GetID()), GTK_SENSITIVE);
	gtk_signal_connect(GTK_OBJECT(wMain.GetID()), "size_allocate",
                   	GTK_SIGNAL_FUNC(MoveResize), this);
	gtk_widget_set_events (wMain.GetID(),
                       	GDK_KEY_PRESS_MASK
                       	| GDK_KEY_RELEASE_MASK
                       	| GDK_FOCUS_CHANGE_MASK);

#ifdef USE_XIM
	gtk_signal_connect(GTK_OBJECT(wMain.GetID()), "realize",
										GtkSignalFunc(Realize), this);
	gtk_signal_connect(GTK_OBJECT(wMain.GetID()), "unrealize",
										GtkSignalFunc(UnRealize), this);
//	gtk_signal_connect(GTK_OBJECT(wMain.GetID()), "cursor_moved",
//										GtkSignalFunc(CursorMoved), this);
#endif

	// Using "after" connect to avoid main window using cursor keys
	// to move focus.
	//gtk_signal_connect(GTK_OBJECT(wMain), "key_press_event",
	//	GtkSignalFunc(key_event), this);
	gtk_signal_connect_after(GTK_OBJECT(wMain.GetID()), "key_press_event",
                         	GtkSignalFunc(KeyPress), this);

	gtk_signal_connect(GTK_OBJECT(wMain.GetID()), "key_release_event",
                   	GtkSignalFunc(KeyRelease), this);
	gtk_signal_connect(GTK_OBJECT(wMain.GetID()), "focus_in_event",
                   	GtkSignalFunc(FocusIn), this);
	gtk_signal_connect(GTK_OBJECT(wMain.GetID()), "focus_out_event",
                   	GtkSignalFunc(FocusOut), this);
	gtk_signal_connect(GTK_OBJECT(wMain.GetID()), "destroy", 
					GtkSignalFunc(DestroyWindow), this);

	wDraw = gtk_drawing_area_new();
	gtk_signal_connect(GTK_OBJECT(wDraw.GetID()), "expose_event",
                   	GtkSignalFunc(Expose), this);
	gtk_signal_connect(GTK_OBJECT(wDraw.GetID()), "motion_notify_event",
                   	GtkSignalFunc(Motion), this);
	gtk_signal_connect(GTK_OBJECT(wDraw.GetID()), "button_press_event",
                   	GtkSignalFunc(Press), this);
	gtk_signal_connect(GTK_OBJECT(wDraw.GetID()), "button_release_event",
                   	GtkSignalFunc(MouseRelease), this);
	gtk_signal_connect(GTK_OBJECT(wDraw.GetID()), "selection_received",
                   	GtkSignalFunc(SelectionReceived), this);
	gtk_signal_connect(GTK_OBJECT(wDraw.GetID()), "selection_get",
                   	GtkSignalFunc(SelectionGet), this);
	gtk_signal_connect(GTK_OBJECT(wDraw.GetID()), "selection_clear_event",
                   	GtkSignalFunc(SelectionClearEvent), this);

	gtk_widget_set_events(wDraw.GetID(),
                       	GDK_EXPOSURE_MASK
                       	| GDK_LEAVE_NOTIFY_MASK
                       	| GDK_BUTTON_PRESS_MASK
                       	| GDK_BUTTON_RELEASE_MASK
                       	| GDK_POINTER_MOTION_MASK
                       	| GDK_POINTER_MOTION_HINT_MASK
                      	);

	gtk_drawing_area_size(GTK_DRAWING_AREA(wDraw.GetID()), 1000, 1000);
	gtk_fixed_put(GTK_FIXED(sci), wDraw.GetID(), 0, 0);

	adjustmentv = gtk_adjustment_new(0.0, 0.0, 201.0, 1.0, 20.0, 20.0);
	scrollbarv = gtk_vscrollbar_new(GTK_ADJUSTMENT(adjustmentv));
	GTK_WIDGET_UNSET_FLAGS(scrollbarv.GetID(), GTK_CAN_FOCUS);
	gtk_signal_connect(GTK_OBJECT(adjustmentv), "value_changed",
                   	GTK_SIGNAL_FUNC(ScrollSignal), this);
	gtk_fixed_put(GTK_FIXED(sci), scrollbarv.GetID(), 0, 0);

	adjustmenth = gtk_adjustment_new(0.0, 0.0, 101.0, 1.0, 20.0, 20.0);
	scrollbarh = gtk_hscrollbar_new(GTK_ADJUSTMENT(adjustmenth));
	GTK_WIDGET_UNSET_FLAGS(scrollbarh.GetID(), GTK_CAN_FOCUS);
	gtk_signal_connect(GTK_OBJECT(adjustmenth), "value_changed",
                   	GTK_SIGNAL_FUNC(ScrollHSignal), this);
	gtk_fixed_put(GTK_FIXED(sci), scrollbarh.GetID(), 0, 0);

	gtk_widget_grab_focus(wMain.GetID());

	static const GtkTargetEntry targets[] = {
                                    		{ "STRING", 0, TARGET_STRING },
                                    		{ "TEXT",   0, TARGET_TEXT },
                                    		{ "COMPOUND_TEXT", 0, TARGET_COMPOUND_TEXT },
                                    	};
	static const gint n_targets = sizeof(targets) / sizeof(targets[0]);

	gtk_selection_add_targets(GTK_WIDGET(wDraw.GetID()), GDK_SELECTION_PRIMARY,
		     targets, n_targets);

	if (!clipboard_atom)
		clipboard_atom = gdk_atom_intern("CLIPBOARD", FALSE);

	gtk_selection_add_targets(GTK_WIDGET(wDraw.GetID()), clipboard_atom,
                           	targets, n_targets);

	gtk_drag_dest_set(GTK_WIDGET(wDraw.GetID()), 
		GTK_DEST_DEFAULT_ALL, targets, n_targets, 
		static_cast<GdkDragAction>(GDK_ACTION_COPY|GDK_ACTION_MOVE));
	gtk_signal_connect(GTK_OBJECT(wDraw.GetID()), 
		"drag_data_received",
		GTK_SIGNAL_FUNC(DragDataReceived),
		this);
	gtk_signal_connect(GTK_OBJECT(wDraw.GetID()), 
		"drag_motion",
		GTK_SIGNAL_FUNC(DragMotion),
		this);
	gtk_signal_connect(GTK_OBJECT(wDraw.GetID()), 
		"drag_leave",
		GTK_SIGNAL_FUNC(DragLeave),
		this);
	gtk_signal_connect(GTK_OBJECT(wDraw.GetID()), 
		"drag_end",
		GTK_SIGNAL_FUNC(DragEnd),
		this);
	gtk_signal_connect(GTK_OBJECT(wDraw.GetID()), 
		"drag_drop",
		GTK_SIGNAL_FUNC(Drop),
		this);
	// gtk_drag_source_set not used as it stops dragging over text to select it
	//gtk_drag_source_set(GTK_WIDGET(wDraw.GetID()),
	//	static_cast<GdkModifierType>(GDK_BUTTON1_MASK|GDK_BUTTON3_MASK),
	//	targets, n_targets,
	//	GDK_ACTION_COPY);
	gtk_signal_connect(GTK_OBJECT(wDraw.GetID()), 
		"drag_data_get",
		GTK_SIGNAL_FUNC(DragDataGet),
		this);
		
	SetTicking(true);
}

void ScintillaGTK::Finalise() {
	SetTicking(false);
	ScintillaBase::Finalise();
}

void ScintillaGTK::StartDrag() {
	dragWasDropped = false;
	static const GtkTargetEntry targets[] = {
                                    		{ "STRING", 0, TARGET_STRING },
                                    		{ "TEXT",   0, TARGET_TEXT },
                                    		{ "COMPOUND_TEXT", 0, TARGET_COMPOUND_TEXT },
                                    	};
	static const gint n_targets = sizeof(targets) / sizeof(targets[0]);
	GtkTargetList *tl = gtk_target_list_new(targets, n_targets);
	gtk_drag_begin(GTK_WIDGET(wDraw.GetID()),
		tl,
		static_cast<GdkDragAction>(GDK_ACTION_COPY|GDK_ACTION_MOVE),
		evbtn.button,
		reinterpret_cast<GdkEvent *>(&evbtn));
}

long ScintillaGTK::WndProc(unsigned int iMessage, unsigned long wParam, long lParam) {
	switch (iMessage) {

	case SCI_GRABFOCUS:
		gtk_widget_grab_focus(wMain.GetID());
		break;

	case SCI_GETDIRECTFUNCTION:
		return reinterpret_cast<long>(DirectFunction);
	
	case SCI_GETDIRECTPOINTER:
		return reinterpret_cast<long>(this);

	default:
		return ScintillaBase::WndProc(iMessage,wParam,lParam);
	}
	return 0l;
}

long ScintillaGTK::DefWndProc(unsigned int, unsigned long, long) {
	return 0;
}

void ScintillaGTK::SetTicking(bool on) {
	if (timer.ticking != on) {
		timer.ticking = on;
		if (timer.ticking) {
			timer.tickerID = gtk_timeout_add(timer.tickSize, TimeOut, this);
		} else {
			gtk_timeout_remove(timer.tickerID);
		}
	}
	timer.ticksToWait = caret.period;
}

void ScintillaGTK::SetMouseCapture(bool on) {
	if (mouseDownCaptures) {
		if (on) {
			gtk_grab_add(GTK_WIDGET(wDraw.GetID()));
		} else {
			gtk_grab_remove(GTK_WIDGET(wDraw.GetID()));
		}
	}
	capturedMouse = on;
}

bool ScintillaGTK::HaveMouseCapture() {
	return capturedMouse;
}

// Redraw all of text area. This paint will not be abandoned.
void ScintillaGTK::FullPaint() {
	paintState = painting;
	rcPaint = GetTextRectangle();
	paintingAllText = true;
	Surface sw;
	sw.Init((wDraw.GetID())->window);
	Paint(&sw, rcPaint);
	sw.Release();
	paintState = notPainting;
}

// Synchronously paint a rectangle of the window.
void ScintillaGTK::SyncPaint(PRectangle rc) {
	paintState = painting;
	rcPaint = rc;
	PRectangle rcText = GetTextRectangle();
	paintingAllText = rcPaint.Contains(rcText);
	Surface sw;
	sw.Init((wDraw.GetID())->window);
	Paint(&sw, rc);
	sw.Release();
	if (paintState == paintAbandoned) {
		// Painting area was insufficient to cover new styling or brace highlight positions
		FullPaint();
	}
	paintState = notPainting;
}

void ScintillaGTK::ScrollText(int linesToMove) {
	//Platform::DebugPrintf("ScintillaGTK::ScrollText %d\n", linesToMove);
	PRectangle rc = GetClientRectangle();
	int diff = vs.lineHeight * -linesToMove;
	WindowID wi = wDraw.GetID();
	GdkGC *gc = gdk_gc_new(wi->window);
	
	// Redraw exposed bit : scrolling upwards
	if (diff > 0) {
		gdk_draw_pixmap(wi->window,
			gc, wi->window,
			0, diff,
			0, 0,
			rc.Width(), rc.Height() - diff);
		// RedrawRect(PRectangle(0,rc.Height() - diff -
		//                           vs.lineHeight, rc.Width(), rc.Height()));
		gdk_gc_unref(gc);
		SyncPaint(PRectangle(0,rc.Height() - diff -
			vs.lineHeight, rc.Width(), rc.Height()));
		
	// Redraw exposed bit : scrolling downwards
	} else {
		gdk_draw_pixmap(wi->window,
			gc, wi->window,
			0, 0,
			0, -diff,
			rc.Width(), rc.Height() - diff);
		gdk_gc_unref(gc);
		// RedrawRect(PRectangle(0,0,rc.Width(),-diff + vs.lineHeight));
		SyncPaint(PRectangle(0,0,rc.Width(),-diff + vs.lineHeight));
	}
}


void ScintillaGTK::SetVerticalScrollPos() {
	gtk_adjustment_set_value(GTK_ADJUSTMENT(adjustmentv), topLine);
}

void ScintillaGTK::SetHorizontalScrollPos() {
	gtk_adjustment_set_value(GTK_ADJUSTMENT(adjustmenth), xOffset / 2);
}

bool ScintillaGTK::ModifyScrollBars(int nMax, int nPage) {
	bool modified = false;
	int pageScroll = LinesToScroll();

	if (GTK_ADJUSTMENT(adjustmentv)->upper != (nMax+1) ||
    		GTK_ADJUSTMENT(adjustmentv)->page_size != nPage ||
    		GTK_ADJUSTMENT(adjustmentv)->page_increment != pageScroll) {
		GTK_ADJUSTMENT(adjustmentv)->upper = nMax + 1;
		GTK_ADJUSTMENT(adjustmentv)->page_size = nPage;
		GTK_ADJUSTMENT(adjustmentv)->page_increment = pageScroll;
		gtk_adjustment_changed(GTK_ADJUSTMENT(adjustmentv));
		modified = true;
	}

	if (GTK_ADJUSTMENT(adjustmenth)->upper != 2000 ||
    		GTK_ADJUSTMENT(adjustmenth)->page_size != 200) {
		GTK_ADJUSTMENT(adjustmenth)->upper = 2000;
		GTK_ADJUSTMENT(adjustmenth)->page_size = 200;
		gtk_adjustment_changed(GTK_ADJUSTMENT(adjustmenth));
		modified = true;
	}
	return modified;
}

void ScintillaGTK::ReconfigureScrollBars() {
	PRectangle rc = wMain.GetClientPosition();
	Resize(rc.Width(), rc.Height());
}

void ScintillaGTK::NotifyChange() {
	gtk_signal_emit(GTK_OBJECT(sci), scintilla_signals[COMMAND_SIGNAL],
                Platform::LongFromTwoShorts(ctrlID, SCEN_CHANGE), wMain.GetID());
}

void ScintillaGTK::NotifyFocus(bool focus) {
	gtk_signal_emit(GTK_OBJECT(sci), scintilla_signals[COMMAND_SIGNAL],
                Platform::LongFromTwoShorts(ctrlID, focus ? SCEN_SETFOCUS : SCEN_KILLFOCUS), wMain.GetID());
}

void ScintillaGTK::NotifyParent(SCNotification scn) {
	scn.nmhdr.hwndFrom = wMain.GetID();
	scn.nmhdr.idFrom = ctrlID;
	gtk_signal_emit(GTK_OBJECT(sci), scintilla_signals[NOTIFY_SIGNAL],
                	ctrlID, &scn);
}

void ScintillaGTK::NotifyKey(int key, int modifiers) {
	SCNotification scn;
	scn.nmhdr.code = SCN_KEY;
	scn.ch = key;
	scn.modifiers = modifiers;

	NotifyParent(scn);
}

int ScintillaGTK::KeyDefault(int key, int modifiers) {
	if (!(modifiers & SCI_CTRL) && !(modifiers & SCI_ALT) && (key < 256)) {
		AddChar(key);
	} else {
		// Pass up to container in case it is an accelerator
		NotifyKey(key, modifiers);
	}
	//Platform::DebugPrintf("SK-key: %d %x %x\n",key, modifiers);
	return 1;
}

void ScintillaGTK::Copy() {
	if (currentPos != anchor) {
		delete []pasteBuffer;
		pasteBuffer = CopySelectionRange();
		pasteBufferIsRectangular = selType == selRectangle;
		gtk_selection_owner_set(GTK_WIDGET(wDraw.GetID()),
                        		clipboard_atom,
                        		GDK_CURRENT_TIME);
	}
}

void ScintillaGTK::Paste() {
	gtk_selection_convert(GTK_WIDGET(wDraw.GetID()),
                       	clipboard_atom,
                       	gdk_atom_intern("STRING", FALSE), GDK_CURRENT_TIME);
}

void ScintillaGTK::CreateCallTipWindow(PRectangle rc) {
	ct.wCallTip = gtk_window_new(GTK_WINDOW_POPUP);
	ct.wDraw = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(ct.wCallTip.GetID()), ct.wDraw.GetID());
	gtk_signal_connect(GTK_OBJECT(ct.wDraw.GetID()), "expose_event",
               	GtkSignalFunc(ScintillaGTK::ExposeCT), &ct);
	gtk_widget_set_events(ct.wDraw.GetID(), GDK_EXPOSURE_MASK);
	gtk_drawing_area_size(GTK_DRAWING_AREA(ct.wDraw.GetID()), 
		rc.Width(), rc.Height());
	ct.wDraw.Show();
}

void ScintillaGTK::AddToPopUp(const char *label, int cmd, bool enabled) {
	char fulllabel[200];
	strcpy(fulllabel, "/");
	strcat(fulllabel, label);
	GtkItemFactoryEntry itemEntry = {
		fulllabel, NULL, 
		GTK_SIGNAL_FUNC(ScintillaGTK::PopUpCB), cmd, 
		const_cast<gchar *>(label[0] ? "<Item>" : "<Separator>")
	};
	gtk_item_factory_create_item(GTK_ITEM_FACTORY(popup.GetID()), 
		&itemEntry, this, 1);
	if (cmd) {
		GtkWidget *item = gtk_item_factory_get_widget_by_action(
			popup.GetID(), cmd);
		if (item)
			gtk_widget_set_sensitive(item, enabled);
	}
}

bool ScintillaGTK::OwnPrimarySelection() {
	return (gdk_selection_owner_get(GDK_SELECTION_PRIMARY)
		== GTK_WIDGET(wDraw.GetID())->window);
}

void ScintillaGTK::ClaimSelection() {
	// X Windows has a 'primary selection' as well as the clipboard.
	// Whenever the user selects some text, we become the primary selection
  	if (currentPos != anchor) {
		primarySelection = true;
  		gtk_selection_owner_set(GTK_WIDGET(wDraw.GetID()), 
                                GDK_SELECTION_PRIMARY, GDK_CURRENT_TIME);
		delete []primarySelectionCopy;
		primarySelectionCopy = NULL;
	} else if (OwnPrimarySelection()) {
		if (primarySelectionCopy == NULL) 
			gtk_selection_owner_set(NULL, GDK_SELECTION_PRIMARY, GDK_CURRENT_TIME);
		primarySelection = true;
	} else {
		delete []primarySelectionCopy;
		primarySelectionCopy = NULL;
		primarySelection = false;
  	}
}

void ScintillaGTK::ReceivedSelection(GtkSelectionData *selection_data) {
	if (selection_data->type == GDK_TARGET_STRING) {
//Platform::DebugPrintf("Received String Selection %x %d\n", selection_data->selection, selection_data->length);
		if (((selection_data->selection == clipboard_atom)||
			(selection_data->selection == GDK_SELECTION_PRIMARY)) &&
    			(selection_data->length > 0)) {
			char *ptr = reinterpret_cast<char *>(selection_data->data);
			unsigned int len = selection_data->length;
			for (unsigned int i=0; i<static_cast<unsigned int>(selection_data->length); i++) {
				if ((len == static_cast<unsigned int>(selection_data->length)) && (0 == ptr[i]))
					len = i;
			}
			pdoc->BeginUndoAction();
			int selStart = SelectionStart();
                        if (selection_data->selection != GDK_SELECTION_PRIMARY) {
                                ClearSelection();
                        }
			// Check for "\n\0" ending to string indicating that selection is rectangular
			bool isRectangular = ((selection_data->length > 1) && 
				(ptr[selection_data->length-1] == 0 && ptr[selection_data->length-2] == '\n'));
			if (isRectangular) {
				PasteRectangular(selStart, ptr, len);
			} else {
				pdoc->InsertString(currentPos, ptr, len);
				SetEmptySelection(currentPos + len);
			}
			pdoc->EndUndoAction();
		}
	}
	Redraw();
}

void ScintillaGTK::ReceivedDrop(GtkSelectionData *selection_data) {
	dragWasDropped = true;
	if (selection_data->type == GDK_TARGET_STRING) {
		if (selection_data->length > 0) {
			char *ptr = reinterpret_cast<char *>(selection_data->data);
			// 3rd argument is false because the deletion of the moved data is handle by GetSelection
			bool isRectangular = ((selection_data->length > 1) && 
				(ptr[selection_data->length-1] == 0 && ptr[selection_data->length-2] == '\n'));
			DropAt(posDrop, ptr, false, isRectangular);
		}
	}
	Redraw();
}

void ScintillaGTK::GetSelection(GtkSelectionData *selection_data, guint info, char *text, bool isRectangular) {
	char *selBuffer = text;
	char *tmpBuffer = NULL; // Buffer to be freed
	
	if (selection_data->selection == GDK_SELECTION_PRIMARY) {
		if (primarySelectionCopy != NULL) {            
			selBuffer = primarySelectionCopy;
		} else {
			tmpBuffer = CopySelectionRange();
			selBuffer = tmpBuffer;
		}                
	}

	if (info == TARGET_STRING) {
		int len = strlen(selBuffer);
		// Here is a somewhat evil kludge. 
		// As I can not work out how to store data on the clipboard in multiple formats
		// and need some way to mark the clipping as being stream or rectangular,
		// the terminating \0 is included in the length for rectangular clippings.
		// All other tested aplications behave benignly by ignoring the \0.
		if (isRectangular)
			len++;	
		gtk_selection_data_set(selection_data, GDK_SELECTION_TYPE_STRING,
                       	8, reinterpret_cast<unsigned char *>(selBuffer),
                       	len);
	} else if ((info == TARGET_TEXT) || (info == TARGET_COMPOUND_TEXT)) {
		guchar *text;
		GdkAtom encoding;
		gint format;
		gint new_length;
		
		gdk_string_to_compound_text(reinterpret_cast<char *>(selBuffer), 
			&encoding, &format, &text, &new_length);
		gtk_selection_data_set(selection_data, encoding, format, text, new_length);
		gdk_free_compound_text(text);
	}

	delete []tmpBuffer;
}

void ScintillaGTK::UnclaimSelection(GdkEventSelection *selection_event) {
	if ((selection_event->selection == GDK_SELECTION_PRIMARY) && 
		!OwnPrimarySelection()) {
		delete [] primarySelectionCopy;
		primarySelectionCopy = NULL;
		primarySelection = false;
		FullPaint();
	}
}

void ScintillaGTK::Resize(int width, int height) {
	//Platform::DebugPrintf("Resize %d %d\n", width, height);
	DropGraphics();
	GtkAllocation alloc;

	// Not always needed, but some themes can have different sizes of scrollbars
	int scrollBarWidth = GTK_WIDGET(scrollbarv.GetID())->requisition.width;
	int scrollBarHeight = GTK_WIDGET(scrollbarh.GetID())->requisition.height;
	
	// These allocations should never produce negative sizes as they would wrap around to huge 
	// unsigned numbers inside GTK+ causing warnings.
	
	int horizontalScrollBarHeight = scrollBarWidth;
	if (!horizontalScrollBarVisible)
		horizontalScrollBarHeight = 0;
	
	alloc.x = 0;
	alloc.y = 0;
	alloc.width = Platform::Maximum(1, width - scrollBarWidth) + 1;
	alloc.height = Platform::Maximum(1, height - horizontalScrollBarHeight) + 1;
	gtk_widget_size_allocate(GTK_WIDGET(wDraw.GetID()), &alloc);

	alloc.x = 0;
	if (horizontalScrollBarVisible) {
		alloc.y = height - scrollBarHeight + 1;
		alloc.width = Platform::Maximum(1, width - scrollBarWidth) + 1;
		alloc.height = horizontalScrollBarHeight;
	} else {
		alloc.y = height ;
		alloc.width = 0;
		alloc.height = 0;
	}
	gtk_widget_size_allocate(GTK_WIDGET(scrollbarh.GetID()), &alloc);

	alloc.x = width - scrollBarWidth + 1;
	alloc.y = 0;
	alloc.width = scrollBarWidth;
	alloc.height = Platform::Maximum(1, height - scrollBarHeight) + 1;
	gtk_widget_size_allocate(GTK_WIDGET(scrollbarv.GetID()), &alloc);

	SetScrollBars();
}

gint ScintillaGTK::MoveResize(GtkWidget *widget, GtkAllocation *allocation, ScintillaGTK *sciThis) {
	// Platform::DebugPrintf("sci move resize %d %d\n", allocation->width, allocation->height);

	sciThis->Resize(allocation->width, allocation->height);

#ifdef USE_XIM 
  if (sciThis->ic && (gdk_ic_get_style (sciThis->ic) & GDK_IM_PREEDIT_POSITION)) {
	  gint width, height;
	  
	  gdk_window_get_size (widget->window, &width, &height);
	  sciThis->ic_attr->preedit_area.width = width;
	  sciThis->ic_attr->preedit_area.height = height;

	  gdk_ic_set_attr (sciThis->ic, sciThis->ic_attr, GDK_IC_PREEDIT_AREA);
	}
#endif

	return FALSE;
}

gint ScintillaGTK::Press(GtkWidget *, GdkEventButton *event, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("Press %x time=%d state = %x button = %x\n",sciThis,event->time, event->state, event->button);
	// Do not use GTK+ double click events as Scintilla has its own double click detection
	if (event->type != GDK_BUTTON_PRESS) 
		return FALSE;
	
	sciThis->evbtn = *event;
	Point pt;
	pt.x = int(event->x);
	pt.y = int(event->y);

	bool ctrl = event->state & GDK_CONTROL_MASK;
	
	gtk_widget_grab_focus(sciThis->wMain.GetID());
	if (event->button == 1) {
		//sciThis->ButtonDown(pt, event->time, 
		//	event->state & GDK_SHIFT_MASK, 
		//	event->state & GDK_CONTROL_MASK, 
		//	event->state & GDK_MOD1_MASK);
		// Instead of sending literal modifiers use control instead of alt
		// This is because all the window managers seem to grab alt + click for moving
		sciThis->ButtonDown(pt, event->time, 
			event->state & GDK_SHIFT_MASK, 
			event->state & GDK_CONTROL_MASK, 
			event->state & GDK_CONTROL_MASK);
	} else if (event->button == 2) {
		// Grab the primary selection if it exists
		Position pos = sciThis->PositionFromLocation(pt);
		if (sciThis->OwnPrimarySelection() && sciThis->primarySelectionCopy == NULL) 
			sciThis->primarySelectionCopy = sciThis->CopySelectionRange();

		sciThis->SetSelection(pos, pos);
		gtk_selection_convert(GTK_WIDGET(sciThis->wDraw.GetID()), GDK_SELECTION_PRIMARY,
				      gdk_atom_intern("STRING", FALSE), event->time);
	} else if (event->button == 3 && sciThis->displayPopupMenu) {
		// PopUp menu
		// Convert to screen
		int ox = 0;
		int oy = 0;
		gdk_window_get_origin(sciThis->wDraw.GetID()->window, &ox, &oy);
		sciThis->ContextMenu(Point(pt.x + ox, pt.y + oy));
	} else if (event->button == 4) {
		// Wheel scrolling up
		if (ctrl)
			gtk_adjustment_set_value(GTK_ADJUSTMENT(sciThis->adjustmenth),(
				(sciThis->xOffset) / 2 ) - 6);
		else
			gtk_adjustment_set_value(GTK_ADJUSTMENT(sciThis->adjustmentv),
				sciThis->topLine - 3);
	} else if( event->button == 5 ) {
		// Wheel scrolling down
		if (ctrl)
			gtk_adjustment_set_value(GTK_ADJUSTMENT(sciThis->adjustmenth), (
				(sciThis->xOffset) / 2 ) + 6);
		else
			gtk_adjustment_set_value(GTK_ADJUSTMENT(sciThis->adjustmentv),
				sciThis->topLine + 3);
	}
	return FALSE;
}

gint ScintillaGTK::MouseRelease(GtkWidget *, GdkEventButton *event, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("Release %x %d %d\n",sciThis,event->time,event->state);
	if (event->button == 1) {
		Point pt;
		pt.x = int(event->x);
		pt.y = int(event->y);
		//Platform::DebugPrintf("Up %x %x %d %d %d\n",
		//	sciThis,event->window,event->time, pt.x, pt.y);
		if (event->window != sciThis->wDraw.GetID()->window)
			// If mouse released on scroll bar then the position is relative to the 
			// scrollbar, not the drawing window so just repeat the most recent point.
			pt = sciThis->ptMouseLast;
		sciThis->ButtonUp(pt, event->time, event->state & 4);
	}
	return FALSE;
}

gint ScintillaGTK::Motion(GtkWidget *, GdkEventMotion *event, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("Move %x %d\n",sciThis,event->time);
	int x = 0;
	int y = 0;
	GdkModifierType state;
	if (event->is_hint) {
		gdk_window_get_pointer(event->window, &x, &y, &state);
	} else {
		x = static_cast<int>(event->x);
		y = static_cast<int>(event->y);
		state = static_cast<GdkModifierType>(event->state);
	}
	//Platform::DebugPrintf("Move %x %x %d %c %d %d\n",
	//	sciThis,event->window,event->time,event->is_hint? 'h' :'.', x, y);
	if (state & GDK_BUTTON1_MASK) {
		Point pt;
		pt.x = x;
		pt.y = y;
		sciThis->ButtonMove(pt);
	}
	return FALSE;
}

// Map the keypad keys to their equivalent functions
static int KeyTranslate(int keyIn) {
	switch (keyIn) {
		case GDK_ISO_Left_Tab:	return SCK_TAB;
		case GDK_KP_Down:	return SCK_DOWN;
		case GDK_KP_Up:		return SCK_UP;
		case GDK_KP_Left:	return SCK_LEFT;
		case GDK_KP_Right:	return SCK_RIGHT;
		case GDK_KP_Home:	return SCK_HOME;
		case GDK_KP_End:	return SCK_END;
		case GDK_KP_Page_Up:	return SCK_PRIOR;
		case GDK_KP_Page_Down:	return SCK_NEXT;
		case GDK_KP_Delete:	return SCK_DELETE;
		case GDK_KP_Insert:	return SCK_INSERT;
		case GDK_KP_Enter:	return SCK_RETURN;
			
		case GDK_Down:		return SCK_DOWN;
		case GDK_Up:		return SCK_UP;
		case GDK_Left:		return SCK_LEFT;
		case GDK_Right:		return SCK_RIGHT;
		case GDK_Home:		return SCK_HOME;
		case GDK_End:		return SCK_END;
		case GDK_Page_Up:	return SCK_PRIOR;
		case GDK_Page_Down:	return SCK_NEXT;
		case GDK_Delete:	return SCK_DELETE;
		case GDK_Insert:	return SCK_INSERT;
		case GDK_Escape:	return SCK_ESCAPE;
		case GDK_BackSpace:	return SCK_BACK;
		case GDK_Tab:		return SCK_TAB;
		case GDK_Return:	return SCK_RETURN;
		case GDK_KP_Add:	return SCK_ADD;
		case GDK_KP_Subtract:	return SCK_SUBTRACT;
		case GDK_KP_Divide:	return SCK_DIVIDE;
		default:				return keyIn;
	}
}

gint ScintillaGTK::KeyPress(GtkWidget *, GdkEventKey *event, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("SC-key: %d %x %x\n",event->keyval, event->state, GTK_WIDGET_FLAG(sciThis));
	bool shift = event->state & GDK_SHIFT_MASK;
	bool ctrl = event->state & GDK_CONTROL_MASK;
	bool alt = event->state & GDK_MOD1_MASK;
	int key = event->keyval;
	if (ctrl && (key < 128))
		key = toupper(key);
	else if (!ctrl && (key >= GDK_KP_Multiply && key <= GDK_KP_9)) 
		key &= 0x7F;
	else	
		key = KeyTranslate(key);

	sciThis->KeyDown(key, shift, ctrl, alt);
	//Platform::DebugPrintf("SK-key: %d %x %x\n",event->keyval, event->state, GTK_WIDGET_FLAGS(widget));
	return TRUE;
}

gint ScintillaGTK::KeyRelease(GtkWidget *, GdkEventKey * /*event*/, ScintillaGTK * /*sciThis*/) {
	//Platform::DebugPrintf("SC-keyrel: %d %x %3s\n",event->keyval, event->state, event->string);
	return FALSE;
}

gint ScintillaGTK::DestroyWindow(GtkWidget *, ScintillaGTK *sciThis) {
//Platform::DebugPrintf("Destroying window %x %x\n", sciThis, widget);
	sciThis->Finalise();
	delete sciThis;
	return FALSE;
}

gint ScintillaGTK::Expose(GtkWidget *, GdkEventExpose *ose, ScintillaGTK *sciThis) {
	if (sciThis->firstExpose) {
		sciThis->wDraw.SetCursor(Window::cursorText);
		sciThis->firstExpose = false;
	}
	
	sciThis->paintState = painting;
	
	sciThis->rcPaint.left = ose->area.x;
	sciThis->rcPaint.top = ose->area.y;
	sciThis->rcPaint.right = ose->area.x + ose->area.width;
	sciThis->rcPaint.bottom = ose->area.y + ose->area.height;
		
	PRectangle rcText = sciThis->GetTextRectangle();
	sciThis->paintingAllText = sciThis->rcPaint.Contains(rcText);
	Surface surfaceWindow;
	surfaceWindow.Init((sciThis->wDraw.GetID())->window);
	sciThis->Paint(&surfaceWindow, sciThis->rcPaint);
	surfaceWindow.Release();
	if (sciThis->paintState == paintAbandoned) {
		// Painting area was insufficient to cover new styling or brace highlight positions
		sciThis->FullPaint();
	}
	sciThis->paintState = notPainting;

	return FALSE;
}

void ScintillaGTK::ScrollSignal(GtkAdjustment *adj, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("Scrolly %g %x\n",adj->value,p);
	sciThis->ScrollTo((int)adj->value);
}

void ScintillaGTK::ScrollHSignal(GtkAdjustment *adj, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("Scrollyh %g %x\n",adj->value,p);
	sciThis->HorizontalScrollTo((int)adj->value * 2);
}

void ScintillaGTK::SelectionReceived(GtkWidget *,
	GtkSelectionData *selection_data, guint, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("Selection received\n");
	sciThis->ReceivedSelection(selection_data);
}

void ScintillaGTK::SelectionGet(GtkWidget *,
	GtkSelectionData *selection_data, guint info, guint, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("Selection get\n");
	sciThis->GetSelection(selection_data, info, sciThis->pasteBuffer, sciThis->pasteBufferIsRectangular);
}

void ScintillaGTK::SelectionClearEvent(GtkWidget *, GdkEventSelection *selection_event, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("Selection clear\n");
	sciThis->UnclaimSelection(selection_event);
}

void ScintillaGTK::DragBegin(GtkWidget *, GdkDragContext *, 
	ScintillaGTK *) {
	//Platform::DebugPrintf("DragBegin\n");
}

gboolean ScintillaGTK::DragMotion(GtkWidget *, GdkDragContext *context, 
	gint x, gint y, guint dragtime, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("DragMotion %d %d %x %x %x\n", x, y, 
	//	context->actions, context->suggested_action, sciThis);
	Point npt(x, y);
	sciThis->inDragDrop = true;
	sciThis->SetDragPosition(sciThis->PositionFromLocation(npt));
	gdk_drag_status(context, context->suggested_action, dragtime);
	return FALSE;
}

void ScintillaGTK::DragLeave(GtkWidget *, GdkDragContext * /*context*/, 
	guint, ScintillaGTK *sciThis) {
	sciThis->SetDragPosition(invalidPosition);
	//Platform::DebugPrintf("DragLeave %x\n", sciThis);
}

void ScintillaGTK::DragEnd(GtkWidget *, GdkDragContext * /*context*/, 
	ScintillaGTK *sciThis) {
	// If drag did not result in drop here or elsewhere
	if (!sciThis->dragWasDropped)
		sciThis->SetEmptySelection(sciThis->posDrag);
	sciThis->SetDragPosition(invalidPosition);
	//Platform::DebugPrintf("DragEnd %x %d\n", sciThis, sciThis->dragWasDropped);
}

gboolean ScintillaGTK::Drop(GtkWidget *, GdkDragContext * /*context*/, 
	gint, gint, guint, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("Drop %x\n", sciThis);
	sciThis->SetDragPosition(invalidPosition);
	return FALSE;
}

void ScintillaGTK::DragDataReceived(GtkWidget *, GdkDragContext * /*context*/,
	gint, gint, GtkSelectionData *selection_data, guint /*info*/, guint, 
	ScintillaGTK *sciThis) {
	sciThis->ReceivedDrop(selection_data);
	sciThis->SetDragPosition(invalidPosition);
}
	
void ScintillaGTK::DragDataGet(GtkWidget *, GdkDragContext *context,
	GtkSelectionData *selection_data, guint info, guint, ScintillaGTK *sciThis) {
	sciThis->dragWasDropped = true;
	if (sciThis->currentPos != sciThis->anchor) {
		sciThis->GetSelection(selection_data, info, sciThis->dragChars, sciThis->dragIsRectangle);
	}
	if (context->action == GDK_ACTION_MOVE) {
		int selStart = sciThis->SelectionStart();
		int selEnd = sciThis->SelectionEnd();
		if (sciThis->posDrop > selStart) {
			if (sciThis->posDrop > selEnd)
				sciThis->posDrop = sciThis->posDrop - (selEnd-selStart);
			else
				sciThis->posDrop = selStart;
			sciThis->posDrop = sciThis->pdoc->ClampPositionIntoDocument(sciThis->posDrop);
		}
		sciThis->ClearSelection();
	}
	sciThis->SetDragPosition(invalidPosition);
}

int ScintillaGTK::TimeOut(ScintillaGTK *sciThis) {
	sciThis->Tick();
	return 1;
}

void ScintillaGTK::PopUpCB(ScintillaGTK *sciThis, guint action, GtkWidget *) {
	if (action) {
		sciThis->Command(action);
	}
}

gint ScintillaGTK::ExposeCT(GtkWidget *widget, GdkEventExpose * /*ose*/, CallTip *ctip) {
	Surface surfaceWindow;
	//surfaceWindow.Init((ct->wCallTip.GetID())->window);
	surfaceWindow.Init(widget->window);
	ctip->PaintCT(&surfaceWindow);
	surfaceWindow.Release();
	return TRUE;
}

long ScintillaGTK::DirectFunction(
    ScintillaGTK *sciThis, unsigned int iMessage, unsigned long wParam, long lParam) {
	return sciThis->WndProc(iMessage, wParam, lParam);
}

long scintilla_send_message(ScintillaObject *sci, int iMessage, int wParam, int lParam) {
	ScintillaGTK *psci = reinterpret_cast<ScintillaGTK *>(sci->pscin);
	return psci->WndProc(iMessage, wParam, lParam);
}

static void scintilla_class_init          (ScintillaClass *klass);
static void scintilla_init                (ScintillaObject *sci);

guint scintilla_get_type() {
	static guint scintilla_type = 0;

	if (!scintilla_type) {
		GtkTypeInfo scintilla_info = {
    		"Scintilla",
    		sizeof (ScintillaObject),
    		sizeof (ScintillaClass),
    		(GtkClassInitFunc) scintilla_class_init,
    		(GtkObjectInitFunc) scintilla_init,
    		(GtkArgSetFunc) NULL,
    		(GtkArgGetFunc) NULL,
    		0
		};

		scintilla_type = gtk_type_unique(gtk_fixed_get_type(), &scintilla_info);
	}

	return scintilla_type;
}

static void scintilla_class_init(ScintillaClass *klass) {
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass*) klass;

	scintilla_signals[COMMAND_SIGNAL] = gtk_signal_new(
                                        	"command",
                                        	GTK_RUN_LAST,
                                        	object_class->type,
                                        	GTK_SIGNAL_OFFSET(ScintillaClass, command),
                                        	gtk_marshal_NONE__INT_POINTER,
                                        	GTK_TYPE_NONE,
                                        	2, GTK_TYPE_INT, GTK_TYPE_POINTER);

	scintilla_signals[NOTIFY_SIGNAL] = gtk_signal_new(
                                       	"notify",
                                       	GTK_RUN_LAST,
                                       	object_class->type,
                                       	GTK_SIGNAL_OFFSET(ScintillaClass, notify),
                                       	gtk_marshal_NONE__INT_POINTER,
                                       	GTK_TYPE_NONE,
                                       	2, GTK_TYPE_INT, GTK_TYPE_POINTER);

	gtk_object_class_add_signals(object_class,
                             	reinterpret_cast<unsigned int *>(scintilla_signals), LAST_SIGNAL);

	klass->command = NULL;
	klass->notify = NULL;
}

static void scintilla_init(ScintillaObject *sci) {
	GTK_WIDGET_SET_FLAGS(sci, GTK_CAN_FOCUS);
	sci->pscin = new ScintillaGTK(sci);
}

GtkWidget* scintilla_new() {
	return GTK_WIDGET(gtk_type_new(scintilla_get_type()));
}

void scintilla_set_id(ScintillaObject *sci,int id) {
	ScintillaGTK *psci = reinterpret_cast<ScintillaGTK *>(sci->pscin);
	psci->ctrlID = id;
}
