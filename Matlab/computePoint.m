function [ X0, Y0 ] = computePoint( B, L1, L2 )
%ComputeOrigin 
    X0 = (B^2+L1^2-L2^2)/(2*B);
    Y0 = sqrt(L2^2 - (B-X0)^2);
end

