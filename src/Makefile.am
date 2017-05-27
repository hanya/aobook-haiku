AM_CPPFLAGS=-I../mlib/include -Isub -I@freetype_dir@ -DPACKAGE_DATA_DIR=\""$(datadir)/aobook"\"
AM_CFLAGS=-pthread

bin_PROGRAMS=aobook
aobook_LDADD=../mlib/libmlib.a -lX11 -lXext -lfontconfig -lfreetype -lpng -ljpeg -lz -lrt -lm

aobook_SOURCES= \
bookmarkdat.c \
bookmarkwin.c \
captiondlg.c \
envoptdlg.c \
envoptdlg_key.c \
envoptdlg_mouse.c \
envoptdlg_opt.c \
envoptdlg_tool.c \
filedialog.c \
globaldata.c \
main.c \
mainfunc.c \
mainwindow.c \
style.c \
stylelist.c \
styleoptdlg.c \
styleoptdlg_basic.c \
styleoptdlg_char.c \
styleoptdlg_font.c \
unzip.c \
sub/aoFont.c \
sub/aoLayout.c \
sub/aoLayout_draw.c \
sub/aoLayout_line.c \
sub/aoLayout_main.c \
sub/aoLayout_sub.c \
sub/aoTextConv.c \
sub/aoTextConv_sub.c \
sub/aoTextLoad.c\
appicon.h \
bookmarkdat.h \
bookmarkwin.h \
deftrans.h \
envoptdlg.h \
envoptdlg_pv.h \
globaldata.h \
mainfunc.h \
mainwindow.h \
menudat.h \
style.h \
stylelist.h \
styleoptdlg_pv.h \
sub/_aobufdef.h \
sub/_aolayout.h \
sub/_aotextconv.h \
sub/aoFont.h \
sub/aoLayout.h \
sub/aoStyle.h \
sub/aoText.h \
trgroup.h \
trid.h \
trid_menu.h
