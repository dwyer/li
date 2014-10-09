AR=ar rcu
CC=cc
CP=cp -r
INSTALL=install
LEX=lex
MKDIR=mkdir -p
MV=mv
RANLIB=ranlib
RM=rm -f
YACC=yacc -d

CFLAGS=-O2 -Wall -Wextra -ansi -pedantic
LDFLAGS=-lm

PREFIX=/usr/local
TO_BIN=$(PREFIX)/bin
TO_LIB=$(PREFIX)/lib
TO_INCLUDE=$(PREFIX)/include

OBJDIR=obj
SRCDIR=src
LI_BIN=li
LI_LIB=libli.a
LI_OBJS_=li.o
LI_OBJS=$(addprefix $(OBJDIR)/, $(LI_OBJS_))
LI_LIB_OBJS_=li_read.o li_parse.o li_error.o li_eval.o li_object.o li_proc.o \
	    li_write.o lib_bytevector.o
LI_LIB_OBJS=$(addprefix $(OBJDIR)/, $(LI_LIB_OBJS_))
ALL_OBJS=$(LI_OBJS) $(LI_LIB_OBJS)

all: $(LI_BIN)

$(LI_BIN): $(LI_OBJS) $(LI_LIB)
	$(CC) -o $@ $+ $(LDFLAGS)

$(LI_LIB): $(LI_LIB_OBJS)
	$(AR) $@ $(LI_LIB_OBJS)
	$(RANLIB) $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(MKDIR) $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(SRCDIR)/li_read.c: $(SRCDIR)/li_read.y
	yacc -d $(SRCDIR)/li_read.y
	mv y.tab.c $(SRCDIR)/li_read.c
	mv y.tab.h $(SRCDIR)/li_read.h

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
	$(RM) $(LI_BIN) $(LI_LIB) src/li_parse.c src/li_read.[ch]
	$(RM) -r $(OBJDIR)

test: $(LI_BIN)
	./$(LI_BIN) test/test.li

tags: src/li.h
	ctags -f $@ $<
