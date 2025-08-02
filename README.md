# owsc

A _very simple_ [obs-websocket](https://github.com/obsproject/obs-websocket)
client library written in C.

## Getting Started

[mongoose](https://github.com/cesanta/mongoose) is added as a git
submodule, so make sure to clone the repository with `--recurse-submodules`.

```console
$ git clone --recurse-submodules https://github.com/UserIsntAvailable/owsc
```

### Building

Running `build.sh` would build a shared library in the `build`
directory.

```console
$ ./build.sh
```

You can also build and run the provided examples located in the
`examples` directory.

```console
$ ./build.sh run simple
```

## Acknowledgment

- This library is heavily inspired in [obs-wsc](https://github.com/univrsal/obs-wsc),
but _only_ supporting the newer 5.0 version of the `obs-websocket` protocol.
