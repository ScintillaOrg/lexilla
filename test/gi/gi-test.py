#!/usr/bin/python
from gi.repository import Gtk
from gi.repository import Scintilla

win = Gtk.Window()
win.connect("delete-event", Gtk.main_quit)
sci = Scintilla.Object()
win.add(sci)
win.show_all()
win.resize(400,300)
Gtk.main()
