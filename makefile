CC = gcc
AR = ar
COPT = -g -s -c -O2 -Wall -I../smapi -DUNIX
VER = 0.3
LIBDIR = /usr/local/lib
INSTDIR = /usr/local/bin

OBJS    = patmat.o line.o fidoconfig.o fconf2msged.o fconf2golded.o tparser.o

ALL: $(OBJS) \
     fidoconfig.a \
     libfidoconfig.so.$(VER) \
     tparser \
     fconf2msged \
     fconf2golded \
     install

fidoconfig.a: fidoconfig.o line.o common.o patmat.o
	$(AR) r fidoconfig.a fidoconfig.o line.o common.o patmat.o

libfidoconfig.so.$(VER): fidoconfig.o line.o common.o patmat.o
	$(CC) -shared -Wl,-soname,libfidoconfig.so.0 -o libfidoconfig.so.$(VER) line.o common.o fidoconfig.o patmat.o

%.o: %.c
	$(CC) $(COPT) $*.c

fconf2msged: fconf2msged.o fidoconfig.a
	$(CC) fconf2msged.o -o fconf2msged -lfidoconfig

fconf2golded: fconf2golded.o fidoconfig.a
	$(CC) fconf2golded.o -o fconf2golded -lfidoconfig

tparser: tparser.o fidoconfig.a
	$(CC) tparser.o -o tparser -lfidoconfig

clean:
	rm -f *.o

install: 
	cp -f libfidoconfig.so.$(VER) $(LIBDIR)
	ln -sf $(LIBDIR)/libfidoconfig.so.$(VER) $(LIBDIR)/libfidoconfig.so.0
	ln -sf $(LIBDIR)/libfidoconfig.so.0 $(LIBDIR)/libfidoconfig.so
	install -s fconf2golded $(INSTDIR)
	install -s fconf2msged $(INSTDIR)
	install -s tparser $(INSTDIR)
