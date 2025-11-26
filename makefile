#
# Makefile for xmlviewer
# Part of Plot.mcc MUI package
# (c) 2008-2025 Michal Zukowski
#gcc -noixemul -c *.c -I. -DYAML_VERSION_MAJOR=0 -DYAML_VERSION_MINOR=2 -DYAML_VERSION_PATCH=5 -DYAML_VERSION_STRING="1.0"


EXE = xmlviewer
FLEXCAT = ../../ambient/ambient/FlexCat
CC  = gcc
CFLAGS += -noixemul -Wall -Ilibyaml
#-DMEMTRACK
LDFLAGS += -noixemul  -O3
LIBS =   -ldebug 

OBJS    =  obj/logo.o  obj/xmlviewerlist.o obj/xmlviewerexpat.o obj/xmlviewerjson.o obj/xmlvieweryaml.o obj/xmlvieweriff.o \
			obj/xmlviewerfiletype.o obj/xmlviewerdata.o obj/xmlviewertree.o obj/xmlviewerabout.o \
			libyaml/*.o \
			obj/cjson.o obj/$(EXE).o

all:    $(EXE)

clean:
	-rm -rf $(OBJS) $(EXE)
locales:
	$(FLEXCAT) catalogs/xmlviewer.cd  catalogs/polski/xmlviewer.ct catalog catalogs/polski/xmlviewer.catalog
	$(FLEXCAT) catalogs/xmlviewer.cd  catalogs/deutsch/xmlviewer.ct catalog catalogs/deutsch/xmlviewer.catalog

$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

obj/$(EXE).o: $(EXE).c xmlviewerlist.h
	$(CC) -c  $(CFLAGS) -o $@ $<

obj/logo.o: logo.c
	$(CC) -c  $(CFLAGS) -o $@ $<

obj/xmlviewerlist.o: xmlviewerlist.c  xmlviewerlist.h
	$(CC) -c  $(CFLAGS) -o $@ $<

obj/xmlviewerexpat.o: xmlviewerexpat.c  xmlviewerexpat.h
	$(CC) -c  $(CFLAGS) -o $@ $<

obj/xmlviewerjson.o: xmlviewerjson.c  xmlviewerjson.h
	$(CC) -c  $(CFLAGS) -o $@ $<

obj/xmlvieweryaml.o: xmlvieweryaml.c  xmlvieweryaml.h
	$(CC) -c  $(CFLAGS) -o $@ $<

obj/xmlvieweriff.o: xmlvieweriff.c  xmlvieweriff.h
	$(CC) -c  $(CFLAGS) -o $@ $<

obj/xmlviewerfiletype.o: xmlviewerfiletype.c  xmlviewerfiletype.h
	$(CC) -c  $(CFLAGS) -o $@ $<

obj/xmlviewerdata.o: xmlviewerdata.c  xmlviewerdata.h
	$(CC) -c  $(CFLAGS) -o $@ $<

obj/xmlviewertree.o: xmlviewertree.c xmlviewertree.h
	$(CC) -c  $(CFLAGS) -o $@ $<

obj/xmlviewerabout.o: xmlviewerabout.c xmlviewerabout.h
	$(CC) -c  $(CFLAGS) -o $@ $<
	
obj/cjson.o: cJSON.c cJSON.h
	$(CC) -c  $(CFLAGS) -o $@ $<
