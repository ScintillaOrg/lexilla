// Scintilla source code edit control
// ScintillaGTK.cxx - GTK+ specific subclass of ScintillaBase
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "Platform.h"

#if PLAT_GTK_WIN32
#include "Windows.h"
#endif

#include "Scintilla.h"
#include "ScintillaWidget.h"
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
#include "SString.h"
#include "ScintillaBase.h"

#include "gtk/gtksignal.h"

#ifdef _MSC_VER
// Constant conditional expressions are because of GTK+ headers
#pragma warning(disable: 4127)
// Ignore unreferenced local functions in GTK+ headers
#pragma warning(disable: 4505)
#endif

class ScintillaGTK : public ScintillaBase {
	_ScintillaObject *sci;
	Window scrollbarv;
	Window scrollbarh;
	GtkObject *adjustmentv;
	GtkObject *adjustmenth;
	int scrollBarWidth;
	int scrollBarHeight;
	
	// Because clipboard access is asynchronous, copyText is created by Copy
	SelectionText copyText;
	
	SelectionText primary;
	
	GdkEventButton evbtn;
	bool capturedMouse;
	bool dragWasDropped;

	GtkWidgetClass *parentClass;

	static GdkAtom clipboard_atom;
 
#if PLAT_GTK_WIN32
	CLIPFORMAT cfColumnSelect;
#endif

	// Input context used for supporting internationalized key entry
	GdkIC *ic;
	GdkICAttr *ic_attr;

	// Wheel mouse support
	unsigned int linesPerScroll;
	GTimeVal lastWheelMouseTime;
	gint lastWheelMouseDirection;       
	gint wheelMouseIntensity;

	// Private so ScintillaGTK objects can not be copied
ScintillaGTK(const ScintillaGTK &) : ScintillaBase() {}
	ScintillaGTK &operator=(const ScintillaGTK &) { return * this; }

public:
	ScintillaGTK(_ScintillaObject *sci_);
	virtual ~ScintillaGTK();
	static void ClassInit(GtkObjectClass* object_class, GtkWidgetClass *widget_class);

private:
	virtual void Initialise();
	virtual void Finalise();
	virtual void StartDrag();
public: 	// Public for scintilla_send_message
	virtual sptr_t WndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
private:
	virtual sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
	virtual void SetTicking(bool on);
	virtual void SetMouseCapture(bool on);
	virtual bool HaveMouseCapture();
	void FullPaint();
	virtual PRectangle GetClientRectangle();
	void SyncPaint(PRectangle rc);
	virtual void ScrollText(int linesToMove);
	virtual void SetVerticalScrollPos();
	virtual void SetHorizontalScrollPos();
	virtual bool ModifyScrollBars(int nMax, int nPage);
	void ReconfigureScrollBars();
	virtual void NotifyChange();
	virtual void NotifyFocus(bool focus);
	virtual void NotifyParent(SCNotification scn);
	void NotifyKey(int key, int modifiers);
	void NotifyURIDropped(const char *list);
	virtual int KeyDefault(int key, int modifiers);
	virtual void Copy();
	virtual void Paste();
	virtual void CreateCallTipWindow(PRectangle rc);
	virtual void AddToPopUp(const char *label, int cmd = 0, bool enabled = true);
	bool OwnPrimarySelection();
	virtual void ClaimSelection();
	void ReceivedSelection(GtkSelectionData *selection_data);
	void ReceivedDrop(GtkSelectionData *selection_data);
	void GetSelection(GtkSelectionData *selection_data, guint info, SelectionText *selected);
	void UnclaimSelection(GdkEventSelection *selection_event);
	void Resize(int width, int height);

	// Callback functions
	void RealizeThis(GtkWidget *widget);
	static void Realize(GtkWidget *widget);
	void UnRealizeThis(GtkWidget *widget);
	static void UnRealize(GtkWidget *widget);
	void MapThis();
	static void Map(GtkWidget *widget);
	void UnMapThis();
	static void UnMap(GtkWidget *widget);
	static gint CursorMoved(GtkWidget *widget, int xoffset, int yoffset, ScintillaGTK *sciThis);
	static gint FocusIn(GtkWidget *widget, GdkEventFocus *event);
	static gint FocusOut(GtkWidget *widget, GdkEventFocus *event);
	static void SizeRequest(GtkWidget *widget, GtkRequisition *requisition);
	static void SizeAllocate(GtkWidget *widget, GtkAllocation *allocation);
	static gint Expose(GtkWidget *widget, GdkEventExpose *ose, ScintillaGTK *sciThis);
	static gint ExposeMain(GtkWidget *widget, GdkEventExpose *ose);
	static void Draw(GtkWidget *widget, GdkRectangle *area);

	static void ScrollSignal(GtkAdjustment *adj, ScintillaGTK *sciThis);
	static void ScrollHSignal(GtkAdjustment *adj, ScintillaGTK *sciThis);
	gint PressThis(GdkEventButton *event);
	static gint Press(GtkWidget *widget, GdkEventButton *event);
	static gint MouseRelease(GtkWidget *widget, GdkEventButton *event);
#if PLAT_GTK_WIN32
	static gint ScrollEvent(GtkWidget *widget, GdkEventScroll *event);
#endif
	static gint Motion(GtkWidget *widget, GdkEventMotion *event);
	static gint KeyPress(GtkWidget *widget, GdkEventKey *event);
	static gint KeyRelease(GtkWidget *widget, GdkEventKey *event);
	static void Destroy(GtkObject *object);
	static void SelectionReceived(GtkWidget *widget, GtkSelectionData *selection_data,
	                              guint time);
	static void SelectionGet(GtkWidget *widget, GtkSelectionData *selection_data,
	                         guint info, guint time);
	static gint SelectionClear(GtkWidget *widget, GdkEventSelection *selection_event);
	static gint SelectionNotify(GtkWidget *widget, GdkEventSelection *selection_event);
	static void DragBegin(GtkWidget *widget, GdkDragContext *context);
	static gboolean DragMotion(GtkWidget *widget, GdkDragContext *context,
	                           gint x, gint y, guint time);
	static void DragLeave(GtkWidget *widget, GdkDragContext *context,
	                      guint time);
	static void DragEnd(GtkWidget *widget, GdkDragContext *context);
	static gboolean Drop(GtkWidget *widget, GdkDragContext *context,
	                     gint x, gint y, guint time);
	static void DragDataReceived(GtkWidget *widget, GdkDragContext *context,
	                             gint x, gint y, GtkSelectionData *selection_data, guint info, guint time);
	static void DragDataGet(GtkWidget *widget, GdkDragContext *context,
	                        GtkSelectionData *selection_data, guint info, guint time);
	static gint TimeOut(ScintillaGTK *sciThis);
	static void PopUpCB(ScintillaGTK *sciThis, guint action, GtkWidget *widget);
	static gint ExposeCT(GtkWidget *widget, GdkEventExpose *ose, CallTip *ct);
	static sptr_t DirectFunction(ScintillaGTK *sciThis,
	                             unsigned int iMessage, uptr_t wParam, sptr_t lParam);
};

