## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.7

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 configure config.h.in \
                       stamp-h.in config.log config.cache config.status

SUBDIRS = Client MSVC/SettingsGUI

docdir = $(prefix)/share/doc/$(PACKAGE)-$(VERSION)
sharedir = $(prefix)/share

gnome_screensaver_dir = /usr/lib/gnome-screensaver

# add documentation files here
doc_DATA = AUTHORS README NEWS ChangeLog Runtime/Instructions.rtf Runtime/License.rtf

# extra files to be included in distributions
EXTRA_DIST = \
	configure \
	COPYING \
	INSTALL \
	electricsheep-saver \
	electricsheep-saver-gnome \
	Runtime/Scripts/*.lua \
	Runtime/Scripts/Dictionary/*.lua \
	Runtime/Scripts/logging/*.lua \
	menu-entries/ElectricSheep.desktop \
	menu-entries/electricsheep-saver.desktop \
	menu-entries/electricsheep.xpm \
	Runtime/electricsheep-attr.png \
	Runtime/electricsheep-frown.png \
	Runtime/electricsheep-smile.png \
	Runtime/TrebuchetMS-20.glf \
	Runtime/TrebuchetMS-24.glf \
	Runtime/sheep_logo.xpm \
	Runtime/sheep_logo2.xpm \
	wxWidgets-2.9.1 \
	$(doc_DATA)


dist-hook:
	rm -rf `find $(distdir)/wxWidgets-2.9.1 -name svn`


installdirs: 
	mkinstalldirs
	$(srcdir)/mkinstalldirs
	mkdir -p $(DESTDIR)$(bindir)
	mkdir -p $(DESTDIR)$(sharedir)/electricsheep/Scripts
	mkdir -p $(DESTDIR)$(sharedir)/applications
	mkdir -p $(DESTDIR)$(sharedir)/pixmaps
	mkdir -p $(DESTDIR)$(sharedir)/electricsheep/icons
	mkdir -p $(DESTDIR)$(gnome_screensaver_dir)

install-data-local:
	cp -rf $(srcdir)/Runtime/Scripts $(DESTDIR)$(sharedir)/electricsheep
	mkdir -p $(DESTDIR)$(sharedir)/applications/
	cp -f $(srcdir)/menu-entries/ElectricSheep.desktop $(DESTDIR)$(sharedir)/applications/
	mkdir -p $(DESTDIR)$(sharedir)/pixmaps/
	cp -f $(srcdir)/menu-entries/electricsheep.xpm $(DESTDIR)$(sharedir)/pixmaps/
	cp -f $(srcdir)/Runtime/electricsheep-attr.png $(DESTDIR)$(sharedir)/electricsheep/
	cp -f $(srcdir)/Runtime/electricsheep-frown.png $(DESTDIR)$(sharedir)/electricsheep/
	cp -f $(srcdir)/Runtime/electricsheep-smile.png $(DESTDIR)$(sharedir)/electricsheep/
	cp -f $(srcdir)/Runtime/TrebuchetMS-20.glf $(DESTDIR)$(sharedir)/electricsheep/
	cp -f $(srcdir)/Runtime/TrebuchetMS-24.glf $(DESTDIR)$(sharedir)/electricsheep/
	mkdir -p $(DESTDIR)$(bindir)
	cp -f $(srcdir)/electricsheep-saver $(DESTDIR)$(bindir) && chmod a+rx $(DESTDIR)$(bindir)/electricsheep-saver
	mkdir -p $(DESTDIR)/usr/share/applications/screensavers/
	cp -f $(srcdir)/menu-entries/electricsheep-saver.desktop $(DESTDIR)/usr/share/applications/screensavers/electricsheep.desktop
	mkdir -p $(DESTDIR)$(gnome_screensaver_dir)
	cp -f $(srcdir)/electricsheep-saver-gnome $(DESTDIR)$(gnome_screensaver_dir)/electricsheep-saver && chmod a+rx $(DESTDIR)$(gnome_screensaver_dir)/electricsheep-saver

# install-exec stuff goes here
install-exec-local:


# uninstall stuff goes here
uninstall-local:
	rm -rf $(DESTDIR)$(bindir)/electricsheep
	rm -rf $(DESTDIR)$(bindir)/electricsheep-preferences
	rm -rf $(DESTDIR)$(bindir)/electricsheep-saver
	rm -rf $(DESTDIR)$(docdir)
	rm -rf $(DESTDIR)$(sharedir)/electricsheep
	rm -rf $(DESTDIR)$(sharedir)/pixmaps/electricsheep.xmp
	rm -rf $(DESTDIR)$(sharedir)/applications/ElectricSheep.desktop
	rm -rf $(DESTDIR)/usr/share/applications/screensavers/electricsheep.desktop
	rm -rf $(DESTDIR)$(gnome_screensaver_dir)/electricsheep-saver

# anything to be cleaned up for distro ?
DISTCLEANFILES =

# flags to pass to automake
ACLOCAL_AMFLAGS = -I m4
