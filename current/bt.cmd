nasm -O0 -f as86 -o entry.obj src/kernel/entry.asm || exit /b
wsl bcc -c -x -I -o kernel.obj src/kernel/kernel2.c || exit /b
wsl ld86 -d -0 -o kernel.bin kernel.obj || exit /b
ndisasm kernel.bin
