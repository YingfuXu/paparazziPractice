clear;
clc;
all close;

%read image name
imagePath=fullfile('/home/yingfu/ETHDataset/Vicon_Room_102/mav0/cam0/data/');
dirOutput=dir(fullfile(imagePath,'*.png'));
imageNames = {dirOutput.name};

%print the image size
exampleFrame = imread([imagePath imageNames{1,1}]); 
imageSize = size(exampleFrame)

% %show image
% figure(1)
% imshow(image1);
% figure(2)
% imshow(image2);

%find the FAST features and show on the image
% FAST = detectFASTFeatures(image1);
% figure(1); imshow(image1);
% hold on;
% plot(FAST);

KLTtracker = vision.PointTracker('BlockSize',[33, 33],'MaxBidirectionalError',2);
%TODO: How to set  the area of the searching block of the point in the next
%frame? Is it possible for KLT? What's the theory of the KLT algorithm?

%neglect the feature near the edge of the image
edgeWidth = 20;
objectRegion = [edgeWidth, edgeWidth, imageSize(2)-2*edgeWidth, imageSize(1)-2*edgeWidth];

previousFrameKey = 1; % == 1 Means the previousFrame is a Key Frame. bool flag
%Define the first image as the first Key Frame

for i = 2:length(imageNames)/10
    
    %regared the first image as K0, so the second image is the first image which has detected flow, it comes at camera time step K1
    frameNumber = i - 1; 
    
    previousFrameName = imageNames{1,i - 1};
    %set the previous frame in which the points are detected
    if(previousFrameKey == 1)
        lastKeyFrameName = previousFrameName;
    end
    
    previousFrame = imread([imagePath lastKeyFrameName]); 
    %TODO: the objectFrame of the tracker should be the last Key Frame
    
    FAST = detectFASTFeatures(previousFrame,'MinQuality',0.6,'MinContrast',0.2,'ROI',objectRegion);  
    %TODO: Make feature points evenly distributed across the picture
    %TODO: Choose 30 points with best quality to track
    
    if(FAST.Count < 5) %Give up the image without enough FAST features
    %TODO: save the information which marks the bad texture quality
        
        continue;
    end
    
%     if(FAST.Count < 30) %Track all the points
%         
%         continue;
%     end
    
    %Set the points in the previous frame to be tracked by the KLT
    objectFrame = previousFrame;
    detectedPoints = FAST;
    initialize(KLTtracker,detectedPoints.Location,objectFrame);
    % How to initialize only one tracker and detect new point in the
    % current frame and then add them to the tracker?
    
    %set the current frame
    currentFrameName = imageNames{1,i};
    currentFrame = imread([imagePath currentFrameName]); 
    
    %track the points detected in the previous frame in the current frame
    [trackedPoints,validity] = KLTtracker(currentFrame);
    
%     figure(2)
%     %plot the Tracked Points
%     subplot(1,2,2);
%     showTrackedPoints = insertMarker(currentFrame,trackedPoints(validity, :),'o'); 
%     imshow(showTrackedPoints);
%     
%     %plot the detected fast points in the previous frame
%     subplot(1,2,1);
%     imshow(previousFrame);
%     hold on;
%     plot(FAST);

    sizeTrackedPoints = size(trackedPoints);
    numberTrackedPoints = sizeTrackedPoints(1,1);

    numberValidPoint = 1;
    for j = 1:numberTrackedPoints
        if(validity(j) == 1)
            %Save all ValidPoint in the 3-d Array validPoints: 1d-frameNumber,
            %2d-numberValidPoint, 3d-startPosition and endPosition of the KLT Flow
            validPoints(frameNumber, numberValidPoint, :) = [detectedPoints.Location(j,1), detectedPoints.Location(j,2), trackedPoints(j,1), trackedPoints(j,2)];
            numberValidPoint = numberValidPoint + 1;
        end
    end
    amountValidPoints = numberValidPoint - 1;
    
    %TODO: whether the current frame is a Key Frame
    
%     if() %means it is not a Key Frame
%         previousFrameKey =0;
%     end
    
    
    %plot the tracked point in the current frame and the optical flow line segment
    figure(1)
    showTrackedPoints = insertMarker(currentFrame,trackedPoints(validity, :),'+'); 
    imshow(showTrackedPoints);
    hold on;
    %plot optical flow line segment
    for k = 1:numberValidPoints
        plot([validPoints(frameNumber, k, 1), validPoints(frameNumber, k, 3)], [validPoints(frameNumber, k, 2), validPoints(frameNumber, k, 4)], 'm');
    end

%     for j = 1:numberTrackedPoints
%         if(validity(j) == 1)
%             plot([detectedPoints.Location(j,1), trackedPoints(j,1)], [detectedPoints.Location(j,2), trackedPoints(j,2)], 'm');
%         end
%     end
        
%     annotation('arrow',detectedPoints.Location(1, :), trackedPoints(1, :));

%         figure(3)
%     plot([1,2], [1,2]);

    release(KLTtracker);
    
end

%save 3-d Array validPoints (the position of validPoints in the previous frame and current frame of every image) in a .mat file.
save validPoints.mat validPoints


% objectFrame = image1;
% detectedPoints = FAST;
% initialize(KLTtracker,detectedPoints.Location,objectFrame);
% 
% newFrame = image2;
% [trackedPoints,validity] = KLTtracker(newFrame);
% 
% showTrackedPoints = insertMarker(newFrame,trackedPoints(validity, :),'o');
% figure(2); imshow(showTrackedPoints);
% 
% release(KLTtracker);


% opflow = opticalFlowLK(randn(100,100),randn(100,100));
% plot(opflow,'DecimationFactor',[10 10],'ScaleFactor',10);

% opticFlow = opticalFlowLK('NoiseThreshold',0.009);
% % image1 = rgb2gray(image1);
% flow = estimateFlow(opticFlow,image1); 

% opflow = opticalFlow(image1,image2);
% subplot(1,2,1),imshow(double(Vx),[]);
% subplot(1,2,2),imshow(double(Vy),[]);
