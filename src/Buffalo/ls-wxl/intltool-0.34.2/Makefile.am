## Process this file with automake to produce Makefile.in

SUBDIRS = tests doc

# Scripts which are bundled with all intltoolized packages:
distributed_scripts =				\
	intltool-extract			\
	intltool-merge				\
	intltool-update

# The intltool scripts
bin_SCRIPTS =					\
	$(distributed_scripts)			\
	intltoolize				\
	intltool-prepare

# Deprecated:
EXTRA_SCRIPTS = intltool-unicodify

# These are required by intltoolize
dist_pkgdata_DATA =	Makefile.in.in
nodist_pkgdata_DATA =	$(distributed_scripts:=.in)

# This macro file should be visible to Automake's aclocal.
dist_aclocal_DATA = intltool.m4

CLEANFILES =					\
	$(bin_SCRIPTS)				\
	$(EXTRA_SCRIPTS)			\
	$(nodist_pkgdata_DATA)

EXTRA_DIST =					\
	$(distributed_scripts:=.in.in)		\
	intltool-prepare.in			\
	intltool-unicodify.in			\
	intltool.spec

edit_in_in = sed -e 's|[@]PACKAGE@|$(PACKAGE)|g' \
	    -e 's|[@]VERSION@|$(VERSION)|g'

edit = $(edit_in_in) -e "s|[@]INTLTOOL_PERL@|$(PERL)|g" \
	    -e "s|[@]INTLTOOL_ICONV@|$(INTLTOOL_ICONV)|g" \
	    -e "s|[@]INTLTOOL_MSGFMT@|$(INTLTOOL_MSGFMT)|g" \
	    -e "s|[@]INTLTOOL_MSGMERGE@|$(INTLTOOL_MSGMERGE)|g" \
	    -e "s|[@]INTLTOOL_XGETTEXT@|$(INTLTOOL_XGETTEXT)|g" \
	    -e "s|[@]INTLTOOL_EXTRACT@|$(bindir)/intltool-extract|g" \
	    -e "s|[@]INTLTOOL_LIBDIR@|$(libdir)|g"

intltool-extract.in: intltool-extract.in.in Makefile
	$(edit_in_in) "$(srcdir)/intltool-extract.in.in" > $@

intltool-merge.in: intltool-merge.in.in Makefile
	$(edit_in_in) "$(srcdir)/intltool-merge.in.in" > $@

intltool-update.in: intltool-update.in.in Makefile
	$(edit_in_in) "$(srcdir)/intltool-update.in.in" > $@

%: %.in Makefile
	$(edit) < $< > $@
