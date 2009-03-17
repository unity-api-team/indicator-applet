defsdir = $(datadir)/pygtk/2.0/defs
defs_DATA = indicate.defs

CFLAGS = -Wall -Werror
INCLUDES = \
        -DG_LOG_DOMAIN=\"indicate-python\" \
        -DDATADIR=\"$(datadir)\" \
        -DLIBDIR=\"$(libdir)\" \
        $(INDICATE_PYTHON_CFLAGS) \
        $(PYTHON_INCLUDES)

pkgpythondir = $(pyexecdir)
pkgpyexecdir = $(pyexecdir)

pkgindicatedir = $(pkgpythondir)/indicate
pkgindicate_PYTHON = __init__.py

indicatedir = $(pkgpyexecdir)/indicate
indicate_LTLIBRARIES = _indicate.la

_indicate_la_LDFLAGS = -module -avoid-version -export-symbols-regex init_indicate -lpyglib-2.0-python2.6
_indicate_la_LIBADD = $(INDICATE_PYTHON_LIBS) 
_indicate_la_SOURCES = indicatemodule.c indicate.c
#nodist_indicate_la_SOURCES = indicate.c

CLEANFILES = indicate.c
EXTRA_DIST = indicate.override indicate-arg-types.py $(defs_DATA)
indicate.c: $(defs_DATA) indicate.override

%.c: %.defs
	(cd $(srcdir) \
	 && $(PYGTK_CODEGEN) \
		--register $(PYGTK_DEFSDIR)/gtk-types.defs \
		--register $(PYGTK_DEFSDIR)/gdk-types.defs \
		--load-types indicate-arg-types.py \
	    --override $*.override \
	    --prefix py$* $*.defs) > gen-$*.c \
	 && cp gen-$*.c $*.c \
	 && rm -f gen-$*.c
