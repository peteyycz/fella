CC ?= cc
CFLAGS ?= -Wall -Wextra -O2
PKG_CONFIG ?= pkg-config

RAYLIB_CFLAGS := $(shell $(PKG_CONFIG) --cflags raylib)
RAYLIB_LIBS := $(shell $(PKG_CONFIG) --libs raylib)

TARGET := fella
SRC := main.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC) clay.h clay_renderer_raylib.c
	$(CC) $(CFLAGS) -o $@ $(SRC) $(RAYLIB_CFLAGS) $(RAYLIB_LIBS) -lm -lpthread -ldl

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
