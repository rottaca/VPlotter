function [ ] = simulateGCode( code )

    currPos = [0,0];
    currPen = false;
    figure(1);
    cla;
    hold on;
    axis equal;
    
    updateCnt = 0;
    for i = 1:size(code,1)
       newPos = currPos;
       cmd = code(i,:);
       if strfind(cmd,'G28')
           newPos = [0,0];
       elseif strfind(cmd,'M3')
           currPen = true;
       elseif strfind(cmd,'M4')
           currPen = false;
       elseif strfind(cmd, 'G0')
           tmp = strsplit(cmd,' ');
           X = strjoin(tmp(2));
           Y = strjoin(tmp(3));
           newPos = [str2num(X(2:end)),str2num(Y(2:end))];
       end
       
       if currPen
          plot([currPos(1), newPos(1)],[currPos(2), newPos(2)],'b'); 
          updateCnt = updateCnt +1;
       end
       currPos = newPos;
       
       if(updateCnt > 400)
           updateCnt = 0;
           
          drawnow;
       end
       
    end

end

