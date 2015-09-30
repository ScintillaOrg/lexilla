all: Scintilla-0.1.typelib

ifdef GTK3
GTKVERSION=gtk+-3.0
else
GTKVERSION=gtk+-2.0
endif

GI_SCANNER = g-ir-scanner
GI_COMPILER = g-ir-compiler
GTK_LIBS = $(shell pkg-config --libs $(GTKVERSION))

FORCE:

../../bin/scintilla.a: FORCE
	$(MAKE) -C ../../gtk all

libscintilla.so: ../../bin/scintilla.a
	$(CXX) -shared -o $@ -Wl,--whole-archive $^ -Wl,--no-whole-archive $(GTK_LIBS)

Scintilla-0.1.gir: libscintilla.so
	LDFLAGS=-Wl,-rpath=$(shell pwd) CFLAGS="-include gtk/gtk.h" \
		$(GI_SCANNER) --warn-all -i Gtk-3.0 -DG_IR_SCANNING -DGTK \
		--c-include Scintilla.h --c-include ScintillaWidget.h \
		-n Scintilla --nsversion 0.1 --library scintilla ../../include/ScintillaWidget.h \
		-o $@
	@echo Verifing Scintilla-0.1.gir file
	@diff $@.good $@ || (echo "GIR FILE MISMATCH!"; exit 1)


Scintilla-0.1.typelib: Scintilla-0.1.gir
	$(GI_COMPILER) $^ -o $@

clean:
	rm -f libscintilla.so Scintilla-0.1.gir Scintilla-0.1.typelib
