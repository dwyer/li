LIBS=\
     cxr.so \
     process-context.so \
     misc.so \
     socket.so \
     time.so

INCLUDE=../src

CFLAGS=-I$(INCLUDE)
LDFLAGS=-L../ -lli

all: $(LIBS)

%.so: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC -o $@ $<

clean:
	$(RM) *.so

cxr.so: cxr.c $(INCLUDE)/li.h $(INCLUDE)/li_lib.h
process-context.so: process-context.c $(INCLUDE)/li.h $(INCLUDE)/li_lib.h
misc.so: misc.c $(INCLUDE)/li.h $(INCLUDE)/li_lib.h
socket.so: socket.c $(INCLUDE)/li.h $(INCLUDE)/li_lib.h
time.so: time.c $(INCLUDE)/li.h $(INCLUDE)/li_lib.h
