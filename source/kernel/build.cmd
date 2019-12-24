rem wcc -0 -d0 -ml -s -wx -zls kernel.c
rem wlink FILE kernel.obj FORMAT DOS OUTPUT RAW NAME kernel.bin OPTION NODEFAULTLIBS, START=main_
rem ndisasm.exe kernel.bin

rem problem seems to be the stack isn't set right initially

wasm -0 entry.asm
wcc -3 -d0 -ms -s -wx -zls kernel.c

wlink FILE entry.obj FILE kernel.obj NAME kernel.bin FORMAT DOS OUTPUT RAW^
  OFFSET=0x0000 OPTION NODEFAULTLIBS^
  ORDER CLNAME CODE^
      SEGMENT ENTRY OFFSET=0x0000^
      SEGMENT main_TEXT OFFSET=0x0004^
  CLNAME DATA OFFSET=0x200
