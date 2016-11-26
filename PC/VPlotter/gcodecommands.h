#ifndef GCODECOMMANDS
#define GCODECOMMANDS

#define CALIBRATE(B,L,R) QString("M5 B%1 L%2 R%3").arg(B).arg(L).arg(R)
#define SPEED_DIV(F) QString("G0 F%1").arg(F)
#define MOVE_TO_AND_SPEED(X,Y,F) QString("G0 X%1 Y%2 F%3").arg(X).arg(Y).arg(F)
#define MOVE_TO(X,Y) QString("G0 X%1 Y%2").arg(X).arg(Y)
#define MOVE_X(X) QString("G0 X%1").arg(X)
#define MOVE_Y(Y) QString("G0 Y%1").arg(Y)
#define PEN_UP QString("M4")
#define PEN_DOWN  QString("M3")
#define GET_INFO QString("M7")
#define GET_POSITION QString("M8")
#define USE_ABSOLUTE_POS QString("G90")
#define USE_RELATIVE_POS QString("G91")
#define GCODE_HOME QString("G28")

#endif // GCODECOMMANDS

