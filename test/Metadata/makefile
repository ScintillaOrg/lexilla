.PHONY: all check clean

INCLUDES = -I ../../include -I ../../access -I ../../../scintilla/include
EXE = $(if $(windir),Metadata.exe,Metadata)

ifdef windir
	RM = $(if $(wildcard $(dir $(SHELL))rm.exe), $(dir $(SHELL))rm.exe -f, del /q)
	CXX = g++
else
	LIBS += -ldl
endif

all: $(EXE)

check: $(EXE)
	./$(EXE)

clean:
	$(RM) $(EXE) *.o

vpath %.cxx ../../access

%.o: %.cxx
	$(CXX) $(INCLUDES) -c $< -o $@

$(EXE): *.cxx LexillaAccess.o
	$(CXX) $(INCLUDES) $^ $(LIBS) -o $@
