@REM Compiler Flags:
@REM   -B   compile via assembly
@REM   -G   compile for speed
@REM   -I.. include dir
@REM   -L.. lib dir
@REM   -ml  Large memory model - needed for far ptrs
@REM   -c   compile only
@DEL *.OBJ

@REM Compile to Object
TCC -G -I\TC\INCLUDE -L\TC\LIB -ml -c GRAPH3.C
TCC -B -G -I\TC\INCLUDE -L\TC\LIB -ml -c GRAPH4.C
TCC -G -I\TC\INCLUDE -L\TC\LIB -ml -c GRAPH5.C
TCC -B -G -I\TC\INCLUDE -L\TC\LIB -ml -c GRAPH6.C
TCC -B -G -I\TC\INCLUDE -L\TC\LIB -ml -c GRAPH7J.C
TCC -B -G -I\TC\INCLUDE -L\TC\LIB -ml -c GRAPH7K.C
TCC -G -I\TC\INCLUDE -L\TC\LIB -ml -c GRAPH7M.C
TCC -G -I\TC\INCLUDE -L\TC\LIB -ml -c GRAPH8.C

@REM Make Library
@DEL GAMELIB.LIB
@DEL GAMELIB.TXT
TLIB GAMELIB.LIB +GRAPH3.OBJ +GRAPH4.OBJ +GRAPH5.OBJ +GRAPH6.OBJ +GRAPH7J.OBJ +GRAPH7K.OBJ +GRAPH7M.OBJ +GRAPH8.OBJ, GAMELIB.TXT
