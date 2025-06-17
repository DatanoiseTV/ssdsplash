CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS = -lpthread -lm -static

SRCDIR = src
OBJDIR = obj
BINDIR = bin

DAEMON_SOURCES = $(SRCDIR)/ssdsplash.c $(SRCDIR)/ssd1306.c $(SRCDIR)/font.c $(SRCDIR)/image.c $(SRCDIR)/truetype.c
CLIENT_SOURCES = $(SRCDIR)/ssdsplash-send.c

DAEMON_OBJECTS = $(DAEMON_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

DAEMON_TARGET = $(BINDIR)/ssdsplash
CLIENT_TARGET = $(BINDIR)/ssdsplash-send

.PHONY: all clean install

all: $(DAEMON_TARGET) $(CLIENT_TARGET)

$(DAEMON_TARGET): $(DAEMON_OBJECTS) | $(BINDIR)
	$(CC) $(DAEMON_OBJECTS) -o $@ $(LDFLAGS)

$(CLIENT_TARGET): $(CLIENT_OBJECTS) | $(BINDIR)
	$(CC) $(CLIENT_OBJECTS) -o $@ -static

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJDIR) $(BINDIR)

install: all
	install -D $(DAEMON_TARGET) /usr/bin/ssdsplash
	install -D $(CLIENT_TARGET) /usr/bin/ssdsplash-send
	install -D systemd/ssdsplash.service /etc/systemd/system/ssdsplash.service

install-sysv: all
	install -D $(DAEMON_TARGET) /usr/bin/ssdsplash
	install -D $(CLIENT_TARGET) /usr/bin/ssdsplash-send
	install -D etc/init.d/S30ssdsplash /etc/init.d/S30ssdsplash

.PHONY: all clean install