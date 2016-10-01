CXXFLAGS+=-std=c++14 -O3 -march=native -Wno-write-strings -Wno-unused-result
debug_hydralisk:CXXFLAGS+=-g
debug_gland:CXXFLAGS+=-g
debug_gland_memcpy:CXXFLAGS+=-g

all: gland gland_memcpy hydralisk child

gland:LOADLIBES+=-lnuma

gland_memcpy:LOADLIBES+=-lnuma

hydralisk:LOADLIBES+=-lcurses

hydralisk:hydralisk.cpp InputWatcher.h

debug_hydralisk: clean hydralisk

debug_gland: clean gland

debug_gland_memcpy: clean gland_memcpy

install: 
	cp gland /usr/local/bin
	cp hydralisk /usr/local/bin
	cp gland_memcpy /usr/local/bin

clean:
	$(RM) hydralisk gland child gland_memcpy
