# V-Plotter Project

## Setup
The system consist of two stepper motors which control the draw position and a servo which controls the drawing state (drawing/not drawing).
The hardware is controlled by a Arduino Nano V3 that understands basic G-Code commands (similar to 3D-Printer-GCodes).

A host software, written in C++ with a Qt user interface sends the commands to the controller. The software is able (in the future) to convert images and SVG graphics into gcode commands. The programm then simulates the generated movements and renderes the result in a graphics scene.


## Supported G-Code-Commands

The firmware supports currently only some basic commands.
### GCode
- G0 \[X\_\] \[Y\_\]  -> Move to/add X,Y
- G28                 -> Go To Home
- G90                 -> Absolute Positioning
- G91                 -> Relative Positioning

### MCodes
- M3                  -> Lift pen (don't draw)
- M4                  -> Move pen down (draw)
- M5 B\_ L\_ R\_         -> Calibrates the hardware control, Defines a - baselength B and left and right cord length.
- M7                  -> Debug output
- M8                  -> Get current position (X Y)
