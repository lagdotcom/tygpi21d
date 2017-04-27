@REM Compiler Flags:
@REM   -G   compile for speed
@REM   -I.. include dir
@REM   -L.. lib dir
@REM   -mh  Huge memory model - needed for far ptrs
@REM   -c   compile only
@DEL *.OBJ

@REM Compile to Object
TCC -G -I\TC\INCLUDE -L\TC\LIB -mh -c GRAPH3.C

@REM Make Library
@DEL GAMELIB.*
TLIB GAMELIB.LIB +GRAPH3.OBJ, GAMELIB.TXT
