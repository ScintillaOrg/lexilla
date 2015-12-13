#!/usr/bin/python
import gi
gi.require_version('Scintilla', '0.1')

# Scintilla is imported before because it loads Gtk with a specified version
# this avoids a warning when Gtk is imported without version such as below (where
# it is imported without because this script works with gtk2 and gtk3)
from gi.repository import Scintilla
from gi.repository import Gtk

win = Gtk.Window()
win.connect("delete-event", Gtk.main_quit)
sci = Scintilla.Object()
win.add(sci)
win.show_all()
win.resize(400,300)
Gtk.main()
