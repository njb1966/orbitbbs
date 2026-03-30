#############################################################################
#
# This is the makefile for BC++ 2.0 and beyond.
#

.autodepend
.nosilent

#############################################################################
#
# Define the paths for the various types of files.
#

SRC=.
OBJ=obj
EXE=exe

.path.c   = $(SRC)
.path.obj = $(OBJ)
.path.exe = $(EXE)
.path.com = $(EXE)


#############################################################################
#
# Get 286 and 87 options.  Define the env-var "87" or "286" and the
# corresponding type of executable will be built automatically
#

!if $d(87)
OPS87=-f87
LIB87=fp87
!else
LIB87=emu
!endif

!if $d(286)
OPS286=-1
!endif


#############################################################################
#
# Code to specify options for the overlays.  Comment out these lines
# to not use overlays.
#

OVER_START = /o
OVER_END = /o-
OVER_LIB= overlay.lib
USE_OVERLAYS=


#############################################################################
#
# Define command lines to use.
#

!if $d(386)
COMPILER=bccx
!else
COMPILER=bcc
!endif

TCOPTS= -O -Z -d -ml -r -N- $(OPS87) $(OPS286) -c -IC:\BCPP31\INCLUDE

TCC = $(COMPILER) $(TCOPTS) $(TCC_OPTS) -n$(OBJ) {$(SRC)\$&.c }

!if $d(USE_OVERLAYS)
TCC_NRM = $(COMPILER) -H=$(OBJ)\bbs.sym $(TCOPTS) $(TCC_OPTS) -Y -n$(OBJ) {$(SRC)\$&.c }
TCC_OVL = $(COMPILER) -H=$(OBJ)\bbs.sym $(TCOPTS) $(TCC_OPTS) -Yo -n$(OBJ) {$(SRC)\$&.c }
TCC_VER = $(COMPILER) $(TCOPTS) $(TCC_OPTS) -Y -n$(OBJ) version.c
!else
TCC_NRM = $(TCC)
TCC_OVL = $(TCC)
TCC_VER = $(COMPILER) $(TCOPTS) $(TCC_OPTS) -n$(OBJ) version.c
!endif


#############################################################################
#
# generic compilation rule
#
.c.obj:
  $(TCC)


#############################################################################
#
#
# Normal, non-overlaid bbs files
#
BBS_NRM = \
        bbs.obj      bbsutl.obj   bbsutl1.obj  bbsutl2.obj  \
        com.obj      conio.obj    connect1.obj extrn.obj    \
        mmenu.obj    modem.obj    msgbase.obj  msgbase1.obj \
        share.obj    strings.obj  subacc.obj   utility.obj  \
        xfer.obj     ripspeed.obj

BBS_O_NRM = \
        $(OBJ)\bbs.obj      $(OBJ)\bbsutl.obj   $(OBJ)\bbsutl1.obj  \
        $(OBJ)\bbsutl2.obj  $(OBJ)\com.obj      $(OBJ)\conio.obj    \
        $(OBJ)\connect1.obj $(OBJ)\extrn.obj    $(OBJ)\mmenu.obj    \
        $(OBJ)\modem.obj    $(OBJ)\msgbase.obj  $(OBJ)\msgbase1.obj \
        $(OBJ)\share.obj    $(OBJ)\strings.obj  $(OBJ)\subacc.obj   \
        $(OBJ)\utility.obj  $(OBJ)\xfer.obj     $(OBJ)\ripspeed.obj \
        

#############################################################################
#
# Overlaid bbs files
#
BBS_OVL = \
        batch.obj    bbsovl1.obj  bbsovl2.obj  bbsovl3.obj  \
        chnedit.obj  conf.obj     defaults.obj diredit.obj  \
        extrn1.obj   gfiles.obj   gfledit.obj  ini.obj      \
        instmsg.obj  lilo.obj     misccmd.obj  multinst.obj \
        multmail.obj netsup.obj   newuser.obj  qwk.obj      \
        qwk1.obj     readmail.obj ripovl.obj   sr.obj       \
        srrcv.obj    srsend.obj   subedit.obj  subreq.obj   \
        subxtr.obj   sysopf.obj   tedit.obj    uedit.obj    \
        voteedit.obj xferovl.obj  xferovl2.obj xfertmp.obj  \
        xinit.obj

