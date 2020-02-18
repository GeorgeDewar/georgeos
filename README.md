# georgeos

This project is about building a series of toy operating systems targeting the x86, of increasing sophistication.

1. A single-file boot sector that prints a Hello message
2. Our bootloader now loads a separate Kernel from disk. The Kernel is still a single ASM file.
3. We move from ASM to C and build a functional 16-bit OS with a text-based shell. It can run other programs. BIOS calls are relied on extensively.
4. We switch to Protected Mode. Now we have a 32-bit OS and 4GB of memory to play with... This one has not been started yet.

The plan is to turn this into an educational thing, in the form of blog posts and/or videos (see below).

I did my development on a Windows computer, though I'd rather be using Linux for it and may switch it over to WSL.

## Build tools required for Windows

- cmder - a very useful terminal for Windows, much better than cmd and it also also includes unix-like utilities such as grep, sort, ls, 
  etc. I downloaded the full version which includes Git for Windows.
- VS Code - as an IDE
- Open Watcom build tools - https://github.com/open-watcom/open-watcom-v2/releases - I installed the version for Windows x64, and checked
  "include 16 bit compilers", and unselected all target OSs as we are making our own OS
- ImDisk Virtual Disk Driver - http://www.ltr-data.se/files/imdiskinst.exe
- QEMU - great for fast testing

## YouTube Videos

[Episode 1: Intro](https://youtu.be/tr0IeiFYJdk) - starting on the bootloader and making it print to the screen
