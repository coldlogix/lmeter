.AUTODEPEND

#		*Translator Definitions*
CC = bcc +DXCVIEW.CFG
TASM = TASM
TLIB = tlib
TLINK = tlink
LIBPATH = e:\BC\LIB
INCLUDEPATH = e:\BC\INCLUDE


#		*Implicit Rules*
.c.obj:
  $(CC) -c {$< }

.cpp.obj:
  $(CC) -c {$< }

#		*List Macros*
Link_Exclude =  \
 dxc_tab.c \
 lexyy.c

Link_Include =  \
 dxc_tab.obj \
 lexyy.obj \
 dxcview.obj \
 list.obj \
 egavga.obj

#		*Explicit Rules*
dxcview.exe: dxcview.cfg $(Link_Include) $(Link_Exclude)
  $(TLINK) /v/x/i/c/d/P-/L$(LIBPATH) @&&|
c0l.obj+
dxc_tab.obj+
lexyy.obj+
dxcview.obj+
list.obj+
egavga.obj
dxcview
		# no map file
graphics.lib+
fp87.lib+
mathl.lib+
cl.lib
|


#		*Individual File Dependencies*
dxc_tab.c: dxc.y 
	BISON -d dxc.y

lexyy.c: dxc.l 
	FLEX -8 -I -Sc:\djgcc\lib\flex.ske dxc.L

dxc_tab.obj: dxcview.cfg dxc_tab.c 

lexyy.obj: dxcview.cfg lexyy.c 

dxcview.obj: dxcview.cfg dxcview.c 

list.obj: dxcview.cfg list.c 

#		*Compiler Configuration File*
dxcview.cfg: dxcview.mak
  copy &&|
-ml
-3
-a
-f287
-j10
-g50
-N
-v
-y
-G
-O
-Og
-Oe
-Om
-Ov
-Ol
-Ob
-Op
-Oi
-Z
-k-
-d
-h
-vi-
-H=DXCVIEW.SYM
-Fc
-wbbf
-wpin
-wamb
-wamp
-wasm
-wdef
-w-rvl
-wsig
-wucp
-weas
-wpre
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DYY_USE_PROTOS
-P-.C
| dxcview.cfg