BBS_O_OVL = \
        $(OBJ)\batch.obj    $(OBJ)\bbsovl1.obj  $(OBJ)\bbsovl2.obj  \
        $(OBJ)\bbsovl3.obj  $(OBJ)\chnedit.obj  $(OBJ)\conf.obj     \
        $(OBJ)\defaults.obj $(OBJ)\diredit.obj  $(OBJ)\extrn1.obj   \
        $(OBJ)\gfiles.obj   $(OBJ)\gfledit.obj  $(OBJ)\ini.obj      \
        $(OBJ)\instmsg.obj  $(OBJ)\lilo.obj     $(OBJ)\misccmd.obj  \
        $(OBJ)\multinst.obj $(OBJ)\multmail.obj $(OBJ)\netsup.obj   \
        $(OBJ)\newuser.obj  $(OBJ)\qwk.obj      $(OBJ)\qwk1.obj     \
        $(OBJ)\readmail.obj $(OBJ)\ripovl.obj   $(OBJ)\sr.obj       \
        $(OBJ)\srrcv.obj    $(OBJ)\srsend.obj   $(OBJ)\subedit.obj  \
        $(OBJ)\subreq.obj   $(OBJ)\subxtr.obj   $(OBJ)\sysopf.obj   \
        $(OBJ)\tedit.obj    $(OBJ)\uedit.obj    $(OBJ)\voteedit.obj \
        $(OBJ)\xferovl.obj  $(OBJ)\xferovl2.obj $(OBJ)\xfertmp.obj  \
        $(OBJ)\xinit.obj


#############################################################################
#
# Building everything requires bbs.exe, return.exe, miniesm.exe and fix.exe.
#

all: bbs.exe return.exe miniesm.exe fix.exe

#############################################################################
#
# Building return.exe is easy.
#

return.exe: return.obj share.obj version.c
  $(TCC_VER)
  tlink /c /x @&&|
c0l $(OBJ)\return.obj $(OBJ)\share.obj $(OBJ)\version.obj
$*
nul
$(LIB87) mathl cl
|

miniesm.exe: miniesm.c strings.c share.c
  $(TCC_NRM)
  tlink /c /x @&&|
  c0l $(OBJ)\miniesm.obj
  $*
  nul
  $(LIB87) mathl cl
|

fix.exe: fix.obj
  $(TCC_VER)
  tlink /c /x c0l+$(OBJ)\fix+$(OBJ)\version,$(EXE)\fix,,$(LIB87)+mathl+cl


#############################################################################
#
# How to link the BBS
#

bbs.exe: $(BBS_NRM) $(BBS_OVL) version.c
  $(TCC_VER)
  tlink /c @&&|
  shrnka.obj c0l $(OBJ)\version.obj $(BBS_O_NRM) ripint.obj $(OVER_START) $(BBS_O_OVL) $(OVER_END)
$*
bbs.map
shrnk $(LIB87) mathl $(OVER_LIB) cl
|
#  c0l $(OBJ)\version.obj $(BBS_O_NRM) $(OVER_START) $(BBS_O_OVL) $(OVER_END)
#$(LIB87) mathl $(OVER_LIB) cl

#############################################################################
#
# How to make non-overlaid BBS files
#

bbs.obj     : bbs.c
  $(TCC_NRM)
bbsutl.obj  : bbsutl.c
  $(TCC_NRM)
bbsutl1.obj : bbsutl1.c
  $(TCC_NRM)
