PREFIX ?= /usr
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share
APPDIR = $(DATADIR)/applications
ICONDIR = $(DATADIR)/icons/hicolor/128x128/apps
LOCALEDIR = $(DATADIR)/locale

CC = gcc
CFLAGS = `pkg-config --cflags elementary ecore` -Wall -O2
LDFLAGS = `pkg-config --libs elementary ecore`

SRC = $(wildcard src/*.c)
TARGET = nl-ease

POFILES = $(wildcard po/*.po)
MOFILES = $(POFILES:.po=.mo)

all: $(TARGET) mo

$(TARGET):
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

# Compile translations
mo: $(MOFILES)

po/%.mo: po/%.po
	msgfmt $< -o $@

# Install
install: all
	# binary
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $(TARGET) $(DESTDIR)$(BINDIR)/

	# desktop file
	mkdir -p $(DESTDIR)$(APPDIR)
	cp data/nl-ease.desktop $(DESTDIR)$(APPDIR)/

	# icon
	mkdir -p $(DESTDIR)$(ICONDIR)
	cp data/icon/icon.png $(DESTDIR)$(ICONDIR)/nl-ease.png

	# translations
	for mo in $(MOFILES); do \
		lang=$$(basename $$mo .mo); \
		mkdir -p $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES; \
		cp $$mo $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES/nl-ease.mo; \
	done

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(APPDIR)/nl-ease.desktop
	rm -f $(DESTDIR)$(ICONDIR)/nl-ease.png
	for mo in $(MOFILES); do \
		lang=$$(basename $$mo .mo); \
		rm -f $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES/nl-ease.mo; \
	done

	@if [ -n "$(SUDO_USER)" ]; then \
		USER_HOME=$$(getent passwd $(SUDO_USER) | cut -d: -f6); \
	else \
		USER_HOME=$$HOME; \
	fi; \
	rm -f "$$USER_HOME/.config/nl-ease.conf"; \
	rm -f "$$USER_HOME/.config/nl-ease.lock"; \
	rm -f "$$USER_HOME/.config/nl-ease.pid"

clean:
	rm -f $(TARGET) po/*.mo
