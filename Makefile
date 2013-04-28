CC=cc
CP=cp -r
MKDIR=mkdir -p
RM=rm -f
RMDIR=rmdir

CFILES=main.c read.c write.c object.c eval.c proc.c
CFLAGS=-c -ansi -Wall -I./include
LDFLAGS=-lm
OBJDIR=obj
SRCDIR=src
PROGRAM=subscm
PREFIX=/usr

OBJS=$(addprefix $(OBJDIR)/, $(CFILES:.c=.o))
SRCS=$(addprefix $(SRCDIR)/, $(CFILES))

all: $(OBJDIR) $(SRCS) $(PROGRAM)

debug: CC+=-g -DDEBUG
debug: all

profile: CC+=-pg
profile: all

$(PROGRAM): $(OBJS) $(BINDIR)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

install: all
	$(CP) $(PROGRAM) $(PREFIX)/bin/$(PROGRAM)
	$(MKDIR) $(PREFIX)/lib/$(PROGRAM)
	$(CP) lib/* $(PREFIX)/lib/$(PROGRAM)

uninstall:
	$(RM) $(PREFIX)/bin/$(PROGRAM)
	$(RM) $(PREFIX)/lib/$(PROGRAM)/*
	$(RMDIR) $(PREFIX)/lib/$(PROGRAM)

clean:
	$(RM) $(PROGRAM) $(OBJS)
	$(RMDIR) $(OBJDIR)
