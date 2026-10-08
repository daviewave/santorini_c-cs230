# Santorini (CS230 project 1) build driver.
# Targets follow docs/conventions.md section 3: all debug test check run dist clean.

# Compiler and flags. CC, OPT and LDFLAGS can be overridden on the command line
# (for example `make CC=clang` or `make OPT=-O1`).
# make predefines CC as `cc`, so plain `CC ?= gcc` would never apply; only the
# built-in default is replaced, a command-line or environment CC still wins.
ifeq ($(origin CC),default)
CC := gcc
endif
OPT ?= -O2
STD := -std=c99
# The full development warning set, treated as errors. The course only needs
# `-std=c99 -Wall`; the extra flags catch shadowing, implicit conversions,
# missing prototypes and variable length arrays before a grader does.
WARNINGS := -Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes \
            -Wmissing-prototypes -Wconversion -Wvla -Werror
CFLAGS ?= $(OPT) $(STD) $(WARNINGS)
# Dependency files so that editing a header rebuilds what includes it.
DEPFLAGS := -MMD -MP
LDFLAGS ?=

# Layout: sources in src/, everything generated under build/ or dist/.
BUILD := build
SRC := src/Santorini.c
OBJ := $(patsubst src/%.c,$(BUILD)/%.o,$(SRC))
BIN := $(BUILD)/Santorini
TEST_SRC := $(wildcard test/unit/test_*.c)
TEST_BIN := $(patsubst test/unit/%.c,$(BUILD)/test/%,$(TEST_SRC))
DIST := dist

.PHONY: all debug test unit-tests check run dist clean

# all: release build of the game into build/.
all: $(BIN)

# debug: same binary built with -O0 -g3. The objects are removed first so a
# previous release build cannot be mistaken for a debug one.
debug: OPT := -O0 -g3
debug: clean-objects $(BIN)

.PHONY: clean-objects
clean-objects:
	rm -f $(OBJ) $(OBJ:.o=.d) $(BIN)

$(BIN): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Pattern rule for every object; header dependencies land next to the object.
$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

# Unit tests include the source file directly (see test/unit/test_santorini.c),
# so each test binary depends on every source and on the harness header.
unit-tests: $(TEST_BIN)

$(BUILD)/test/%: test/unit/%.c $(SRC) test/unit/check.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

# test: build the game and the unit tests, then run every unit and e2e test.
test: all unit-tests
	test/run_tests.sh

# check: static analysis over every source with gcc's analyzer.
check:
	@mkdir -p $(BUILD)/analyzer
	$(CC) $(CFLAGS) -fanalyzer -c -o $(BUILD)/analyzer/Santorini.o $(SRC)
	@echo "analyzer: clean"

# run: build and play interactively.
run: all
	./$(BIN)

# dist: the flat Gradescope bundle (Santorini.c, README.txt and a minimal
# Makefile), then prove that it builds with the course's own flags.
define DIST_MAKEFILE
# Minimal flat Makefile for the Santorini submission bundle.
ifeq ($$(origin CC),default)
CC := gcc
endif
CFLAGS ?= -std=c99 -Wall -Wextra -O2

Santorini: Santorini.c
	$$(CC) $$(CFLAGS) -o $$@ $$<

clean:
	rm -f Santorini

.PHONY: clean
endef
export DIST_MAKEFILE

dist: all
	rm -rf $(DIST)
	mkdir -p $(DIST)
	cp $(SRC) README.txt $(DIST)/
	printf '%s\n' "$$DIST_MAKEFILE" > $(DIST)/Makefile
	$(MAKE) -C $(DIST)
	@echo "dist: bundle in $(DIST)/ builds"

# clean: remove everything generated.
clean:
	rm -rf $(BUILD) $(DIST)

-include $(OBJ:.o=.d)
