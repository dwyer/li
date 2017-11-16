LIBS=socket.so
CFLAGS=-I../src
LDFLAGS=-L../ -lli

all: $(LIBS)

%.so: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC -o $@ $<

clean:
	$(RM) *.so
