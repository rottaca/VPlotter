function [ Str ] = generateGCode( img )
    GCode;
    Str = strvcat(gCodePenUp(),gCodeHome());
    
    penDown = false;
    
    for y = 0:size(img,1)-1
       Str = strvcat(Str,gCodeMoveTo(0,y));
       for x = 0:size(img,2)-1
           c = img(x+1,y+1);
           
           if(c > 0.5 && ~penDown)
                Str = strvcat(Str,gCodePenDown());
                penDown = true;
           elseif (c < 0.5 && penDown)
                Str = strvcat(Str,gCodePenUp());
                penDown = false;
           end
           Str = strvcat(Str,gCodeMoveTo(x,y));
       end
       Str = strvcat(Str,gCodePenUp());
       penDown = false;
    end

end

