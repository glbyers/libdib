
srcdir	= .
CC 	= gcc
OBJS	= bmp.o
LIBS	= libbmp.so.0.0

#----------------------------------------------------------------------
# Rules Section
#----------------------------------------------------------------------

all:	libbmp

.PHONY:	all clean cleanbin dep

libbmp:	bmp.o
		$(CC) -shared -Wl,-soname,libbmp.so.0 \
		-o libbmp.so.0.0 bmp.o

bmp.o:	bmp.c bmp.h
		$(CC) -fPIC -ggdb -Wall -ansi -pedantic -c -I/usr/local/include bmp.c

clean:	cleanbin
		rm -f .depend *~ 

cleanbin:
		rm -f $(OBJS) libbmp.so*

dep:
.depend:

install: 
		install -g users -m 644 libbmp.so.0.0 /usr/local/lib
		install -g users -m 644 bmp.h /usr/local/include

		cd /usr/local/lib
		ln -sf libbmp.so.0.0 libbmp.so.0
		ln -sf libbmp.so.0.0 libbmp.so

