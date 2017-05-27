EXTRA_DIST = aobook.desktop aobook.png aobook.svg

desktopdir = $(datadir)/applications
desktop_DATA = aobook.desktop

icondir = $(datadir)/icons/hicolor/48x48/apps
icon_DATA = aobook.png

svgdir = $(datadir)/icons/hicolor/scalable/apps
svg_DATA = aobook.svg

## rule

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache

update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating GTK icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated. After (un)install, run this:"; \
		echo "*** $(gtk_update_icon_cache)"; \
	fi
