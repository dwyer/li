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
LI_LIB_OBJS_=read.o parse.o write.o object.o eval.o proc.o error.o
LI_LIB_OBJS=$(addprefix $(OBJDIR)/, $(LI_LIB_OBJS_))
ALL_OBJS=$(LI_OBJS) $(LI_LIB_OBJS)

all: $(LI_BIN)

$(SRCDIR)/read.c: $(SRCDIR)/read.y
	yacc -d $(SRCDIR)/read.y
	mv y.tab.c $(SRCDIR)/read.c
	mv y.tab.h $(SRCDIR)/read.h

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
	$(RM) $(LI_BIN) $(LI_LIB) $(ALL_OBJS) src/parse.c src/read.[ch]
	$(RMDIR) $(OBJDIR)
