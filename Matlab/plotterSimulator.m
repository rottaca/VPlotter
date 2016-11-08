% http://www.homofaciens.de/technics-machines-v-plotter_en.htm

%%%%%%%%%%%%%%%
%% CALIBRATION PART
%%%%%%%%%%%%%%%
% Baselength between both stepper motors
% All values in meters
calib = {};
calib.M = 2;
calib.B = 1;
calib.L1 = 0;
calib.L2 = 1;

[calib.X0,calib.Y0] = computePoint(calib.B,calib.L1,calib.L2);

drawCalibration(figure(1),calib);

[L1, L2, X, Y] = computeCordLength(calib, 0.5,0.5);
drawPlotPoint(calib, L1, L2, X, Y);

[ StressL1, StressL2, Quality, Resolution ] = computeStatisticsImages( calib );
figure(2);
image(StressL1);
title('Cord tension left');
colormap jet;
colorbar;
axis equal;
figure(3);
image(StressL2);
title('Cord tension right');
colormap jet;
colorbar;
axis equal;

figure(4);
imagesc(Quality);
title('Valid drawing area');
colormap jet;
axis equal;

figure(5);
imagesc(Resolution);
title('Minimum delta movement between pixels');
colormap jet;
colorbar;
axis equal;