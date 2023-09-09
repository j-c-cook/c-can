# c-can

A controller area network (CAN) library written in C.

## Background

Logging to binary log format (BLF) with the [python-can](https://github.com/hardbyte/python-can) package on embedded Linux (armv7l chip archetecture) resulted in dropped CAN frames. When supplied with 500 messages per second at a bitrate of 250 kb/s (about a 57% bus load), the `python-can` package missed more than 13% of messages on the armv7l chip. (The issue was not associated with `python-can` itself, but rather a lack of computing resources to effectively run a daemon in Python on the low power CPU.) `c-can` has been developed to enable successful BLF CAN logging on the armv7l archetecture. Though, the library can be cross-compiled for any archetecture (granted there is a compiler available). 

## Features 

`c-can` has been developed with a generic IO and bus interface that is modeled after `python-can`. The feature set is intended to be expanded to enable cross-platform usage. 

### Log writer types

- [x] BLFWriter - Ported from https://github.com/hardbyte/python-can/blob/develop/can/io/blf.py

### Log reader types

- [ ] BLFReader

### Interfaces

- [x] socketcan (Linux)
- [ ] vector (Windows)
- [ ] pcan (Windows, MacOS) 

## Usage

Create a rotating logger that is mounted to `can0` using the `socketcan` interface.
`./rlogger &`
