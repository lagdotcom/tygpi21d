@REM Compiler Flags:
@REM   -B   compile via assembly
@REM   -G   compile for speed
@REM   -I.. include dir
@REM   -L.. lib dir
@REM   -mc  Compact memory model: small code, large data
@REM   -c   compile only
@DEL *.OBJ

@REM Compile to Object
TCC    -G -I\TC\INCLUDE -L\TC\LIB -mc -c GRAPH3.C
TCC -B -G -I\TC\INCLUDE -L\TC\LIB -mc -c GRAPH4.C
TCC    -G -I\TC\INCLUDE -L\TC\LIB -mc -c GRAPH5.C
TCC -B -G -I\TC\INCLUDE -L\TC\LIB -mc -c GRAPH6.C
TCC -B -G -I\TC\INCLUDE -L\TC\LIB -mc -c GRAPH7J.C
TCC -B -G -I\TC\INCLUDE -L\TC\LIB -mc -c GRAPH7K.C
TCC    -G -I\TC\INCLUDE -L\TC\LIB -mc -c GRAPH7M.C
TCC    -G -I\TC\INCLUDE -L\TC\LIB -mc -c GRAPH8.C

@REM Make Library
@DEL GAMELIB.LIB
@DEL GAMELIB.TXT
TLIB GAMELIB.LIB +GRAPH3.OBJ +GRAPH4.OBJ +GRAPH5.OBJ +GRAPH6.OBJ +GRAPH7J.OBJ +GRAPH7K.OBJ +GRAPH7M.OBJ +GRAPH8.OBJ, GAMELIB.TXT