enum {
    COMMAND_SIGNAL,
    NOTIFY_SIGNAL,
    LAST_SIGNAL
};

static gint scintilla_signals[LAST_SIGNAL] = { 0 };
static GtkWidgetClass* parent_class = NULL;

GdkAtom ScintillaGTK::clipboard_atom = GDK_NONE;

enum {
    TARGET_STRING,
    TARGET_TEXT,
    TARGET_COMPOUND_TEXT
};

static GtkWidget *PWidget(Window &w) { 
	return reinterpret_cast<GtkWidget *>(w.GetID()); 
}

static ScintillaGTK *ScintillaFromWidget(GtkWidget *widget) {
	ScintillaObject *scio = reinterpret_cast<ScintillaObject *>(widget);
	return reinterpret_cast<ScintillaGTK *>(scio->pscin);
}

ScintillaGTK::ScintillaGTK(_ScintillaObject *sci_) :
		adjustmentv(0), adjustmenth(0),
		scrollBarWidth(30), scrollBarHeight(30),
		capturedMouse(false), dragWasDropped(false),
		parentClass(0),
		ic(NULL), ic_attr(NULL), lastWheelMouseDirection(0),
		wheelMouseIntensity(0) {
	sci = sci_;
	wMain = GTK_WIDGET(sci);

#if PLAT_GTK_WIN32       
 	// There does not seem to be a real standard for indicating that the clipboard
	// contains a rectangular selection, so copy Developer Studio.
	cfColumnSelect = static_cast<CLIPFORMAT>(
		::RegisterClipboardFormat("MSDEVColumnSelect"));

  	// Get intellimouse parameters when running on win32; otherwise use
	// reasonable default     
#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif
	::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &linesPerScroll, 0);
#else
	linesPerScroll = 4;
#endif
	lastWheelMouseTime.tv_sec = 0;
	lastWheelMouseTime.tv_usec = 0;

	Initialise();
}

ScintillaGTK::~ScintillaGTK() {
}

void ScintillaGTK::RealizeThis(GtkWidget *widget) {
	//Platform::DebugPrintf("ScintillaGTK::realize this\n");
	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);
	GdkWindowAttr attrs;
	attrs.window_type = GDK_WINDOW_CHILD;
	attrs.x = widget->allocation.x;
	attrs.y = widget->allocation.y;
	attrs.width = widget->allocation.width;
	attrs.height = widget->allocation.height;
	attrs.wclass = GDK_INPUT_OUTPUT;
	attrs.visual = gtk_widget_get_visual(widget);
	attrs.colormap = gtk_widget_get_colormap(widget);
	attrs.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;
	GdkCursor *cursor = gdk_cursor_new(GDK_XTERM);
	attrs.cursor = cursor;
	widget->window = gdk_window_new(gtk_widget_get_parent_window(widget), &attrs,
	                                GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP | GDK_WA_CURSOR);
	gdk_window_set_user_data(widget->window, widget);
	gdk_window_set_background(widget->window, &widget->style->bg[GTK_STATE_NORMAL]);
	gdk_window_show(widget->window);
	gdk_cursor_destroy(cursor);

	if (gdk_im_ready() && (ic_attr = gdk_ic_attr_new()) != NULL) {
		gint width, height;
		GdkColormap *colormap;
		GdkEventMask mask;
		GdkICAttr *attr = ic_attr;
		GdkICAttributesType attrmask = GDK_IC_ALL_REQ;
		GdkIMStyle style;
		GdkIMStyle supported_style = (GdkIMStyle) (GDK_IM_PREEDIT_NONE |
		                             GDK_IM_PREEDIT_NOTHING |
		                             GDK_IM_PREEDIT_POSITION |
		                             GDK_IM_STATUS_NONE |
		                             GDK_IM_STATUS_NOTHING);

		if (widget->style && widget->style->font->type != GDK_FONT_FONTSET)
			supported_style = (GdkIMStyle) ((int) supported_style & ~GDK_IM_PREEDIT_POSITION);

		attr->style = style = gdk_im_decide_style(supported_style);
		attr->client_window = widget->window;

		if ((colormap = gtk_widget_get_colormap (widget)) != gtk_widget_get_default_colormap ()) {
			attrmask = (GdkICAttributesType) ((int) attrmask | GDK_IC_PREEDIT_COLORMAP);
			attr->preedit_colormap = colormap;
		}

		switch (style & GDK_IM_PREEDIT_MASK) {
		case GDK_IM_PREEDIT_POSITION:
			if (widget->style && widget->style->font->type != GDK_FONT_FONTSET)	{
				g_warning("over-the-spot style requires fontset");
				break;
			}

			attrmask = (GdkICAttributesType) ((int) attrmask | GDK_IC_PREEDIT_POSITION_REQ);
			gdk_window_get_size(widget->window, &width, &height);
			attr->spot_location.x = 0;
			attr->spot_location.y = height;
			attr->preedit_area.x = 0;
			attr->preedit_area.y = 0;
			attr->preedit_area.width = width;
			attr->preedit_area.height = height;
			attr->preedit_fontset = widget->style->font;

			break;
		}
		ic = gdk_ic_new(attr, attrmask);

		if (ic == NULL)
			g_warning("Can't create input context.");
		else {
			mask = gdk_window_get_events(widget->window);
			mask = (GdkEventMask) ((int) mask | gdk_ic_get_events(ic));
			gdk_window_set_events(widget->window, mask);

			if (GTK_WIDGET_HAS_FOCUS(widget))
				gdk_im_begin(ic, widget->window);
		}
	}
	gtk_widget_realize(PWidget(scrollbarv));
	gtk_widget_realize(PWidget(scrollbarh));
}

void ScintillaGTK::Realize(GtkWidget *widget) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	sciThis->RealizeThis(widget);
}

void ScintillaGTK::UnRealizeThis(GtkWidget *widget) {
	if (GTK_WIDGET_MAPPED(widget)) {
		gtk_widget_unmap(widget);
	}
	GTK_WIDGET_UNSET_FLAGS(widget, GTK_REALIZED);
	gtk_widget_unrealize(PWidget(scrollbarv));
	gtk_widget_unrealize(PWidget(scrollbarh));
	if (ic) {
		gdk_ic_destroy(ic);
		ic = NULL;
	}
	if (ic_attr) {
		gdk_ic_attr_destroy(ic_attr);
		ic_attr = NULL;
	}
	if (GTK_WIDGET_CLASS(parentClass)->unrealize)
		GTK_WIDGET_CLASS(parentClass)->unrealize(widget);
	//gdk_window_destroy(widget->window);
	//widget->window = 0;

	Finalise();
}

void ScintillaGTK::UnRealize(GtkWidget *widget) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	sciThis->UnRealizeThis(widget);
}

static void MapWidget(GtkWidget *widget) {
	if (widget &&
	        GTK_WIDGET_VISIBLE(widget) &&
	        !GTK_WIDGET_MAPPED(widget)) {
		gtk_widget_map(widget);
	}
}

