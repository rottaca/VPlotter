#ifndef GCODECOMMANDS
#define GCODECOMMANDS

#define GCODE_SET_HOME QString("M5")
#define GCODE_HOME QString("G28")
#define GCODE_SPEED_DIV(F) QString("G0 F%1").arg(F)
#define GCODE_MOVE_TO_AND_SPEED(X,Y,F) QString("G0 X%1 Y%2 F%3").arg(X).arg(Y).arg(F)
#define GCODE_MOVE_TO(X,Y) QString("G0 X%1 Y%2").arg(X).arg(Y)
#define GCODE_MOVE_X(X) QString("G0 X%1").arg(X)
#define GCODE_MOVE_Y(Y) QString("G0 Y%1").arg(Y)
#define GCODE_PEN_UP QString("M4")
#define GCODE_PEN_DOWN  QString("M3")
#define GCODE_GET_INFO QString("M7")
#define GCODE_GET_POSITION QString("M8")
#define GCODE_USE_ABSOLUTE_POS QString("G90")
#define GCODE_USE_RELATIVE_POS QString("G91")

#define GCODE_SPEED_MOVE GCODE_SPEED_DIV(3)
#define GCODE_SPEED_DRAW GCODE_SPEED_DIV(8)


#endif // GCODECOMMANDS

