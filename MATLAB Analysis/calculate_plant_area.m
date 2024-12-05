% Complete Script: Imgur Image Retrieval, Leaf Area Calculation, and ThingSpeak Update

% Configuration
accessToken = 'PLACE ACCESS TOKEN HERE'; % Imgur Access Token
channelID = PLACE CHANNEL ID HERE; % ThingSpeak Channel ID
writeAPIKey = 'PLACE WRITE API KEY HERE'; % ThingSpeak Write API Key
distanceToCamera_cm = PLACE CONFIGURED DISTANCE HERE; % Distance from camera to plant (cm)

% Camera specifications
sensorHeight_mm = 2.7; % Sensor height in mm
focalLength_mm = 4.9; % Focal length in mm
sensorHeight_cm = sensorHeight_mm / 10; % Convert sensor height to cm
focalLength_cm = focalLength_mm / 10; % Convert focal length to cm

% Imgur API endpoint
apiUrl = 'https://api.imgur.com/3/account/me/images';
options = weboptions('HeaderFields', {'Authorization', ['Bearer ' accessToken]}, 'Timeout', 60);

try
    % Step 1: Retrieve image
    response = webread(apiUrl, options);
    
    if isfield(response, 'data') && ~isempty(response.data)
        % Search for the most recent image with "top_view" in title
        topViewImage = [];
        for i = 1:length(response.data)
            if startsWith(response.data(i).title, 'top_view')
                topViewImage = response.data(i);
                break;
            end
        end
        
        if ~isempty(topViewImage)
            imageUrl = topViewImage.link;
            disp(['Processing image: ', imageUrl]);

            % Step 1: Download the image
            outputFile = 'top_view_image.jpg';
            websave(outputFile, imageUrl);

            % Step 2: Analyze the plant region
            I = imread(outputFile);
            hsvImage = rgb2hsv(I);

            % Define green color range in HSV
            lowerGreen = [30/360, 0.4, 0.2]; % Adjust based on plant color
            upperGreen = [85/360, 1.0, 1.0];

            % Create a binary mask for green regions
            greenMask = (hsvImage(:,:,1) >= lowerGreen(1)) & (hsvImage(:,:,1) <= upperGreen(1)) & ...
                        (hsvImage(:,:,2) >= lowerGreen(2)) & (hsvImage(:,:,2) <= upperGreen(2)) & ...
                        (hsvImage(:,:,3) >= lowerGreen(3)) & (hsvImage(:,:,3) <= upperGreen(3));
            greenMask = imclose(greenMask, strel('disk', 11));
            greenMask = bwareafilt(greenMask, 1); % Keep the largest green region

            % Step 3: Compute the bounding box
            stats = regionprops(greenMask, 'BoundingBox'); % Get bounding box properties
            boundingBox = stats.BoundingBox; % Bounding box format: [x, y, width, height]

            % Draw the bounding box on the image
            figure;
%             imshow(I);
            hold on;
            rectangle('Position', boundingBox, 'EdgeColor', 'red', 'LineWidth', 2);
            title('Plant with Bounding Box');
            hold off;

            % Step 4: Calculate real-world dimensions and area
            imageHeight_px = size(I, 1);
            imageWidth_px = size(I, 2);

            % Calculate real-world height and width of the imaged area
            H_real_cm = (sensorHeight_cm * distanceToCamera_cm) / focalLength_cm;
            W_real_cm = (sensorHeight_cm * distanceToCamera_cm) / focalLength_cm;

            % Calculate area per pixel
            A_pixel_cm2 = (H_real_cm * W_real_cm) / (imageHeight_px * imageWidth_px);

            % Extract bounding box dimensions
            boxWidth_px = boundingBox(3);
            boxHeight_px = boundingBox(4);

            % Convert bounding box dimensions to real-world dimensions
            boxWidth_cm = boxWidth_px * sqrt(A_pixel_cm2);
            boxHeight_cm = boxHeight_px * sqrt(A_pixel_cm2);

            % Calculate bounding box area in cm²
            boundingBoxArea_cm2 = boxWidth_cm * boxHeight_cm;

            % Step 5: Output results
            fprintf('Bounding Box Width: %.2f cm\n', boxWidth_cm);
            fprintf('Bounding Box Height: %.2f cm\n', boxHeight_cm);
            fprintf('Bounding Box Area: %.2f cm²\n', boundingBoxArea_cm2);

            % Send the calculated area to ThingSpeak Field 8
            try
                thingSpeakWrite(channelID, boundingBoxArea_cm2, 'Fields', 5, 'WriteKey', writeAPIKey);
                disp('Bounding box area successfully written to ThingSpeak Field 8.');
            catch ME
                disp(['Error sending data to ThingSpeak: ', ME.message]);
            end

        else
            error('No "top_view" images found in your Imgur account.');
        end
    else
        error('No images found in your Imgur account.');
    end
catch ME
    disp(['Error: ', ME.message]);
end
