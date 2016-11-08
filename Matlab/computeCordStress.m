function [ F1, F2 ] = computeCordStress(calib, L1, L2, X, Y )
    % Calib parameters in meters
    % Combined force 
    FG = calib.M*9.81;
    
    sinA = Y/L1;
    sinB = Y/L2;
    
    cosA = X/L1;
    cosB = (calib.B-X)/L2;
    
    F1 = cosB*FG/(cosA*sinB+sinA*cosB);
    F2 = cosA*FG/(cosA*sinB+sinA*cosB);
   
end

