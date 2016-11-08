function [ StressL1, StressL2, Quality, Resolution ] = computeStatisticsImages( calib )
    
    res = 0.01;
    imgHeight = round(1.0/res*calib.B);
    
    StressL1 = zeros(imgHeight,calib.B/res);
    StressL2 = zeros(imgHeight,calib.B/res);
    Quality = zeros(imgHeight,calib.B/res);
    Resolution = zeros(imgHeight,calib.B/res);
    
    for x = 1:size(StressL1,2)
        for y = 1:size(StressL1,1)
            
             [L1, L2, X, Y] = computeCordLength(calib, (x-1-calib.X0)*res, (y-1-calib.Y0)*res);
             [ F1, F2 ] = computeCordStress(calib, L1, L2, X, Y );
             
             StressL1(y,x) = F1;
             StressL2(y,x) = F2;
             
             
             [L1_1, L2_1] = computeCordLength(calib, (x-calib.X0)*res, (y-1-calib.Y0)*res);
             [L1_2, L2_2] = computeCordLength(calib, (x-1-calib.X0)*res, (y-calib.Y0)*res);
             
             Resolution(y,x) = min(abs([L1_1-L1,L2_1-L2,L1_2-L1,L2_2-L2]));
             
        end
    end
    
    minTension = 0.25*9.81*calib.M;
    maxTension = 1.75*9.81*calib.M;
    
    Quality(StressL1 > maxTension | StressL1 < minTension ) = 1;
    Quality(StressL2 > maxTension | StressL2 < minTension ) = 1;
    Quality(Resolution < 0.002) = 2;
    
end

