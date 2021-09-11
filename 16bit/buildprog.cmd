@echo off

wcc -q -0 -d0 -ms -s -wx -zls -zc -fi=src\system_library\os.h -fo=build\programs\%1.obj src\programs\%1.c || exit /b
wlink ^
  FILE src\system_library\os.obj FILE src\system_library\entry.obj FILE build\programs\%1.obj ^
  NAME build\programs\%1.exe FORMAT DOS OUTPUT RAW^
  OFFSET=0x0000 OPTION NODEFAULTLIBS^
  OPTION FILLCHAR=0x90^
  OPTION QUIET^
  ORDER CLNAME CODE^
      SEGMENT ENTRY OFFSET=0x0000^
  CLNAME DATA || exit /b

REM copy to floppy disk data directory
copy build\programs\%1.exe src\data\
