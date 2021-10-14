# GeorgeOS 32bit

A toy/educational operating system.

This is the 32-bit re-write of the earlier beginnings found in the 16bit directory.

Desired features in order of priority:
- [x] Hardware support for screen, keyboard, serial communication
- [x] Timer (delay, etc)
- Drive support:
  - [x] Floppy
  - [ ] ATA hard disk
  - [ ] USB mass storage
- File systems:
  - [x] FAT12
  - [ ] FAT16
  - [ ] FAT32
- Dynamic memory allocation
- A filesystem that draws from Unix and Windows principals
  - Forward slashes, root is /
  - Everything is a file (that can be read from or written to)
  - Every file on a drive is under the drive, e.g. /drive/floppy1/file.txt
- Networking support + TCP/IP stack
- GUI
- Simple web browser
- Sound support
- Multitasking
- Successfully compile one or two existing programs for new OS
- More advanced graphics support (higher res / mulitple monitors)

## Resources

### General OS Dev / Tutorial resources

- https://wiki.osdev.org/Main_Page
  Heaps of good general info
- https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf
  An exceptional, but unfinished tutorial
- http://www.osdever.net/tutorials/view/brans-kernel-development-tutorial
  A nice clear approach for handling interrupts / IRQs
- https://www.youtube.com/playlist?list=PLHh55M_Kq4OApWScZyPl5HhgsTJS9MZ6M
  Lots of good content - although it's in C++, most of it translates

### Floppy

- http://www.brokenthorn.com/Resources/OSDev20.html
- https://github.com/levex/osdev/blob/master/drivers/fdc.c
- http://bos.asmhackers.net/docs/floppy/docs/floppy_tutorial.txt

### NASM

- https://nasm.us/doc/nasmdoc4.html
  Getting those macros right

### Debugging

- https://sourceware.org/gdb/onlinedocs/gdb/index.html
  You need to know how to inspect registers, memory and set breakpoints at least
