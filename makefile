CC = gcc
AR = ar
COPT = -ggdb -s -c -fPIC -O3 -Wall -I../smapi -DUNIX
#LOPT = -L. -L../smapi
LOPT =
VER = 0.5
LIBDIR = /usr/local/lib
INSTDIR = /usr/local

OBJS    = patmat.o line.o fidoconfig.o fconf2msged.o fconf2golded.o tparser.o dir.o common.o

ALL: $(OBJS) \
     libfidoconfig.a \
     libfidoconfig.so.$(VER) \
     tparser \
     fconf2msged \
     fconf2aquaed \
     fconf2golded 

libfidoconfig.a: fidoconfig.o line.o common.o patmat.o dir.o
	$(AR) r libfidoconfig.a fidoconfig.o line.o common.o patmat.o dir.o

libfidoconfig.so.$(VER): fidoconfig.o line.o common.o patmat.o dir.o
	$(CC) -shared -Wl,-soname,libfidoconfig.so.0 -o libfidoconfig.so.$(VER) line.o common.o fidoconfig.o patmat.o dir.o

%.o: %.c
	$(CC) $(COPT) $*.c

fconf2msged: fconf2msged.o libfidoconfig.a
	$(CC) $(LOPT) fconf2msged.o -o fconf2msged -lfidoconfig -lsmapilnx

fconf2golded: fconf2golded.o libfidoconfig.a
	$(CC) $(LOPT) fconf2golded.o -o fconf2golded -lfidoconfig -lsmapilnx

fconf2aquaed: fconf2aquaed.o libfidoconfig.a
	$(CC) $(LOPT) fconf2aquaed.o -o fconf2aquaed -lfidoconfig -lsmapilnx

tparser: tparser.o libfidoconfig.a
	$(CC) $(LOPT) tparser.o -o tparser -lfidoconfig -lsmapilnx

clean:
	-rm *~
	-rm *.o
	-rm core

distclean: clean
	-rm tparser fconf2golded fconf2msged fconf2aquaed
	-rm libfidoconfig.so.$(VER)
	-rm libfidoconfig.a

install: 
	cp -f libfidoconfig.so.$(VER) $(LIBDIR)
	ln -sf $(LIBDIR)/libfidoconfig.so.$(VER) $(LIBDIR)/libfidoconfig.so.0
	ln -sf $(LIBDIR)/libfidoconfig.so.0 $(LIBDIR)/libfidoconfig.so
	install -s fconf2golded fconf2msged fconf2aquaed $(INSTDIR)/bin
	install -s tparser $(INSTDIR)/bin
	install -s fidoconfig.h $(INSTDIR)/include
