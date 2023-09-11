# C-CAN changelog

## Version 0.3 

- Fixes BLF compression.
- Updates bus API to be generic for hardware interfaces.

## Version 0.2

- The creation of a logger now requires a file name input.
- File logger API is made generic.
- Install instructions are added to CMakeLists. 

## Version 0.1.1

- Replace the hardcoded data length character value of 8 to utilize the value retrieved from the socket. 

## Version 0.1.0

The initial release of this library contains the following functionality:
- reception of CAN frame from socket,
- logging to Vector's proprietary Binary Log Format (BLF),
- rollover logging with both time (5 minutes) and size (1 MB) inputs.