void ScintillaGTK::MapThis() {
	//Platform::DebugPrintf("ScintillaGTK::map this\n");
	GTK_WIDGET_SET_FLAGS(PWidget(wMain), GTK_MAPPED);
	MapWidget(PWidget(scrollbarh));
	MapWidget(PWidget(scrollbarv));
	scrollbarv.SetCursor(Window::cursorReverseArrow);
	scrollbarh.SetCursor(Window::cursorReverseArrow);
	gdk_window_show(PWidget(wMain)->window);
}

void ScintillaGTK::Map(GtkWidget *widget) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	sciThis->MapThis();
}

void ScintillaGTK::UnMapThis() {
	//Platform::DebugPrintf("ScintillaGTK::unmap this\n");
	GTK_WIDGET_UNSET_FLAGS(PWidget(wMain), GTK_MAPPED);
	gdk_window_hide(PWidget(wMain)->window);
	gtk_widget_unmap(PWidget(scrollbarh));
	gtk_widget_unmap(PWidget(scrollbarv));
}

void ScintillaGTK::UnMap(GtkWidget *widget) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	sciThis->UnMapThis();
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

gint ScintillaGTK::FocusIn(GtkWidget *widget, GdkEventFocus * /*event*/) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("ScintillaGTK::focus in %x\n", sciThis);
	GTK_WIDGET_SET_FLAGS(widget, GTK_HAS_FOCUS);
	sciThis->SetFocusState(true);

	if (sciThis->ic)
		gdk_im_begin(sciThis->ic, widget->window);

	return FALSE;
}

gint ScintillaGTK::FocusOut(GtkWidget *widget, GdkEventFocus * /*event*/) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("ScintillaGTK::focus out %x\n", sciThis);
	GTK_WIDGET_UNSET_FLAGS(widget, GTK_HAS_FOCUS);
	sciThis->SetFocusState(false);

	gdk_im_end();

	return FALSE;
}

void ScintillaGTK::SizeRequest(GtkWidget *widget, GtkRequisition *requisition) {
	requisition->width = 2000;
	requisition->height = 2000;
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	GtkRequisition child_requisition;
	gtk_widget_size_request(PWidget(sciThis->scrollbarh), &child_requisition);
	gtk_widget_size_request(PWidget(sciThis->scrollbarv), &child_requisition);
}

void ScintillaGTK::SizeAllocate(GtkWidget *widget, GtkAllocation *allocation) {
	//Platform::DebugPrintf(stderr, "size_allocate %p %0d,%0d %0d,%0d\n", widget,
	//	widget->allocation.x, widget->allocation.y, allocation->width, allocation->height);
	widget->allocation = *allocation;
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	if (GTK_WIDGET_REALIZED(widget))
		gdk_window_move_resize(widget->window,
		                       widget->allocation.x,
		                       widget->allocation.y,
		                       widget->allocation.width,
		                       widget->allocation.height);

	sciThis->Resize(allocation->width, allocation->height);

	if (sciThis->ic && (gdk_ic_get_style (sciThis->ic) & GDK_IM_PREEDIT_POSITION)) {
		gint width, height;

		gdk_window_get_size(widget->window, &width, &height);
		sciThis->ic_attr->preedit_area.width = width;
		sciThis->ic_attr->preedit_area.height = height;

		gdk_ic_set_attr(sciThis->ic, sciThis->ic_attr, GDK_IC_PREEDIT_AREA);
	}
}

void ScintillaGTK::Initialise() {
	//Platform::DebugPrintf("ScintillaGTK::Initialise\n");
	parentClass = reinterpret_cast<GtkWidgetClass *>(
	                  gtk_type_class(gtk_container_get_type()));

	GTK_WIDGET_SET_FLAGS(PWidget(wMain), GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS(GTK_WIDGET(PWidget(wMain)), GTK_SENSITIVE);
	gtk_widget_set_events(PWidget(wMain),
	                      GDK_EXPOSURE_MASK
	                      | GDK_STRUCTURE_MASK
	                      | GDK_KEY_PRESS_MASK
	                      | GDK_KEY_RELEASE_MASK
	                      | GDK_FOCUS_CHANGE_MASK
	                      | GDK_LEAVE_NOTIFY_MASK
	                      | GDK_BUTTON_PRESS_MASK
	                      | GDK_BUTTON_RELEASE_MASK
	                      | GDK_POINTER_MOTION_MASK
	                      | GDK_POINTER_MOTION_HINT_MASK);

	adjustmentv = gtk_adjustment_new(0.0, 0.0, 201.0, 1.0, 20.0, 20.0);
	scrollbarv = gtk_vscrollbar_new(GTK_ADJUSTMENT(adjustmentv));
	GTK_WIDGET_UNSET_FLAGS(PWidget(scrollbarv), GTK_CAN_FOCUS);
	gtk_signal_connect(GTK_OBJECT(adjustmentv), "value_changed",
	                   GTK_SIGNAL_FUNC(ScrollSignal), this);
	gtk_widget_set_parent(PWidget(scrollbarv), PWidget(wMain));
	gtk_widget_show(PWidget(scrollbarv));

	adjustmenth = gtk_adjustment_new(0.0, 0.0, 101.0, 1.0, 20.0, 20.0);
	scrollbarh = gtk_hscrollbar_new(GTK_ADJUSTMENT(adjustmenth));
	GTK_WIDGET_UNSET_FLAGS(PWidget(scrollbarh), GTK_CAN_FOCUS);
	gtk_signal_connect(GTK_OBJECT(adjustmenth), "value_changed",
	                   GTK_SIGNAL_FUNC(ScrollHSignal), this);
	gtk_widget_set_parent(PWidget(scrollbarh), PWidget(wMain));
	gtk_widget_show(PWidget(scrollbarh));

	gtk_widget_grab_focus(PWidget(wMain));

	static const GtkTargetEntry targets[] = {
	    { "STRING", 0, TARGET_STRING },
	    { "TEXT", 0, TARGET_TEXT },
	    { "COMPOUND_TEXT", 0, TARGET_COMPOUND_TEXT },
	    { "text/uri-list", 0, 0 },
	};
	static const gint n_targets = sizeof(targets) / sizeof(targets[0]);

	gtk_selection_add_targets(GTK_WIDGET(PWidget(wMain)), GDK_SELECTION_PRIMARY,
	                          targets, n_targets);

	if (!clipboard_atom)
		clipboard_atom = gdk_atom_intern("CLIPBOARD", FALSE);

	gtk_selection_add_targets(GTK_WIDGET(PWidget(wMain)), clipboard_atom,
	                          targets, n_targets);

	gtk_drag_dest_set(GTK_WIDGET(PWidget(wMain)),
	                  GTK_DEST_DEFAULT_ALL, targets, n_targets,
	                  static_cast<GdkDragAction>(GDK_ACTION_COPY | GDK_ACTION_MOVE));

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
	    { "TEXT", 0, TARGET_TEXT },
	    { "COMPOUND_TEXT", 0, TARGET_COMPOUND_TEXT },
	};
	static const gint n_targets = sizeof(targets) / sizeof(targets[0]);
	GtkTargetList *tl = gtk_target_list_new(targets, n_targets);
	gtk_drag_begin(GTK_WIDGET(PWidget(wMain)),
	               tl,
	               static_cast<GdkDragAction>(GDK_ACTION_COPY | GDK_ACTION_MOVE),
	               evbtn.button,
	               reinterpret_cast<GdkEvent *>(&evbtn));
}

