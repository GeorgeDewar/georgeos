## Adventures in OS Development - Ep 1: Intro

This folder contains the source code corresponding to [Episode 1](https://www.youtube.com/watch?v=tr0IeiFYJdk&feature=youtu.be) of Adventures in OS Development, my YouTube video series where we start to build a simple operating system from scratch.

In the video, we wrote **bootload.asm** which assembles into a valid bootloader that will print "Hi" to the screen.

To run in, make sure you have [nasm](https://www.nasm.us/) (to assemble it) and [qemu](https://www.qemu.org/) (to run it) installed, then try running `build && run` from your command line.

If you're using an operating system other than Windows, just look at those two scripts as they only have one command each. You'll figure it out :)

In the next "episode" we should be able to cover a lot more ground as we make our bootloader do more useful stuff.
