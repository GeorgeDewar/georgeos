REM build the OS library
wcc -q -0 -d0 -ms -s -wx -zls -zc -fo=os.obj os.c || exit /b

REM build the standard entry library
wasm -q -0 -fo=entry.obj entry.asm || exit /b
