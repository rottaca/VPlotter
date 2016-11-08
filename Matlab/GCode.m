gCodeHome = @() 'G28';
gCodeMoveTo = @(X,Y) strcat('G0 X', num2str(X), ' Y',  num2str(Y));

gCodeSleep = @() 'M1';
gCodeStop = @() 'M0';
gCodeEnd = @() 'M2';
gCodePenDown = @() 'M3';
gCodePenUp = @() 'M4';


