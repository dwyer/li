CC=cc
CP=cp -r
INSTALL=install
LEX=lex
MKDIR=mkdir -p
MV=mv
RM=rm -f
YACC=yacc -d

CFLAGS=-Wall -Wextra -ansi -pedantic
CFLAGS+=-Wno-c99-extensions # TODO: remove this
LDFLAGS=-lm

PREFIX=/usr/local
TO_BIN=$(PREFIX)/bin
TO_LIB=$(PREFIX)/lib
TO_INCLUDE=$(PREFIX)/include

OBJDIR=obj
SRCDIR=src
LI_BIN=li
LI_LIB=libli.so
LI_OBJS_=li.o
LI_OBJS=$(addprefix $(OBJDIR)/, $(LI_OBJS_))
LI_LIB_OBJS_=read.o \
	     lexer.o \
	     base.o \
	     boolean.o \
	     bytevector.o \
	     char.o \
	     environment.o \
	     error.o \
	     import.o \
	     macro.o \
	     nat.o \
	     number.o \
	     object.o \
	     pair.o \
	     port.o \
	     procedure.o \
	     rat.o \
	     string.o \
	     symbol.o \
	     syntax.o \
	     type.o \
	     utf8.o \
	     vector.o \

LI_LIB_OBJS=$(addprefix $(OBJDIR)/, $(LI_LIB_OBJS_))
ALL_OBJS=$(LI_OBJS) $(LI_LIB_OBJS)

.PHONY: all opt debug profile install uninstall clean test tags

all: $(LI_BIN) libs

libs:
	$(MAKE) -C lib

$(LI_BIN): $(LI_OBJS) $(LI_LIB)
	$(CC) -o $@ $+ $(LDFLAGS)

$(LI_LIB): $(LI_LIB_OBJS)
	$(CC) -o $@ $+ $(LDFLAGS) -shared -fPIC

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(MKDIR) $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(SRCDIR)/read.c: $(SRCDIR)/read.y
	yacc -d $(SRCDIR)/read.y
	mv y.tab.c $(SRCDIR)/read.c
	mv y.tab.h $(SRCDIR)/read.h

debug: CFLAGS+=-g -DDEBUG
debug: all

profile: CFLAGS+=-pg
profile: all

install: all
	$(INSTALL) $(LI_BIN) $(TO_BIN)
	$(INSTALL) $(LI_LIB) $(TO_LIB)
	$(INSTALL) src/li.h $(TO_INCLUDE)

uninstall:
	cd $(TO_BIN) && $(RM) $(LI_BIN)

clean:
	$(RM) $(LI_BIN) $(LI_LIB) src/lexer.c src/read.[ch]
	$(RM) -r $(OBJDIR)
	$(MAKE) -C lib clean

test: $(LI_BIN) libs
	LD_LIBRARY_PATH=$(PWD)/lib ./$(LI_BIN) test/test.li

tags: src/li.h
	ctags -f $@ $<

# automatically made with: gcc -MM src/*.c | awk '{ print "$(OBJDIR)/" $0 }'
$(OBJDIR)/base.o: src/base.c src/li.h src/li_lib.h src/li_num.h
$(OBJDIR)/boolean.o: src/boolean.c src/li.h src/li_lib.h
$(OBJDIR)/bytevector.o: src/bytevector.c src/li.h
$(OBJDIR)/char.o: src/char.c src/li.h src/li_lib.h
$(OBJDIR)/environment.o: src/environment.c src/li.h
$(OBJDIR)/error.o: src/error.c src/li.h
$(OBJDIR)/import.o: src/import.c src/li.h
$(OBJDIR)/li.o: src/li.c src/li.h
$(OBJDIR)/macro.o: src/macro.c src/li.h
$(OBJDIR)/nat.o: src/nat.c src/li.h src/li_num.h
$(OBJDIR)/number.o: src/number.c src/li.h src/li_lib.h src/li_num.h
$(OBJDIR)/object.o: src/object.c src/li.h
$(OBJDIR)/pair.o: src/pair.c src/li.h src/li_lib.h
$(OBJDIR)/port.o: src/port.c src/li.h src/li_lib.h
$(OBJDIR)/procedure.o: src/procedure.c src/li.h src/li_lib.h
$(OBJDIR)/rat.o: src/rat.c src/li.h src/li_num.h
$(OBJDIR)/read.o: src/read.c src/li.h src/li_num.h
$(OBJDIR)/string.o: src/string.c src/li.h src/li_lib.h
$(OBJDIR)/symbol.o: src/symbol.c src/li.h src/li_lib.h
$(OBJDIR)/syntax.o: src/syntax.c src/li.h
$(OBJDIR)/type.o: src/type.c src/li.h
$(OBJDIR)/utf8.o: src/utf8.c src/li.h
$(OBJDIR)/vector.o: src/vector.c src/li.h src/li_lib.h
# end
