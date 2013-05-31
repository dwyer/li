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

OBJDIR=obj
SRCDIR=src

LI_BIN=li
LI_OBJS=li.o

LI_LIB=libli.a
LI_LIB_OBJS=read.o parse.o write.o object.o eval.o proc.o error.o

PREFIX=/usr/local
TO_BIN=$(PREFIX)/bin

DEPS=read.o parse.o write.o object.o eval.o proc.o error.o li.o
SRCS=$(addprefix $(SRCDIR)/, $(DEPS:.o=.c))
OBJS=$(addprefix $(OBJDIR)/, $(DEPS))

all: $(LI_BIN)

$(SRCDIR)/read.c: $(SRCDIR)/read.y
	yacc -d $(SRCDIR)/read.y
	mv y.tab.c $(SRCDIR)/read.c
	mv y.tab.h $(SRCDIR)/read.h

debug: CFLAGS+=-g -DDEBUG
debug: all

profile: CFLAGS+=-pg
profile: all

$(LI_BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $+

$(LI_LIB): $(LI_LIB_OBJS)
	$(AR) $@ $(LI_LIB_OBJS)
	$(RANLIB) $@

$(OBJS): $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

install: all
	$(CP) $(LI_BIN) $(TO_BIN)

uninstall:
	cd $(TO_BIN) && $(RM) $(LI_BIN)

clean:
	$(RM) $(LI_BIN) $(OBJS) src/parse.c src/read.[ch]
	$(RMDIR) $(OBJDIR)
