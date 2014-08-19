AR=ar rcu
CC=cc
CP=cp -r
LEX=lex
MKDIR=mkdir -p
MV=mv
RANLIB=ranlib
RM=rm -f
RMDIR=rmdir
YACC=yacc -d

CFLAGS=-O2 -Wall -ansi -pedantic
LDFLAGS=-lm

PREFIX=/usr/local
TO_BIN=$(PREFIX)/bin

OBJDIR=obj
SRCDIR=src
LI_BIN=li
LI_LIB=libli.a
LI_OBJS_=li.o
LI_OBJS=$(addprefix $(OBJDIR)/, $(LI_OBJS_))
LI_LIB_OBJS_=li_read.o li_parse.o li_write.o li_object.o li_eval.o li_proc.o \
	     li_error.o
LI_LIB_OBJS=$(addprefix $(OBJDIR)/, $(LI_LIB_OBJS_))
ALL_OBJS=$(LI_OBJS) $(LI_LIB_OBJS)

all: $(LI_BIN)

$(SRCDIR)/li_read.c: $(SRCDIR)/li_read.y
	yacc -d $(SRCDIR)/li_read.y
	mv y.tab.c $(SRCDIR)/li_read.c
	mv y.tab.h $(SRCDIR)/li_read.h

debug: CFLAGS+=-g -DDEBUG
debug: all

profile: CFLAGS+=-pg
profile: all

$(LI_BIN): $(LI_LIB) $(LI_OBJS)
	$(CC) $(LDFLAGS) -o $@ $+

$(LI_LIB): $(LI_LIB_OBJS)
	$(AR) $@ $(LI_LIB_OBJS)
	$(RANLIB) $@

$(ALL_OBJS): $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

install: all
	$(CP) $(LI_BIN) $(TO_BIN)

uninstall:
	cd $(TO_BIN) && $(RM) $(LI_BIN)

clean:
	$(RM) $(LI_BIN) $(LI_LIB) $(ALL_OBJS) src/li_parse.c src/li_read.[ch]
	$(RMDIR) $(OBJDIR)
