# c-can

A controller area network (CAN) library written in C.

## Background

Logging to binary log format (BLF) with the [python-can][#1] package on the armv7l chip architecture in embedded Linux 
user space resulted in dropped CAN frames (related to [python-can/issues/474][#2]). When supplied with 500 messages per 
second at a bitrate of 250 kb/s (about a 57% bus load), the `python-can` package missed more than 13% of messages on 
the armv7l chip. (The issue was not associated with `python-can` itself, but rather a lack of computing resources to 
effectively run a daemon in Python on the low power CPU.) `c-can` has been developed to enable successful BLF CAN 
logging on the armv7l architecture. Though, the library can be cross-compiled for any architecture (granted there is a 
compiler available). 

## Features 

`c-can` has been developed with a generic IO and bus interface that is modeled after `python-can`. The feature set is 
intended to be expanded to enable cross-platform usage. 

### Log writer types

- [x] BLFWriter - Ported from https://github.com/hardbyte/python-can/blob/develop/can/io/blf.py

### Log reader types

- [ ] BLFReader

### Interfaces

- [x] socketcan (Linux)
- [ ] vector (Windows)
- [ ] pcan (Windows, MacOS) 

## Cross-compile

See [cross-compile.md](cross-compile.md) for instructions on how to use the Linaro gnueabihf 
toolchain to create a compiled library that can execute on an armv7l chip. 

## Usage

Create a rotating logger daemon that is mounted to `can0` using the `socketcan` interface.

```
channel="can0"
file_name="can1.blf"
channel_idx=1
./rlogger ${channel} ${file_name} ${channel_idx} &
```

[#1]: https://github.com/hardbyte/python-can
[#2]: https://github.com/hardbyte/python-can/issues/474