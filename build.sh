#!/bin/sh

# XXX(Unavailable): I can't be bothered to create a make file right now.
# XXX(Unavailable): move `mongoose` to a `thirdparty` directory.

CC=clang
CFLAGS='-Wall -Wextra -ggdb -Isrc'
LFLAGS=''
# XXX(Unavailable): CFLAGS='-Wall -Wextra -ggdb -DMG_TLS=MG_TLS_OPENSSL'
# XXX(Unavailable): LFLAGS=$(pkg-config --libs openssl)

[ "$1" = 'clean' ] && rm -rf build && exit

mkdir -p build
echo "*" >build/.gitignore

Build()
{
    Srcs=$(find src -name '*.c' -type f -print0 | xargs -0 echo)
    $CC $CFLAGS $LFLAGS $Srcs "$@"
}

case "$1" in
        '') Build -fPIC -shared -o build/owsc.so                 ;;
     'run') Build examples/$2.c -o build/$2    && build/$2       ;;
    'test') Build tests/$2.c -o build/$2-tests && build/$2-tests ;;
         *) echo "Unknown command ($2)"        && exit 1         ;;
esac
