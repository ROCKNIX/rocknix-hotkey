.PHONY: all clean

CC ?= gcc
CXX ?= g++
CCFLAGS = -W -Wall -std=c++11 \
	`pkg-config --cflags sdl2` \
	`pkg-config --cflags libevdev`

BINARY = gptokeyb
LIBRARIES = \
	`pkg-config --libs sdl2` \
	`pkg-config --libs libevdev`
SOURCES = "gptokeyb.cpp"

INPUTFILTER_LIB = inputfilter.so
INPUTFILTER_SRC = inputfilter.c
INPUTFILTER_CFGLAGS = -Wall -fPIC -shared

all: $(BINARY) $(INPUTFILTER_LIB)

$(BINARY):
	$(CXX) $(CCFLAGS) $(INCLUDES) $(SOURCES) -o $(BINARY) $(LIBRARIES)

$(INPUTFILTER_LIB):
	$(CC) $(INPUTFILTER_CFGLAGS) $(INPUTFILTER_SRC) -o $(INPUTFILTER_LIB) -ldl

clean:
	rm -f $(BINARY) $(INPUTFILTER_LIB)
