wasm -q -0 -fo=entry.obj entry.asm || exit /b
wcc -q -0 -d0 -ms -s -wx -zls -zc -ecd -fo=test.obj test.c || exit /b
wlink ^
  FILE entry.obj FILE test.obj ^
  NAME test.bin FORMAT DOS OUTPUT RAW^
  OFFSET=0x0000 OPTION NODEFAULTLIBS^
  OPTION FILLCHAR=0x90^
  ORDER CLNAME CODE^
      SEGMENT ENTRY OFFSET=0x0000^
  CLNAME DATA || exit /b
