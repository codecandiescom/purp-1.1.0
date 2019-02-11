# Generated automatically from Makefile.in by configure.

.EXPORT_ALL_VARIABLES:

YY           = yacc
LL	     = flex

INCPATHS     = -I/usr/include -I/usr/include/ncurses -I/usr/include/rpm
INCLUDES     =

LIBPATHS     = -L/lib -L/usr/local/lib -L/usr/lib
LIBS         = -lfl #-lefence

DEBUG        = #-DDEBUG_TO_TERM -ggdb #-DDEBUG_SEE_KEY_EVENTS
DEFINES      = ${DEBUG} -DYY_NEVER_INTERACTIVE

CFLAGS       = $(O) $(DEFINES) $(INCPATHS) $(INCLUDES)
LFLAGS       = $(LIBPATHS) $(LIBS)


## Implicit rules #########################################

.SUFFIXES: .cpp .c

.cpp.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.c.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATHS) -o $@ $<

## Objects ################################################

OBJECTS = y.tab.o \
	  lex.yy.o \
          confclass.o 



## Build rules ############################################

all: $(OBJECTS)

y.tab.c:
	rm -f y.tab.c y.tab.h
	$(YY) -d conf.y

lex.yy.c:
	rm -f lex.yy.c 
	$(LL) -L conf.l

clean:
	rm -f *.o *.bak *BAK *~ *% #* 
	rm -f y.tab* lex.yy*

count:
	@echo -n "        Header files (lines): "; cat *.h | wc -l;
	@echo -n "Implementation files (lines): "; cat *.c* | wc -l
