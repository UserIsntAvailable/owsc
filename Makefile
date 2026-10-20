CC       := cc
AR       := ar

CFLAGS   := -Wall -Wextra -ggdb -fPIC
CPPFLAGS := -Isrc -Ithirdparty
LFLAGS   :=
SRCS     := src/owsc.c src/crypto.c thirdparty/mongoose/mongoose.c
OBJS     := $(patsubst %.c,build/%.o,$(SRCS))
# XXX(Unavailable): CFLAGS='... -DMG_TLS=MG_TLS_OPENSSL'
# XXX(Unavailable): LFLAGS=$(pkg-config --libs openssl)

.PHONY: all build test clean

all: build/.gitignore build/libowsc.a build/libowsc.so

build/.gitignore:
	mkdir -p build
	printf '*\n' > $@

build/%.o: %.c | build/.gitignore
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

build/libowsc.a: $(OBJS)
	$(AR) rcs $@ $^

build/libowsc.so: $(OBJS)
	$(CC) -shared $(LFLAGS) $^ -o $@

clean:
	rm -rf build