sptr_t ScintillaGTK::WndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	switch (iMessage) {

	case SCI_GRABFOCUS:
		gtk_widget_grab_focus(PWidget(wMain));
		break;

	case SCI_GETDIRECTFUNCTION:
		return reinterpret_cast<sptr_t>(DirectFunction);

	case SCI_GETDIRECTPOINTER:
		return reinterpret_cast<sptr_t>(this);

	default:
		return ScintillaBase::WndProc(iMessage, wParam, lParam);
	}
	return 0l;
}

sptr_t ScintillaGTK::DefWndProc(unsigned int, uptr_t, sptr_t) {
	return 0;
}

void ScintillaGTK::SetTicking(bool on) {
	if (timer.ticking != on) {
		timer.ticking = on;
		if (timer.ticking) {
			timer.tickerID = gtk_timeout_add(timer.tickSize, (GtkFunction)TimeOut, this);
		} else {
			gtk_timeout_remove(timer.tickerID);
		}
	}
	timer.ticksToWait = caret.period;
}

void ScintillaGTK::SetMouseCapture(bool on) {
	if (mouseDownCaptures) {
		if (on) {
			gtk_grab_add(GTK_WIDGET(PWidget(wMain)));
		} else {
			gtk_grab_remove(GTK_WIDGET(PWidget(wMain)));
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
	//Platform::DebugPrintf("ScintillaGTK::FullPaint %0d,%0d %0d,%0d\n",
	//	rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom);
	paintingAllText = true;
	Surface *sw = Surface::Allocate();
	if (sw) {
		sw->Init((PWidget(wMain))->window);
		Paint(sw, rcPaint);
		sw->Release();
		delete sw;
	}
	paintState = notPainting;
}

PRectangle ScintillaGTK::GetClientRectangle() {
	PRectangle rc = wMain.GetClientPosition();
	rc.right -= scrollBarWidth + 1;
	if (horizontalScrollBarVisible)
		rc.bottom -= scrollBarHeight + 1;
	// Move to origin
	rc.right -= rc.left;
	rc.bottom -= rc.top;
	rc.left = 0;
	rc.top = 0;
	return rc;
}

// Synchronously paint a rectangle of the window.
void ScintillaGTK::SyncPaint(PRectangle rc) {
	paintState = painting;
	rcPaint = rc;
	PRectangle rcText = GetTextRectangle();
	paintingAllText = rcPaint.Contains(rcText);
	//Platform::DebugPrintf("ScintillaGTK::SyncPaint %0d,%0d %0d,%0d\n",
	//	rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom);
	Surface *sw = Surface::Allocate();
	if (sw) {
		sw->Init((PWidget(wMain))->window);
		Paint(sw, rc);
		sw->Release();
		delete sw;
	}
	if (paintState == paintAbandoned) {
		// Painting area was insufficient to cover new styling or brace highlight positions
		FullPaint();
	}
	paintState = notPainting;
}

void ScintillaGTK::ScrollText(int linesToMove) {
	PRectangle rc = GetClientRectangle();
	int diff = vs.lineHeight * -linesToMove;
	//Platform::DebugPrintf("ScintillaGTK::ScrollText %d %d %0d,%0d %0d,%0d\n", linesToMove, diff,
	//	rc.left, rc.top, rc.right, rc.bottom);
	GtkWidget *wi = PWidget(wMain);
	GdkGC *gc = gdk_gc_new(wi->window);
	GdkEvent* event;

	// Set up gc so we get GraphicsExposures from gdk_draw_pixmap
	//  which calls XCopyArea
	gdk_gc_set_exposures(gc, TRUE);

	// Redraw exposed bit : scrolling upwards
	if (diff > 0) {
		gdk_draw_pixmap(wi->window,
		                gc, wi->window,
		                0, diff,
		                0, 0,
		                rc.Width()-1, rc.Height() - diff);
		SyncPaint(PRectangle(0, rc.Height() - diff - vs.lineHeight,
		                     rc.Width(), rc.Height()+1));

	// Redraw exposed bit : scrolling downwards
	} else {
		gdk_draw_pixmap(wi->window,
		                gc, wi->window,
		                0, 0,
		                0, -diff,
		                rc.Width()-1, rc.Height() + diff);
		SyncPaint(PRectangle(0, 0, rc.Width(), -diff + vs.lineHeight));
	}

	// Look for any graphics expose
	while ((event = gdk_event_get_graphics_expose(wi->window)) != NULL) {
		gtk_widget_event(wi, event);
		if (event->expose.count == 0) {
			gdk_event_free(event);
			break;
		}
		gdk_event_free(event);
	}

	gdk_gc_unref(gc);
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

	if (GTK_ADJUSTMENT(adjustmentv)->upper != (nMax + 1) ||
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
	                Platform::LongFromTwoShorts(ctrlID, SCEN_CHANGE), PWidget(wMain));
}

void ScintillaGTK::NotifyFocus(bool focus) {
	gtk_signal_emit(GTK_OBJECT(sci), scintilla_signals[COMMAND_SIGNAL],
	                Platform::LongFromTwoShorts(ctrlID, focus ? SCEN_SETFOCUS : SCEN_KILLFOCUS), PWidget(wMain));
}

void ScintillaGTK::NotifyParent(SCNotification scn) {
	scn.nmhdr.hwndFrom = PWidget(wMain);
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

void ScintillaGTK::NotifyURIDropped(const char *list) {
	SCNotification scn;
	scn.nmhdr.code = SCN_URIDROPPED;
	scn.text = list;

	NotifyParent(scn);
}

int ScintillaGTK::KeyDefault(int key, int modifiers) {
	if (!(modifiers & SCI_CTRL) && !(modifiers & SCI_ALT) && (key < 256)) {
		AddChar(key);
		return 1;
	} else {
		// Pass up to container in case it is an accelerator
		NotifyKey(key, modifiers);
		return 0;
	}
	//Platform::DebugPrintf("SK-key: %d %x %x\n",key, modifiers);
}

void ScintillaGTK::Copy() {
	if (currentPos != anchor) {
		CopySelectionRange(&copyText);
		gtk_selection_owner_set(GTK_WIDGET(PWidget(wMain)),
		                        clipboard_atom,
		                        GDK_CURRENT_TIME);
#if PLAT_GTK_WIN32              
		if (selType == selRectangle) {
			::OpenClipboard(NULL);
			::SetClipboardData(cfColumnSelect, 0);
			::CloseClipboard();
		}
#endif              
	}
}

void ScintillaGTK::Paste() {
	gtk_selection_convert(GTK_WIDGET(PWidget(wMain)),
	                      clipboard_atom,
	                      gdk_atom_intern("STRING", FALSE), GDK_CURRENT_TIME);
}

void ScintillaGTK::CreateCallTipWindow(PRectangle rc) {
	ct.wCallTip = gtk_window_new(GTK_WINDOW_POPUP);
	ct.wDraw = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(PWidget(ct.wCallTip)), PWidget(ct.wDraw));
	gtk_signal_connect(GTK_OBJECT(PWidget(ct.wDraw)), "expose_event",
	                   GtkSignalFunc(ScintillaGTK::ExposeCT), &ct);
	gtk_widget_set_events(PWidget(ct.wDraw), GDK_EXPOSURE_MASK);
	gtk_drawing_area_size(GTK_DRAWING_AREA(PWidget(ct.wDraw)),
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
		                      reinterpret_cast<GtkItemFactory *>(popup.GetID()), cmd);
		if (item)
			gtk_widget_set_sensitive(item, enabled);
	}
}

bool ScintillaGTK::OwnPrimarySelection() {
	return (gdk_selection_owner_get(GDK_SELECTION_PRIMARY)
	        == GTK_WIDGET(PWidget(wMain))->window);
}

void ScintillaGTK::ClaimSelection() {
	// X Windows has a 'primary selection' as well as the clipboard.
	// Whenever the user selects some text, we become the primary selection
	if (currentPos != anchor) {
		primarySelection = true;
		gtk_selection_owner_set(GTK_WIDGET(PWidget(wMain)),
		                        GDK_SELECTION_PRIMARY, GDK_CURRENT_TIME);
		primary.Set(0, 0);
	} else if (OwnPrimarySelection()) {
		primarySelection = true;
		if (primary.s == NULL)
			gtk_selection_owner_set(NULL, GDK_SELECTION_PRIMARY, GDK_CURRENT_TIME);
	} else {
		primarySelection = false;
		primary.Set(0, 0);
	}
}

void ScintillaGTK::ReceivedSelection(GtkSelectionData *selection_data) {
	if (selection_data->type == GDK_TARGET_STRING) {
		//Platform::DebugPrintf("Received String Selection %x %d\n", selection_data->selection, selection_data->length);
		if (((selection_data->selection == clipboard_atom) ||
		        (selection_data->selection == GDK_SELECTION_PRIMARY)) &&
		        (selection_data->length > 0)) {
			char *ptr = reinterpret_cast<char *>(selection_data->data);
			unsigned int len = selection_data->length;
			unsigned int i;
			for (i = 0; i < static_cast<unsigned int>(selection_data->length); i++) {
				if ((len == static_cast<unsigned int>(selection_data->length)) && (0 == ptr[i]))
					len = i;
			}                     
#if PLAT_GTK_WIN32
			/* Need to convert \n to correct newline form for this file as win32gtk always returns */
			/* only \n line delimiter from clipboard */
			char *tmpstr = new char [2 * len];
			char *dptr = tmpstr;
			char *sptr = ptr;
			unsigned int newlen = len;                     
			for (i = 0; i < len; i++) {
				if (*sptr == '\n') {
					if (pdoc->eolMode != SC_EOL_LF)
						*dptr++ = '\r';
					if (pdoc->eolMode != SC_EOL_CR)
						*dptr++ = '\n';
					if (pdoc->eolMode == SC_EOL_CRLF)
						newlen++;
					sptr++;
				}
				else {
					*dptr++ = *sptr++;
				}
			}
			ptr = tmpstr;
			len = newlen;
#endif
			pdoc->BeginUndoAction();
			int selStart = SelectionStart();
			if (selection_data->selection != GDK_SELECTION_PRIMARY) {
				ClearSelection();
			}
			// Check for "\n\0" ending to string indicating that selection is rectangular
#if PLAT_GTK_WIN32
			bool isRectangular = ::IsClipboardFormatAvailable(cfColumnSelect);
#else
			bool isRectangular = ((selection_data->length > 1) &&
			                      (ptr[selection_data->length - 1] == 0 && ptr[selection_data->length - 2] == '\n'));
#endif
			if (isRectangular) {
				PasteRectangular(selStart, ptr, len);
			} else {
				pdoc->InsertString(currentPos, ptr, len);
				SetEmptySelection(currentPos + len);
			}
			pdoc->EndUndoAction();
#if PLAT_GTK_WIN32
			delete []tmpstr;
#endif
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
#if PLAT_GTK_WIN32
			bool isRectangular = ::IsClipboardFormatAvailable(cfColumnSelect);
#else
			bool isRectangular = ((selection_data->length > 1) &&
			                      (ptr[selection_data->length - 1] == 0 && ptr[selection_data->length - 2] == '\n'));
#endif
			DropAt(posDrop, ptr, false, isRectangular);
		}
	} else {
		char *ptr = reinterpret_cast<char *>(selection_data->data);
		NotifyURIDropped(ptr);
	}
	Redraw();
}

void ScintillaGTK::GetSelection(GtkSelectionData *selection_data, guint info, SelectionText *text) {
	if (selection_data->selection == GDK_SELECTION_PRIMARY) {
		if (primary.s == NULL) {
			CopySelectionRange(&primary);
		}
		text = &primary;
	}

	char *selBuffer = text->s;

#if PLAT_GTK_WIN32
	// win32gtk requires \n delimited lines and doesn't work right with
	// other line formats, so make a copy of the clip text now with 
	// newlines converted
	char *tmpstr = new char[text->len + 1];
	char *sptr = selBuffer;
	char *dptr = tmpstr;
	while (*sptr != '\0') {
		if (pdoc->eolMode == SC_EOL_CR && *sptr == '\r') {
			*dptr++ = '\n';
			sptr++;
		}
		else if (pdoc->eolMode != SC_EOL_CR && *sptr == '\r') {
			sptr++;
		}
		else {
			*dptr++ = *sptr++;
		}
	}
	*dptr = '\0';       
	selBuffer = tmpstr;
#endif

	if (info == TARGET_STRING) {
		int len = strlen(selBuffer);
		// Here is a somewhat evil kludge.
		// As I can not work out how to store data on the clipboard in multiple formats
		// and need some way to mark the clipping as being stream or rectangular,
		// the terminating \0 is included in the length for rectangular clippings.
		// All other tested aplications behave benignly by ignoring the \0.
		// The #if is here because on Windows cfColumnSelect clip entry is used 
                // instead as standard indicator of rectangularness (so no need to kludge)
#if PLAT_GTK_WIN32 == 0
		if (text->rectangular)
			len++;
#endif
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

#if PLAT_GTK_WIN32
	delete []tmpstr;
#endif
}

void ScintillaGTK::UnclaimSelection(GdkEventSelection *selection_event) {
	//Platform::DebugPrintf("UnclaimSelection\n");
	if (selection_event->selection == GDK_SELECTION_PRIMARY) {
		//Platform::DebugPrintf("UnclaimPrimarySelection\n");
		if (!OwnPrimarySelection()) {
			primary.Set(0, 0);
			primarySelection = false;
			FullPaint();
		}
	}
}

void ScintillaGTK::Resize(int width, int height) {
	//Platform::DebugPrintf("Resize %d %d\n", width, height);
	//printf("Resize %d %d\n", width, height);
	DropGraphics();

	// Not always needed, but some themes can have different sizes of scrollbars
	scrollBarWidth = GTK_WIDGET(PWidget(scrollbarv))->requisition.width;
	scrollBarHeight = GTK_WIDGET(PWidget(scrollbarh))->requisition.height;

	// These allocations should never produce negative sizes as they would wrap around to huge
	// unsigned numbers inside GTK+ causing warnings.
	int horizontalScrollBarHeight = scrollBarWidth;
	if (!horizontalScrollBarVisible)
		horizontalScrollBarHeight = 0;

	GtkAllocation alloc;
	alloc.x = 0;
	if (horizontalScrollBarVisible) {
		alloc.y = height - scrollBarHeight;
		alloc.width = Platform::Maximum(1, width - scrollBarWidth) + 1;
		alloc.height = horizontalScrollBarHeight;
	} else {
		alloc.y = height;
		alloc.width = 0;
		alloc.height = 0;
	}
	gtk_widget_size_allocate(GTK_WIDGET(PWidget(scrollbarh)), &alloc);

	alloc.x = width - scrollBarWidth;
	alloc.y = 0;
	alloc.width = scrollBarWidth;
	alloc.height = Platform::Maximum(1, height - scrollBarHeight) + 1;
	gtk_widget_size_allocate(GTK_WIDGET(PWidget(scrollbarv)), &alloc);

	SetScrollBars();
}

gint ScintillaGTK::PressThis(GdkEventButton *event) {
	//Platform::DebugPrintf("Press %x time=%d state = %x button = %x\n",this,event->time, event->state, event->button);
	// Do not use GTK+ double click events as Scintilla has its own double click detection
	if (event->type != GDK_BUTTON_PRESS)
		return FALSE;

	evbtn = *event;
	Point pt;
	pt.x = int(event->x);
	pt.y = int(event->y);
	PRectangle rcClient = GetClientRectangle();
	//Platform::DebugPrintf("Press %0d,%0d in %0d,%0d %0d,%0d\n",
	//	pt.x, pt.y, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
	if ((pt.x > rcClient.right) || (pt.y > rcClient.bottom)) {
		Platform::DebugPrintf("Bad location\n");
		return FALSE;
	}

	bool ctrl = event->state & GDK_CONTROL_MASK;

	gtk_widget_grab_focus(PWidget(wMain));
	if (event->button == 1) {
		//ButtonDown(pt, event->time,
		//	event->state & GDK_SHIFT_MASK,
		//	event->state & GDK_CONTROL_MASK,
		//	event->state & GDK_MOD1_MASK);
		// Instead of sending literal modifiers use control instead of alt
		// This is because all the window managers seem to grab alt + click for moving
		ButtonDown(pt, event->time,
		                    event->state & GDK_SHIFT_MASK,
		                    event->state & GDK_CONTROL_MASK,
		                    event->state & GDK_CONTROL_MASK);
	} else if (event->button == 2) {
		// Grab the primary selection if it exists
		Position pos = PositionFromLocation(pt);
		if (OwnPrimarySelection() && primary.s == NULL)
			CopySelectionRange(&primary);

		SetSelection(pos, pos);
		gtk_selection_convert(GTK_WIDGET(PWidget(wMain)), GDK_SELECTION_PRIMARY,
		                      gdk_atom_intern("STRING", FALSE), event->time);
	} else if (event->button == 3 && displayPopupMenu) {
		// PopUp menu
		// Convert to screen
		int ox = 0;
		int oy = 0;
		gdk_window_get_origin(PWidget(wMain)->window, &ox, &oy);
		ContextMenu(Point(pt.x + ox, pt.y + oy));
	} else if (event->button == 4) {
		// Wheel scrolling up (only xwin gtk does it this way)
		if (ctrl)
			gtk_adjustment_set_value(GTK_ADJUSTMENT(adjustmenth), (
			                             (xOffset) / 2 ) - 6);
		else
			gtk_adjustment_set_value(GTK_ADJUSTMENT(adjustmentv),
			                         topLine - 3);
	} else if ( event->button == 5 ) {
		// Wheel scrolling down (only xwin gtk does it this way)
		if (ctrl)
			gtk_adjustment_set_value(GTK_ADJUSTMENT(adjustmenth), (
			                             (xOffset) / 2 ) + 6);
		else
			gtk_adjustment_set_value(GTK_ADJUSTMENT(adjustmentv),
			                         topLine + 3);
	}
	return FALSE;
}

gint ScintillaGTK::Press(GtkWidget *widget, GdkEventButton *event) {
	if (event->window != widget->window)
		return FALSE;
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	return sciThis->PressThis(event);
}

gint ScintillaGTK::MouseRelease(GtkWidget *widget, GdkEventButton *event) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Release %x %d %d\n",sciThis,event->time,event->state);
	if (!sciThis->HaveMouseCapture())
		return FALSE;
	if (event->button == 1) {
		Point pt;
		pt.x = int(event->x);
		pt.y = int(event->y);
		//Platform::DebugPrintf("Up %x %x %d %d %d\n",
		//	sciThis,event->window,event->time, pt.x, pt.y);
		if (event->window != PWidget(sciThis->wMain)->window)
			// If mouse released on scroll bar then the position is relative to the
			// scrollbar, not the drawing window so just repeat the most recent point.
			pt = sciThis->ptMouseLast;
		sciThis->ButtonUp(pt, event->time, event->state & 4);
	}
	return FALSE;
}

// win32gtk has a special wheel mouse event for whatever reason and doesn't
// use the button4/5 trick used under X windows.
#if PLAT_GTK_WIN32
gint ScintillaGTK::ScrollEvent(GtkWidget *widget,
                               GdkEventScroll *event) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);

	if (widget == NULL || event == NULL)
		return FALSE;

	// Compute amount and direction to scroll (even tho on win32 there is
	// intensity of scrolling info in the native message, gtk doesn't
	// support this so we simulate similarly adaptive scrolling)
	int cLineScroll;
	int timeDelta = 1000000;
	GTimeVal curTime;
	g_get_current_time(&curTime);
	if (curTime.tv_sec == sciThis->lastWheelMouseTime.tv_sec)
		timeDelta = curTime.tv_usec - sciThis->lastWheelMouseTime.tv_usec;
	else if (curTime.tv_sec == sciThis->lastWheelMouseTime.tv_sec + 1) 
		timeDelta = 1000000 + (curTime.tv_usec - sciThis->lastWheelMouseTime.tv_usec);
	if ((event->direction == sciThis->lastWheelMouseDirection) && (timeDelta < 250000)) {
		if (sciThis->wheelMouseIntensity < 12)
			sciThis->wheelMouseIntensity++;
		cLineScroll = sciThis->wheelMouseIntensity;
	} else {
		cLineScroll = sciThis->linesPerScroll;
		if (cLineScroll == 0)
			cLineScroll = 4;
		sciThis->wheelMouseIntensity = cLineScroll;
	}
	if (event->direction == GDK_SCROLL_UP) {
		cLineScroll *= -1;
	}
	g_get_current_time(&sciThis->lastWheelMouseTime);
	sciThis->lastWheelMouseDirection = event->direction;

	// Note:  Unpatched versions of win32gtk don't set the 'state' value so 
	// only regular scrolling is supported there.  Also, unpatched win32gtk
	// issues spurious button 2 mouse events during wheeling, which can cause
	// problems (a patch for both was submitted by archaeopteryx.com on 13Jun2001)

	// Data zoom not supported
	if (event->state & GDK_SHIFT_MASK) {
		return FALSE;
	}

	// Text font size zoom
	if (event->state & GDK_CONTROL_MASK) {
		if (cLineScroll < 0) {
			sciThis->KeyCommand(SCI_ZOOMIN);
			return TRUE;
		} else {
			sciThis->KeyCommand(SCI_ZOOMOUT);
			return TRUE;
		}

	// Regular scrolling
	} else {
		sciThis->ScrollTo(sciThis->topLine + cLineScroll);
		return TRUE;
	}
}
#endif

