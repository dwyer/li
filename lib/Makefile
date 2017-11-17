LIBS=\
     li/misc.so \
     li/socket.so \
     scheme/cxr.so \
     scheme/process-context.so \
     scheme/time.so

INCLUDE=../src

CFLAGS=-I$(INCLUDE)
LDFLAGS=-L../ -lli

all: $(LIBS)

%.so: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC -o $@ $<

clean:
	$(RM) *.so

li/misc.so: li/misc.c $(INCLUDE)/li.h $(INCLUDE)/li_lib.h
li/socket.so: li/socket.c $(INCLUDE)/li.h $(INCLUDE)/li_lib.h
scheme/cxr.so: scheme/cxr.c $(INCLUDE)/li.h $(INCLUDE)/li_lib.h
scheme/process-context.so: scheme/process-context.c $(INCLUDE)/li.h $(INCLUDE)/li_lib.h
scheme/time.so: scheme/time.c $(INCLUDE)/li.h $(INCLUDE)/li_lib.h
