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
LI_LIB_OBJS_=li_read.o li_parse.o li_chr.o li_error.o li_nat.o li_object.o \
	     li_obj_bytevector.o li_obj_chr.o li_obj_env.o li_obj_mac.o \
	     li_obj_num.o li_obj_pair.o li_obj_port.o li_obj_proc.o \
	     li_obj_spf.o li_obj_str.o li_obj_sym.o li_obj_typ.o li_obj_vec.o \
	     li_proc.o li_rat.o li_sock.o
LI_LIB_OBJS=$(addprefix $(OBJDIR)/, $(LI_LIB_OBJS_))
LI_OPT_OBJS_=
LI_OPT_OBJS=$(addprefix $(OBJDIR)/, $(LI_OPT_OBJS_))
ALL_OBJS=$(LI_OBJS) $(LI_LIB_OBJS)

.PHONY: all opt debug profile install uninstall clean test tags

all: $(LI_BIN)

$(LI_BIN): $(LI_OBJS) $(LI_LIB)
	$(CC) -o $@ $+ $(LDFLAGS)

$(LI_LIB): $(LI_LIB_OBJS)
	$(CC) -o $@ $+ $(LDFLAGS) -shared -fPIC

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(MKDIR) $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(SRCDIR)/li_read.c: $(SRCDIR)/li_read.y
	yacc -d $(SRCDIR)/li_read.y
	mv y.tab.c $(SRCDIR)/li_read.c
	mv y.tab.h $(SRCDIR)/li_read.h

opt: $(LI_OPT_OBJS)
opt: LI_LIB_OBJS+=$(LI_OPT_OBJS)
opt: CFLAGS+=-DLI_OPTIONAL
opt: all

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

# automatically made with 'gcc -MM src/li_*.c'
$(OBJDIR)/li_chr.o: src/li_chr.c src/li.h
$(OBJDIR)/li_error.o: src/li_error.c src/li.h
$(OBJDIR)/li_nat.o: src/li_nat.c src/li.h
$(OBJDIR)/li_obj_chr.o: src/li_obj_chr.c src/li.h
$(OBJDIR)/li_obj_env.o: src/li_obj_env.c src/li.h
$(OBJDIR)/li_obj_mac.o: src/li_obj_mac.c src/li.h
$(OBJDIR)/li_obj_num.o: src/li_obj_num.c src/li.h
$(OBJDIR)/li_obj_pair.o: src/li_obj_pair.c src/li.h
$(OBJDIR)/li_obj_port.o: src/li_obj_port.c src/li.h
$(OBJDIR)/li_obj_proc.o: src/li_obj_proc.c src/li.h
$(OBJDIR)/li_obj_spf.o: src/li_obj_spf.c src/li.h
$(OBJDIR)/li_obj_str.o: src/li_obj_str.c src/li.h
$(OBJDIR)/li_obj_sym.o: src/li_obj_sym.c src/li.h
$(OBJDIR)/li_obj_typ.o: src/li_obj_typ.c src/li.h
$(OBJDIR)/li_obj_vec.o: src/li_obj_vec.c src/li.h
$(OBJDIR)/li_object.o: src/li_object.c src/li.h
$(OBJDIR)/li_proc.o: src/li_proc.c src/li.h
$(OBJDIR)/li_rat.o: src/li_rat.c src/li.h
$(OBJDIR)/li_read.o: src/li_read.c src/li.h
$(OBJDIR)/li_sock.o: src/li_sock.c src/li.h
$(OBJDIR)/li_str.o: src/li_str.c src/li.h