bbsutl2.obj : bbsutl2.c
  $(TCC_NRM)
com.obj     : com.c
  $(TCC_NRM)
conio.obj   : conio.c
  $(TCC_NRM)
connect1.obj: connect1.c
  $(TCC_NRM)
extrn.obj   : extrn.c
  $(TCC_NRM)
mmenu.obj   : mmenu.c
  $(TCC_NRM)
modem.obj   : modem.c
  $(TCC_NRM)
msgbase.obj : msgbase.c
  $(TCC_NRM)
msgbase1.obj: msgbase1.c
  $(TCC_NRM)
strings.obj : strings.c
  $(TCC_NRM)
share.obj   : share.c
  $(TCC_NRM)
subacc.obj  : subacc.c
  $(TCC_NRM)
utility.obj : utility.c
  $(TCC_NRM)
xfer.obj    : xfer.c
  $(TCC_NRM)
ripspeed.obj: ripspeed.c
  $(TCC_NRM)

#############################################################################
#
# how to make overlaid BBS files
#

batch.obj   : batch.c
  $(TCC_OVL)
bbsovl1.obj : bbsovl1.c
  $(TCC_OVL)
bbsovl2.obj : bbsovl2.c
  $(TCC_OVL)
bbsovl3.obj : bbsovl3.c
  $(TCC_OVL)
chnedit.obj : chnedit.c
  $(TCC_OVL)
conf.obj    : conf.c
  $(TCC_OVL)
defaults.obj: defaults.c
  $(TCC_OVL)
diredit.obj : diredit.c
  $(TCC_OVL)
extrn1.obj  : extrn1.c
  $(TCC_OVL)
gfiles.obj  : gfiles.c
  $(TCC_OVL)
gfledit.obj : gfledit.c
  $(TCC_OVL)
ini.obj     : ini.c
  $(TCC_OVL)
instmsg.obj : instmsg.c
  $(TCC_OVL)
lilo.obj    : lilo.c
  $(TCC_OVL)
misccmd.obj : misccmd.c
  $(TCC_OVL)
multinst.obj: multinst.c
  $(TCC_OVL)
multmail.obj: multmail.c
  $(TCC_OVL)
netsup.obj  : netsup.c
  $(TCC_OVL)
newuser.obj : newuser.c
  $(TCC_OVL)
readmail.obj: readmail.c
  $(TCC_OVL)
shrink.obj  : shrink.c
  $(TCC_OVL)
sr.obj      : sr.c
  $(TCC_OVL)
srrcv.obj   : srrcv.c
  $(TCC_OVL)
srsend.obj  : srsend.c
  $(TCC_OVL)
subedit.obj : subedit.c
  $(TCC_OVL)
subreq.obj  : subreq.c
  $(TCC_OVL)
subxtr.obj  : subxtr.c
  $(TCC_OVL)
sysopf.obj  : sysopf.c
  $(TCC_OVL)
tedit.obj   : tedit.c
  $(TCC_OVL)
uedit.obj   : uedit.c
  $(TCC_OVL)
voteedit.obj: voteedit.c
  $(TCC_OVL)
xferovl.obj : xferovl.c
  $(TCC_OVL)
xferovl2.obj: xferovl2.c
  $(TCC_OVL)
xfertmp.obj : xfertmp.c
  $(TCC_OVL)
xinit.obj   : xinit.c
  $(TCC_OVL)
qwk.obj     : qwk.c
  $(TCC_OVL)
qwk1.obj    : qwk1.c
  $(TCC_OVL)
ripovl.obj  : ripovl.c
  $(TCC_OVL)

fix.obj     : fix.c
  $(TCC)


#############################################################################
#
# how to make the fcns.h file.
#


.\strip.exe: strip.c
  bcc -ml strip.c
  del strip.obj

fcns: .\strip.exe
  del fcns.h
  strip fcns.h &&|
$(BBS_NRM) $(BBS_OVL)
|

