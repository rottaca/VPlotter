function [  ] = drawCalibration( figureHandle, calib )
    figure(figureHandle);
    cla;
    hold on;
    % Plot origin
    scatter(0,0,'filled','DisplayName','Origin');
    plot([0 calib.B],[0 0],'DisplayName','Baselength');
    
    scatter(calib.X0,calib.Y0,'filled','DisplayName','DrawOrigin');
    plot([0 calib.X0],[0 calib.Y0],'DisplayName','L1');
    plot([0+calib.B calib.X0],[0 calib.Y0],'DisplayName','L2');
    
    plot([0 calib.X0],[0 0],'DisplayName','X0');
    plot([calib.X0 calib.X0],[0 calib.Y0],'DisplayName','Y0');
    
    axis equal;
    axis tight;
    xlabel('X axis');
    ylabel('Y axis');
    set(gca, 'YDir', 'reverse');
    legend show;
end