gint ScintillaGTK::Motion(GtkWidget *widget, GdkEventMotion *event) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Motion %x %d\n",sciThis,event->time);
	if (event->window != widget->window)
		return FALSE;
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
	Point pt(x, y);
	sciThis->ButtonMove(pt);
	return FALSE;
}

// Map the keypad keys to their equivalent functions
static int KeyTranslate(int keyIn) {
	switch (keyIn) {
	case GDK_ISO_Left_Tab:
		return SCK_TAB;
	case GDK_KP_Down:
		return SCK_DOWN;
	case GDK_KP_Up:
		return SCK_UP;
	case GDK_KP_Left:
		return SCK_LEFT;
	case GDK_KP_Right:
		return SCK_RIGHT;
	case GDK_KP_Home:
		return SCK_HOME;
	case GDK_KP_End:
		return SCK_END;
	case GDK_KP_Page_Up:
		return SCK_PRIOR;
	case GDK_KP_Page_Down:
		return SCK_NEXT;
	case GDK_KP_Delete:
		return SCK_DELETE;
	case GDK_KP_Insert:
		return SCK_INSERT;
	case GDK_KP_Enter:
		return SCK_RETURN;

	case GDK_Down:
		return SCK_DOWN;
	case GDK_Up:
		return SCK_UP;
	case GDK_Left:
		return SCK_LEFT;
	case GDK_Right:
		return SCK_RIGHT;
	case GDK_Home:
		return SCK_HOME;
	case GDK_End:
		return SCK_END;
	case GDK_Page_Up:
		return SCK_PRIOR;
	case GDK_Page_Down:
		return SCK_NEXT;
	case GDK_Delete:
		return SCK_DELETE;
	case GDK_Insert:
		return SCK_INSERT;
	case GDK_Escape:
		return SCK_ESCAPE;
	case GDK_BackSpace:
		return SCK_BACK;
	case GDK_Tab:
		return SCK_TAB;
	case GDK_Return:
		return SCK_RETURN;
	case GDK_KP_Add:
		return SCK_ADD;
	case GDK_KP_Subtract:
		return SCK_SUBTRACT;
	case GDK_KP_Divide:
		return SCK_DIVIDE;
	default:
		return keyIn;
	}
}

