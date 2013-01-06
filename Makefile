CC=cc
RM=rm
CFILES=main.c read.c write.c object.c eval.c proc.c
CFLAGS=-c -ansi -Wall -I./include
LDFLAGS=-lm
OBJDIR=obj
SRCDIR=src
EXECUTABLE=subscm

OBJS=$(addprefix $(OBJDIR)/, $(CFILES:.c=.o))
SRCS=$(addprefix $(SRCDIR)/, $(CFILES))

all: $(OBJDIR) $(SRCS) $(EXECUTABLE)

debug: CC+=-g -DDEBUG
debug: all

profile: CC+=-pg
profile: all

$(EXECUTABLE): $(OBJS) $(BINDIR)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	$(RM) -rf $(EXECUTABLE) $(OBJDIR)
