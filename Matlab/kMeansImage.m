function [ centroids, imageIndexed, imageClustered ] = kMeansImage( img, k )

    [h,w, c] = size(img);
    if(max(img(:)) > 1)
        img = double(img)./255.0;
    end

    imageIndexed = zeros(h,w);
    centroids = rand(c,k);
    
    updateCnt = 1;
    while(updateCnt > 0)
        updateCnt = 0;
        
        % Compute cluster
        for y = 1:h
           for x = 1:w
               minDist = norm(centroids(:,1)- squeeze(img(y,x,:)));
               idx = 1;
               for i=2:k
                   dist = norm(centroids(:,i)- squeeze(img(y,x,:)));
                   if(dist < minDist)
                       minDist = dist;
                       idx = i;
                   end
               end
               oldIdx = imageIndexed(y,x);
               if(oldIdx ~= idx)
                    imageIndexed(y,x) = idx;
                    updateCnt = updateCnt + 1;
               end
           end
        end
        % Update clustercenters
        for i=1:k
            for j=1:c
                channel = img(:,:,j);
                centroids(j,i) = double(sum(channel(imageIndexed == i)))/length(channel(imageIndexed == i));
            end
            if isnan(norm(centroids(:,i)))
                centroids(:,i) = rand(c,1);
                disp('Invalid cluster deteced.');
            end
        end
        disp(updateCnt);
        imagesc(imageIndexed);
        drawnow;
    end
    imageClustered = centroids(imageIndexed);
    
end

