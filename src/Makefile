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

LI_EXEC=li
LI_OBJS=main.o

LI_LIB=libli.a
LI_LIB_OBJS=read.o parse.o error.o eval.o object.o proc.o write.o

ALL_OBJS=$(LI_LIB_OBJS) $(LI_OBJS)

all: $(LI_EXEC)

$(LI_EXEC): $(LI_LIB) $(LI_OBJS)
	$(CC) -o $@ $(LDFLAGS) $(LI_OBJS) $(LI_LIB) $(LIBS)

$(LI_LIB): $(LI_LIB_OBJS)
	$(AR) $@ $(LI_LIB_OBJS)
	$(RANLIB) $@

parse.o: parse.c read.y

parse.c: parse.l
	$(LEX) -o $@ parse.l

read.c: read.y
	$(YACC) -d read.y
	$(MV) y.tab.c read.c
	$(MV) y.tab.h read.h

$(ALL_OBJS): subscm.h

clean:
	$(RM) $(LI_EXEC) $(LI_LIB) $(ALL_OBJS) parse.c read.c read.h
