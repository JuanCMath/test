CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_GNU_SOURCE
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)
LIBS = -lssl -lcrypto $(GTK_LIBS) -lpthread
TARGET = matcomguard

SOURCES = main.c \
          usb_monitor.c \
          process_monitor.c \
          port_scanner.c \
          config.c \
          interface.c

OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean install uninstall setup-dirs test debug help

all: setup-dirs $(TARGET)

setup-dirs:
	@mkdir -p build
	@mkdir -p docs
	@mkdir -p config
	@mkdir -p logs

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LIBS)

%.o: %.c matcomguard.h
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -rf build

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)
	sudo mkdir -p /etc/matcomguard
	@echo "MatCom Guard installed successfully!"
	@echo "Run with: matcomguard"

uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)
	sudo rm -rf /etc/matcomguard
	@echo "MatCom Guard uninstalled."

test: $(TARGET)
	./$(TARGET)

debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

help:
	@echo "MatCom Guard - Sistema de Seguridad UNIX"
	@echo "======================================="
	@echo "Available targets:"
	@echo "  all        - Build the project"
	@echo "  clean      - Remove build files"
	@echo "  install    - Install to system (/usr/local/bin)"
	@echo "  uninstall  - Remove from system"
	@echo "  test       - Run the program"
	@echo "  debug      - Build with debug symbols"
	@echo "  setup-dirs - Create project directories"
	@echo "  help       - Show this help"
	@echo ""
	@echo "Dependencies:"
	@echo "  sudo apt-get install libgtk-3-dev libssl-dev build-essential"
	@echo ""
	@echo "Usage:"
	@echo "  make all && ./matcomguard"