gint ScintillaGTK::KeyPress(GtkWidget *widget, GdkEventKey *event) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("SC-key: %d %x [%s]\n", 
	//	event->keyval, event->state, (event->length > 0) ? event->string : "empty");
	bool shift = event->state & GDK_SHIFT_MASK;
	bool ctrl = event->state & GDK_CONTROL_MASK;
	bool alt = event->state & GDK_MOD1_MASK;
	int key = event->keyval;
	if (ctrl && (key < 128))
		key = toupper(key);
	else if (!ctrl && (key >= GDK_KP_Multiply && key <= GDK_KP_9))
		key &= 0x7F;
	// Hack for keys over 256 and below command keys but makes Hungarian work.
	// This will have to change for Unicode
	else if ((key >= 0x100) && (key < 0x1000))
		key &= 0xff;
	else
		key = KeyTranslate(key);

	bool consumed = false;
	int added = sciThis->KeyDown(key, shift, ctrl, alt, &consumed);
	if (!consumed)
		consumed = added;
	//Platform::DebugPrintf("SK-key: %d %x %x\n",event->keyval, event->state, consumed);
	return consumed;
}

gint ScintillaGTK::KeyRelease(GtkWidget *, GdkEventKey * /*event*/) {
	//Platform::DebugPrintf("SC-keyrel: %d %x %3s\n",event->keyval, event->state, event->string);
	return FALSE;
}

