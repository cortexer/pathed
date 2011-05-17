# makefile for pathed
# Copyright (C) 2011 Neil Butterworth

SHELL := bash
PROG := pathed.exe

OUTDIRS := bin/debug bin/rel obj/debug obj/rel

PROG_REL := bin/rel/$(PROG)
PROG_DEBUG := bin/debug/$(PROG)

SRCFILES := $(wildcard src/*.cpp)

OBJFILES_REL := $(patsubst src/%.cpp,obj/rel/%.o,$(SRCFILES))
OBJFILES_DEBUG := $(patsubst src/%.cpp,obj/debug/%.o,$(SRCFILES))

DEPFILES := $(patsubst src/%.cpp,obj/%.d,$(SRCFILES))

CFLAGS := -Iinc -Wall -Wextra  -MMD -MP
DBFLAGS := -g
RELFLAGS := 

CC := g++

.PHONY:	default all testmake debug release clean dirs

default: debug 

all:	dirs clean debug release

dirs: 
	@mkdir -p  $(OUTDIRS)

debug:	$(PROG_DEBUG)

release: $(PROG_REL)

testmake:
	@echo OBJFILES_REL = $(OBJFILES_REL)
	@echo OBJFILES_DEBUG = $(OBJFILES_DEBUG)
	@echo SRCFILES = $(SRCFILES)
	@echo DEPFILES = $(DEPFILES)

clean:
	rm -f $(OBJFILES_REL) $(OBJFILES_DEBUG) $(DEPFILES) $(PROG)

$(PROG_REL): $(OBJFILES_REL)
	$(CC)  $(OBJFILES_REL) -o $(PROG_REL)
	strip $(PROG_REL)
	@echo "----  created release binary ----"


$(PROG_DEBUG): $(OBJFILES_DEBUG)
	$(CC) $(OBJFILES_DEBUG) -o $(PROG_DEBUG)
	@echo "----  created debug binary ----"

-include $(DEPFILES)

obj/rel/%.o: src/%.cpp
	$(CC) $(RELFLAGS) $(CFLAGS) -MF $(patsubst obj/rel/%.o, obj/%.d,$@) -c $< -o $@

obj/debug/%.o: src/%.cpp
	$(CC) $(DBFLAGS) $(CFLAGS) -MF $(patsubst obj/debug/%.o, obj/%.d,$@) -c $< -o $@

