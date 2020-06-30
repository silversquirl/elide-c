SOURCES := $(wildcard src/*.c)
SOURCES += src/lex.yy.c src/y.tab.c
OBJECTS := $(patsubst src/%.c,build/obj/%.o,$(SOURCES))

HEADERS := $(wildcard src/*.h)

AR := ar
CC := clang -std=c11
CFLAGS := -Wall -Wno-parentheses -Ilib/vlib -D_POSIX_C_SOURCE=200809L
LDFLAGS := -ly

.PHONY: all clean
all: cec test
clean:
	rm -f cec
	rm -rf build/
	rm -f src/y.tab.c src/y.tab.h
	rm -f src/lex.yy.c

cec: $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

build/lib/libcec.a: $(filter-out main.c,$(OBJECTS))
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

build/obj/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

.INTERMEDIATE: src/y.tab.c src/y.tab.h
src/y.tab.c src/y.tab.h: src/parse.y
	@mkdir -p $(dir $@)
	$(YACC) -d -b src/y $<

.INTERMEDIATE: src/lex.yy.c
src/lex.yy.c: src/lex.l src/y.tab.h
	@mkdir -p $(dir $@)
	cd src; $(LEX) lex.l

# Unit tests
VTEST_DIR := test
VTEST_DEPS := build/lib/libcec.a
VTEST_CFLAGS := $(CFLAGS) -Isrc/
VTEST_LDFLAGS := -Lbuild/lib -lcec
include lib/vlib/test/vtest.mk
