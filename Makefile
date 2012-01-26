CC=gcc
RM=rm
CFILES=eval.c main.c object.c parse.c display.c proc.c
CFLAGS=-c -Wall -Wextra -I./include
LDFLAGS=-lm
OBJDIR=obj
SRCDIR=src
EXECUTABLE=scm

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
