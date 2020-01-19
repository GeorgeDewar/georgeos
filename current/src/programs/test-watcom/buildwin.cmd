wasm -q -0 -fo=entry.obj entry.asm || exit /b
wcc -q -0 -d0 -ms -s -wx -zls -zc -fo=test.obj test.c || exit /b
wlink ^
  FILE entry.obj FILE test.obj ^
  NAME test.bin FORMAT DOS OUTPUT RAW^
  OFFSET=0x0000 OPTION NODEFAULTLIBS^
  ORDER CLNAME CODE^
      SEGMENT ENTRY OFFSET=0x0000^
  CLNAME DATA || exit /b