void ScintillaGTK::Destroy(GtkObject* object) {
	ScintillaGTK *sciThis = ScintillaFromWidget(GTK_WIDGET(object));
	//Platform::DebugPrintf("Destroying %x %x\n", sciThis, object);
	sciThis->Finalise();
      
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);

	delete sciThis;
}

static void DrawChild(GtkWidget *widget, GdkRectangle *area) {
	GdkRectangle areaIntersect;
	if (widget &&
	        GTK_WIDGET_DRAWABLE(widget) &&
	        gtk_widget_intersect(widget, area, &areaIntersect)) {
		gtk_widget_draw(widget, &areaIntersect);
	}
}

void ScintillaGTK::Draw(GtkWidget *widget, GdkRectangle *area) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Draw %p %0d,%0d %0d,%0d\n", widget, area->x, area->y, area->width, area->height);
	PRectangle rcPaint(area->x, area->y, area->x + area->width, area->y + area->height);
	sciThis->SyncPaint(rcPaint);
	if (GTK_WIDGET_DRAWABLE(PWidget(sciThis->wMain))) {
		DrawChild(PWidget(sciThis->scrollbarh), area);
		DrawChild(PWidget(sciThis->scrollbarv), area);
	}

}

gint ScintillaGTK::ExposeMain(GtkWidget *widget, GdkEventExpose *ose) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Expose Main %0d,%0d %0d,%0d\n",
	//ose->area.x, ose->area.y, ose->area.width, ose->area.height);
	return Expose(widget, ose, sciThis);
}

gint ScintillaGTK::Expose(GtkWidget *, GdkEventExpose *ose, ScintillaGTK *sciThis) {
	//Platform::DebugPrintf("Expose %0d,%0d %0d,%0d\n",
	//ose->area.x, ose->area.y, ose->area.width, ose->area.height);

	sciThis->paintState = painting;

	sciThis->rcPaint.left = ose->area.x;
	sciThis->rcPaint.top = ose->area.y;
	sciThis->rcPaint.right = ose->area.x + ose->area.width;
	sciThis->rcPaint.bottom = ose->area.y + ose->area.height;

	PRectangle rcText = sciThis->GetTextRectangle();
	sciThis->paintingAllText = sciThis->rcPaint.Contains(rcText);
	Surface *surfaceWindow = Surface::Allocate();
	if (surfaceWindow) {
		surfaceWindow->Init((PWidget(sciThis->wMain))->window);
		sciThis->Paint(surfaceWindow, sciThis->rcPaint);
		surfaceWindow->Release();
		delete surfaceWindow;
	}
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

void ScintillaGTK::SelectionReceived(GtkWidget *widget,
                                     GtkSelectionData *selection_data, guint) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Selection received\n");
	sciThis->ReceivedSelection(selection_data);
}

