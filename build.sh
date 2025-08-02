#!/bin/sh

# XXX(Unavailable): I can't be bothered to create a Makefile.

CC=clang
CFLAGS='-Wall -Wextra -ggdb -Isrc -Ithirdparty'
LFLAGS=''
SRCS='src/owsc.c src/crypto.c thirdparty/mongoose/mongoose.c'
# XXX(Unavailable): CFLAGS='... -DMG_TLS=MG_TLS_OPENSSL'
# XXX(Unavailable): LFLAGS=$(pkg-config --libs openssl)

[ "$1" = 'clean' ] && rm -rf build && exit

mkdir -p build
echo "*" >build/.gitignore

# shellcheck disable=2086
Build() { $CC $CFLAGS $LFLAGS $SRCS "$@"; }

case "$1" in
        '') Build -fPIC -shared   -o build/owsc.so                        ;;
     'run') Build "examples/$2.c" -o "build/$2"       && "build/$2"       ;;
    'test') Build "tests/$2.c"    -o "build/$2-tests" && "build/$2-tests" ;;
         *) echo "unknown command: $2"                && exit 1           ;;
esac
