# owsc

A _very simple_ `obs-websocket` client library written in C.

## Getting Started

You can check the provided examples in the `examples` folder.

If you want to compile the library or run any of the examples you can use
`build.sh`.

```console
# creates a `.so` file in `build`
$ ./build.sh
# runs `examples/simple.c`
$ ./build.sh run simple
```

## Acknowledgment

- This library is heavily inspired in [obs-wsc](https://github.com/univrsal/obs-wsc),
but supporting the 5.0 version of the `obs-websocket` protocol.
