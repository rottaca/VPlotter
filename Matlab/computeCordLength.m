function [ L1, L2, X_, Y_ ] = computeCordLength( calib, X,Y )
    
    X_ = X + calib.X0;
    Y_ = Y + calib.Y0;
    
    L1 = sqrt(X_^2+Y_^2);
    L2 = sqrt((calib.B - X_)^2 + Y_^2);
end