void ScintillaGTK::SelectionGet(GtkWidget *widget,
                                GtkSelectionData *selection_data, guint info, guint) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Selection get\n");
	sciThis->GetSelection(selection_data, info, &sciThis->copyText);
}

gint ScintillaGTK::SelectionClear(GtkWidget *widget, GdkEventSelection *selection_event) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Selection clear\n");
	sciThis->UnclaimSelection(selection_event);
	return gtk_selection_clear(widget, selection_event);
}

gint ScintillaGTK::SelectionNotify(GtkWidget *widget, GdkEventSelection *selection_event) {
	//Platform::DebugPrintf("Selection notify\n");
	return gtk_selection_notify(widget, selection_event);
}

void ScintillaGTK::DragBegin(GtkWidget *, GdkDragContext *) {
	//Platform::DebugPrintf("DragBegin\n");
}

gboolean ScintillaGTK::DragMotion(GtkWidget *widget, GdkDragContext *context,
                                  gint x, gint y, guint dragtime) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("DragMotion %d %d %x %x %x\n", x, y,
	//	context->actions, context->suggested_action, sciThis);
	Point npt(x, y);
	sciThis->inDragDrop = true;
	sciThis->SetDragPosition(sciThis->PositionFromLocation(npt));
	gdk_drag_status(context, context->suggested_action, dragtime);
	return FALSE;
}

void ScintillaGTK::DragLeave(GtkWidget *widget, GdkDragContext * /*context*/, guint) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	sciThis->SetDragPosition(invalidPosition);
	//Platform::DebugPrintf("DragLeave %x\n", sciThis);
}

void ScintillaGTK::DragEnd(GtkWidget *widget, GdkDragContext * /*context*/) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	// If drag did not result in drop here or elsewhere
	if (!sciThis->dragWasDropped)
		sciThis->SetEmptySelection(sciThis->posDrag);
	sciThis->SetDragPosition(invalidPosition);
	//Platform::DebugPrintf("DragEnd %x %d\n", sciThis, sciThis->dragWasDropped);
}

gboolean ScintillaGTK::Drop(GtkWidget *widget, GdkDragContext * /*context*/,
                            gint, gint, guint) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	//Platform::DebugPrintf("Drop %x\n", sciThis);
	sciThis->SetDragPosition(invalidPosition);
	return FALSE;
}

void ScintillaGTK::DragDataReceived(GtkWidget *widget, GdkDragContext * /*context*/,
                                    gint, gint, GtkSelectionData *selection_data, guint /*info*/, guint) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	sciThis->ReceivedDrop(selection_data);
	sciThis->SetDragPosition(invalidPosition);
}

void ScintillaGTK::DragDataGet(GtkWidget *widget, GdkDragContext *context,
                               GtkSelectionData *selection_data, guint info, guint) {
	ScintillaGTK *sciThis = ScintillaFromWidget(widget);
	sciThis->dragWasDropped = true;
	if (sciThis->currentPos != sciThis->anchor) {
		sciThis->GetSelection(selection_data, info, &sciThis->drag);
	}
	if (context->action == GDK_ACTION_MOVE) {
		int selStart = sciThis->SelectionStart();
		int selEnd = sciThis->SelectionEnd();
		if (sciThis->posDrop > selStart) {
			if (sciThis->posDrop > selEnd)
				sciThis->posDrop = sciThis->posDrop - (selEnd - selStart);
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
	Surface *surfaceWindow = Surface::Allocate();
	if (surfaceWindow) {
		surfaceWindow->Init(widget->window);
		ctip->PaintCT(surfaceWindow);
		surfaceWindow->Release();
		delete surfaceWindow;
	}
	return TRUE;
}

sptr_t ScintillaGTK::DirectFunction(
    ScintillaGTK *sciThis, unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	return sciThis->WndProc(iMessage, wParam, lParam);
}

sptr_t scintilla_send_message(ScintillaObject *sci, unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	ScintillaGTK *psci = reinterpret_cast<ScintillaGTK *>(sci->pscin);
	return psci->WndProc(iMessage, wParam, lParam);
}

static void scintilla_class_init (ScintillaClass *klass);
static void scintilla_init (ScintillaObject *sci);

guint scintilla_get_type() {
	static guint scintilla_type = 0;

	if (!scintilla_type) {
		GtkTypeInfo scintilla_info = {
		    "Scintilla",
		    sizeof (ScintillaObject),
		    sizeof (ScintillaClass),
		    (GtkClassInitFunc) scintilla_class_init,
		    (GtkObjectInitFunc) scintilla_init,
		    (gpointer) NULL,
		    (gpointer) NULL,
		    0
		};

		scintilla_type = gtk_type_unique(gtk_container_get_type(), &scintilla_info);
	}

	return scintilla_type;
}

void ScintillaGTK::ClassInit(GtkObjectClass* object_class, GtkWidgetClass *widget_class) {
	// Define default signal handlers for the class:  Could move more
	// of the signal handlers here (those that currently attached to wDraw
	// in Initialise() may require coordinate translation?)

	object_class->destroy = Destroy;  

	widget_class->size_request = SizeRequest;
	widget_class->size_allocate = SizeAllocate;
	widget_class->expose_event = ExposeMain;
	widget_class->draw = Draw;

	widget_class->motion_notify_event = Motion;
	widget_class->button_press_event = Press;
	widget_class->button_release_event = MouseRelease;
#if PLAT_GTK_WIN32
	widget_class->scroll_event = ScrollEvent;
#endif
	widget_class->key_press_event = KeyPress;
	widget_class->key_release_event = KeyRelease;
	widget_class->focus_in_event = FocusIn;
	widget_class->focus_out_event = FocusOut;
	widget_class->selection_received = SelectionReceived;
	widget_class->selection_get = SelectionGet;
	widget_class->selection_clear_event = SelectionClear;
	widget_class->selection_notify_event = SelectionNotify;

	widget_class->drag_data_received = DragDataReceived;
	widget_class->drag_motion = DragMotion;
	widget_class->drag_leave = DragLeave;
	widget_class->drag_end = DragEnd;
	widget_class->drag_drop = Drop;
	widget_class->drag_data_get = DragDataGet;

	widget_class->realize = Realize;
	widget_class->unrealize = UnRealize;
	widget_class->map = Map;
	widget_class->unmap = UnMap;
}

static void scintilla_class_init(ScintillaClass *klass) {
	GtkObjectClass *object_class = (GtkObjectClass*) klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass*) klass;

	parent_class = (GtkWidgetClass*) gtk_type_class (gtk_container_get_type ());

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

	ScintillaGTK::ClassInit(object_class, widget_class);
}

static void scintilla_init(ScintillaObject *sci) {
	GTK_WIDGET_SET_FLAGS(sci, GTK_CAN_FOCUS);
	sci->pscin = new ScintillaGTK(sci);
}

GtkWidget* scintilla_new() {
	return GTK_WIDGET(gtk_type_new(scintilla_get_type()));
}

void scintilla_set_id(ScintillaObject *sci, int id) {
	ScintillaGTK *psci = reinterpret_cast<ScintillaGTK *>(sci->pscin);
	psci->ctrlID = id;
}
