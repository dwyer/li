AR=ar rcu
CC=cc
LEX=lex
MV=mv
RANLIB=ranlib
RM=rm -f
YACC=yacc

CFLAGS=-O2 -Wall -ansi -pedantic
LDFLAGS=
LIBS=-lm

LI_BIN=li
LI_OBJS=li.o

LI_LIB=libli.a
LI_LIB_OBJS=li_read.o li_parse.o li_error.o li_eval.o li_object.o li_proc.o

ALL_OBJS=$(LI_LIB_OBJS) $(LI_OBJS)

all: $(LI_BIN)

$(LI_BIN): $(LI_LIB) $(LI_OBJS)
	$(CC) -o $@ $(LDFLAGS) $(LI_OBJS) $(LI_LIB) $(LIBS)

$(LI_LIB): $(LI_LIB_OBJS)
	$(AR) $@ $(LI_LIB_OBJS)
	$(RANLIB) $@

li_parse.o: li_parse.c li_read.y

li_parse.c: li_parse.l
	$(LEX) -o $@ li_parse.l

li_read.c: li_read.y
	$(YACC) -d li_read.y
	$(MV) y.tab.c li_read.c
	$(MV) y.tab.h li_read.h

$(ALL_OBJS): li.h

clean:
	$(RM) $(LI_BIN) $(LI_LIB) $(ALL_OBJS) li_parse.c li_read.c li_read.h
