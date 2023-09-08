# c-can

A controller area network (CAN) library for written in C.

## Background

Remote CAN data logging with an armv7l processor resulted in dropped CAN frames while using the 
`python-can` package. When supplied 500 messages per second with a bitrate of 250 kb/s (about a 
57% bus load), the `python-can` package missed more than 13% of messages. This library, 
`c-can`, captures all messages sent on the CAN bus when running on an armv7l processor. 

## Log writer types

- [x] BLFWriter - Ported from https://github.com/hardbyte/python-can/blob/develop/can/io/blf.py

## Log reader types

- [ ] BLFReader

## Interfaces

- [x] socketcan
- [ ] vector
- [ ] pcan

## Setup

`sudo apt install emscripten`
`sudo apt install cmake`
`sudo apt install `
