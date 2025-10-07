#
# Makefile for Banking Simulator
# - C++20, dependency generation
# - prefers src/*.cpp, falls back to root *.cpp
# - auto-detects inputs/ledger.txt
# - supports THREADS or lowercase threads
# - quick targets: asan, tsan, gdb, valgrind, test, install-inputs
#

SHELL := /bin/bash

CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -pthread -Iinclude -I.
LDFLAGS := -pthread

RELEASE_FLAGS := -O2
DEBUG_FLAGS := -g -O1 -fno-omit-frame-pointer

# directories and target
SRCDIR := src
BINDIR := bin
TARGET := $(BINDIR)/bank_sim

# prefer src/*.cpp else fallback to root *.cpp
SRCS := $(wildcard $(SRCDIR)/*.cpp)
ifeq ($(strip $(SRCS)),)
  SRCS := $(wildcard *.cpp)
endif

OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

MODE ?= release
ifeq ($(MODE),debug)
  CXXFLAGS += $(DEBUG_FLAGS)
else
  CXXFLAGS += $(RELEASE_FLAGS)
endif

# allow lowercase threads= override (Makefile vars are case-sensitive)
THREADS ?= 4
ifeq ($(strip $(THREADS)),)
  ifneq ($(strip $(threads)),)
    THREADS := $(threads)
  endif
endif

# LEDGER auto-detection: prefer inputs/ledger.txt if present
LEDGER ?= ledger.txt
ifneq ("$(wildcard inputs/ledger.txt)","")
  LEDGER := inputs/ledger.txt
endif

.PHONY: all build clean run debug asan tsan gdb valgrind test install-inputs help

all: build

build: $(TARGET)

# link
$(TARGET): $(OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS)
	@echo "Built $@"

# compile + generate deps
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# include dependency files (if exist)
-include $(DEPS)

$(BINDIR):
	mkdir -p $(BINDIR)

# run with defaults (THREADS and LEDGER can be overridden on the command line)
# usage: make run THREADS=4 LEDGER=inputs/ledger.txt
run: build
	@echo "Running: $(TARGET) $(THREADS) $(LEDGER)"
	@if [ ! -f "$(LEDGER)" ]; then echo "ERROR: ledger file '$(LEDGER)' not found."; exit 1; fi
	@./$(TARGET) $(THREADS) $(LEDGER)

# debug (generic debug mode - does not add sanitizers)
debug:
	$(MAKE) MODE=debug clean all

# ASAN build target
asan: CXXFLAGS += -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer
asan: clean all
	@echo "Built with ASAN: ./$(TARGET)"

# TSAN build target (for race detection)
tsan: CXXFLAGS += -g -O1 -fsanitize=thread -fno-omit-frame-pointer
tsan: clean all
	@echo "Built with TSAN: ./$(TARGET)"

# run under gdb (use after building; respects THREADS/LEDGER)
gdb: build
	@gdb --args ./$(TARGET) $(THREADS) $(LEDGER)

# run under valgrind (if installed)
valgrind: build
	@valgrind --leak-check=full ./$(TARGET) $(THREADS) $(LEDGER)

# quick test helper (runs with inputs/ledger.txt if present)
test:
	@if [ -f "inputs/ledger.txt" ]; then echo "Testing with inputs/ledger.txt"; ./$(TARGET) 1 inputs/ledger.txt; else echo "No inputs/ledger.txt found to test."; fi

# create inputs/ and a small sample ledger if absent
install-inputs:
	@mkdir -p inputs
	@if [ ! -f inputs/ledger.txt ]; then \
	  printf "0 0 500 0\n1 0 300 0\n2 0 200 0\n" > inputs/ledger.txt; \
	  echo "Created inputs/ledger.txt (sample)"; \
	else echo "inputs/ledger.txt already exists"; fi

# clean
clean:
	@echo "Cleaning..."
	-@rm -f $(OBJS) $(DEPS)
	-@rm -rf $(BINDIR)
	@echo "Clean done."

help:
	@printf "Makefile targets:\n"
	@printf "  make / make build            -> build (release)\n"
	@printf "  make run [THREADS=4 LEDGER=] -> build then run (uses inputs/ledger.txt if present)\n"
	@printf "  make debug                   -> clean + build with MODE=debug (no sanitizers)\n"
	@printf "  make asan                    -> clean + build with ASAN (address/undefined)\n"
	@printf "  make tsan                    -> clean + build with TSAN (thread sanitizer)\n"
	@printf "  make gdb                     -> launch gdb with binary and args\n"
	@printf "  make valgrind                -> run under valgrind (if installed)\n"
	@printf "  make install-inputs          -> create inputs/ledger.txt sample\n"
	@printf "  make clean                   -> remove build artifacts\n"
