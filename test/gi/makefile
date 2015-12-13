all: Scintilla-0.1.typelib

ifdef GTK3
GTKVERSION=3.0
else
GTKVERSION=2.0
endif

GI_SCANNER = g-ir-scanner
GI_COMPILER = g-ir-compiler
GTK_LIBS = $(shell pkg-config --libs gtk+-$(GTKVERSION))
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-$(GTKVERSION))
PWD = $(shell pwd)

FORCE:

../../bin/scintilla.a: FORCE
	$(MAKE) -C ../../gtk all

libscintilla.so: ../../bin/scintilla.a
	$(CXX) -shared -o $@ -Wl,--whole-archive $^ -Wl,--no-whole-archive $(GTK_LIBS)

Scintilla-0.1.gir: libscintilla.so
	LDFLAGS=-Wl,-rpath=$(shell pwd) \
		$(GI_SCANNER) --warn-all -i Gtk-$(GTKVERSION) -DG_IR_SCANNING -DGTK \
		--cflags-begin $(GTK_CFLAGS) -include gtk/gtk.h --cflags-end \
		--c-include Scintilla.h --c-include ScintillaWidget.h \
		-n Scintilla --nsversion 0.1 --library scintilla -L$(PWD) ../../include/ScintillaWidget.h \
		-o $@

Scintilla-0.1.typelib: Scintilla-0.1.gir
	$(GI_COMPILER) $^ -o $@

clean:
	rm -f libscintilla.so Scintilla-0.1.gir Scintilla-0.1.typelib
	$(MAKE) -C ../../gtk clean

test: Scintilla-0.1.gir
	@echo Verifing Scintilla-0.1.gir file
	@diff $^.good $^ || (echo "GIR FILE MISMATCH!"; exit 1)
	@echo Launching gi-test.py python program
	GI_TYPELIB_PATH=$(PWD) LD_LIBRARY_PATH=$(PWD) \
		python $(PWD)/gi-test.py
