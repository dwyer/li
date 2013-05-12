CP=cp -r
MKDIR=mkdir -p
RM=rm -f
RMDIR=rmdir

CFILES=main.c read.c write.c object.c eval.c proc.c
CFLAGS=-I./$(INCDIR) -O2 -Wall -ansi -pedantic
LDFLAGS=-lm

INCDIR=include
OBJDIR=obj
SRCDIR=src

PROG=subscm

PREFIX=/usr/local
TO_BIN=$(PREFIX)/bin

OBJS=$(addprefix $(OBJDIR)/, $(CFILES:.c=.o))
SRCS=$(addprefix $(SRCDIR)/, $(CFILES))

all: $(OBJDIR) $(SRCS) $(PROG)

debug: CC+=-g -DDEBUG
debug: all

profile: CC+=-pg
profile: all

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $+

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

install: all
	$(CP) $(PROG) $(TO_BIN)

uninstall:
	cd $(TO_BIN) && $(RM) $(PROG)

clean:
	$(RM) $(PROG) $(OBJS)
	$(RMDIR) $(OBJDIR)
