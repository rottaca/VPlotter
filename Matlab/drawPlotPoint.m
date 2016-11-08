function [  ] = drawPlotPoint(calib, L1, L2, X, Y )
%UNTITLED5 Summary of this function goes here
%   Detailed explanation goes here

    plot([0 calib.B],[0 0],'DisplayName','Baselength');
    
    scatter(X,Y,'filled','DisplayName','DrawPoint');
    
    plot([0 X],[0 Y],'DisplayName','L1New');
    plot([0+calib.B X],[0 Y],'DisplayName','L2New');
    
    axis equal;
    axis tight;
    xlabel('X axis');
    ylabel('Y axis');
    set(gca, 'YDir', 'reverse');
    legend show;

end

