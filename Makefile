
#   xskat - a card game for 1 to 3 players.
#   Copyright (C) 1998  Gunter Gerhardt
#
#   This program is free software; you can redistribute it freely.
#   Use it at your own risk; there is NO WARRANTY.

CFLAGS	=
LDFLAGS	= -L/usr/X11R6/lib -lX11
# perhaps you need one of: -lsocket -lnsl

DEFL    = -DDEFAULT_LANGUAGE=\"german\"
#DEFL   = -DDEFAULT_LANGUAGE=\"english\"

DEFI    = -DDEFAULT_IRC_SERVER=\"irc.uni-paderborn.de\"

SHELL   = /bin/sh
OBJECTS	= skat.o ramsch.o null.o bitmaps.o xio.o xdial.o irc.o text.o

xskat: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

clean:
	rm -f $(OBJECTS) xskat bitmaps.h

ckdef:
	@echo Checking defs.h;\
	sed -n "1,/######/p" defs.h > defs;\
	cat *.c|sed  -n 's/^\([a-zA-Z].*\)(.*)/\1();/p' >> defs;\
	diff defs defs.h || mv defs defs.h;\
	rm -f defs

bitmaps.h: bwcards.xbm colcards.xbm bwcardl.xbm colcardl.xbm icon.xbm
	@echo Generating $@;\
	(echo "/* Generated file. Don't edit. */";\
	echo "extern char colcards_pixs[];";\
	echo "extern char colcardl_pixs[];";\
	for bitmap in bwcards colcards bwcardl colcardl icon;\
	do\
		egrep width"|"height < $$bitmap.xbm;\
		echo "extern char $${bitmap}_bits[];";\
	done) > $@

text.o: text.c text.h defs.h
	$(CC) $(CFLAGS) $(DEFL) -c $< -o $@

irc.o: irc.c irc.h defs.h skat.h xio.h xdial.h text.h
	$(CC) $(CFLAGS) $(DEFI) -c $< -o $@

skat.o: skat.h defs.h text.h
null.o: null.h defs.h skat.h
ramsch.o: ramsch.h defs.h skat.h
xio.o: xio.h defs.h skat.h bitmaps.h irc.h text.h
xdial.o: xdial.h defs.h xio.h skat.h ramsch.h irc.h text.h
bitmaps.o: bwcards.xbm colcards.xbm bwcardl.xbm colcardl.xbm icon.xbm
