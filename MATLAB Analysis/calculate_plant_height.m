% Replace with your Imgur access token
accessToken = 'PLACE YOUR ACCESS TOKEN HERE';

% ThingSpeak channel details
channelID = PLACE YOUR CHANNEL ID HERE;  % Replace with your ThingSpeak channel ID
writeAPIKey = 'PLACE YOUR API WRITE KEY HERE';  % Replace with your ThingSpeak Write API Key

% Camera specifications
sensorHeight_mm = 2.7;        % Sensor height in mm (update with your camera's sensor height)
focalLength_mm = 4.9;          % Focal length in mm
cameraDistance_cm = PLACE YOUR DISTANCE TO PLANT HERE;        % Distance to plant in cm   

% Imgur API endpoint to get account images
apiUrl = 'https://api.imgur.com/3/account/me/images';

% Set up HTTP headers with the access token
options = weboptions('HeaderFields', {'Authorization', ['Bearer ' accessToken]}, 'Timeout', 60);

try
    % Fetch the JSON response from the Imgur API
    response = webread(apiUrl, options);
    
    % Parse the response to get the latest image URL
    if isfield(response, 'data') && ~isempty(response.data)
        % Check if the latest image title starts with "side_view"
        latestImage = response.data(1);
        if startsWith(latestImage.title, 'side_view')
            latestImageUrl = latestImage.link; % Get the link of the first (latest) image
            disp(['Latest Image URL: ', latestImageUrl]);
            
            % Download the latest image
            outputFile = 'latest_image.jpg'; % Local filename
            websave(outputFile, latestImageUrl);

            % Read the image into MATLAB
            imageData = imread(outputFile);

            % Check if image is loaded correctly
            if isempty(imageData)
                error('Error: Image not found or unable to read.');
            end

            % Get image resolution
            [imageHeight, imageWidth, ~] = size(imageData);
            current_resolution = [imageWidth, imageHeight]; % [width, height]
            fprintf('Image size: %d x %d pixels\n', imageWidth, imageHeight);

            % Convert image to HSV color space
            hsvImage = rgb2hsv(imageData);

            % Define green color range in HSV (adjust as needed)
            % Convert OpenCV HSV ranges to MATLAB HSV ranges
            % OpenCV H ranges from 0 to 179, S and V from 0 to 255
            % MATLAB H ranges from 0 to 1, S and V from 0 to 1

            % OpenCV lower_green = [35, 50, 50]
            % OpenCV upper_green = [85, 255, 255]
            % Convert to MATLAB ranges
            lowerGreen = [35/179, 50/255, 50/255]; % [H, S, V]
            upperGreen = [85/179, 1, 1];

            % Create mask for green regions
            mask = (hsvImage(:,:,1) >= lowerGreen(1)) & (hsvImage(:,:,1) <= upperGreen(1)) & ...
                   (hsvImage(:,:,2) >= lowerGreen(2)) & (hsvImage(:,:,2) <= upperGreen(2)) & ...
                   (hsvImage(:,:,3) >= lowerGreen(3)) & (hsvImage(:,:,3) <= upperGreen(3));

            % Find contours of the green regions
            % In MATLAB, use bwconncomp to find connected components
            cc = bwconncomp(mask);

            if cc.NumObjects == 0
                error('No green contours found in the image.');
            end

            % Find the tallest contour (likely the plant)
            stats = regionprops(cc, 'BoundingBox');
            max_height = 0;
            for i = 1:length(stats)
                bb = stats(i).BoundingBox;
                height = bb(4); % BoundingBox format: [x, y, width, height]
                if height > max_height
                    max_height = height;
                    tallest_bb = bb;
                end
            end

            % Get the bounding box coordinates
            x = tallest_bb(1);
            y = tallest_bb(2);
            w = tallest_bb(3);
            h = tallest_bb(4);

            % Debugging: Check the pixel height
            fprintf('Processing image:\n');
            fprintf('  Pixel height of the plant (measured): %.2f pixels\n', h);

            % Calculate physical height per pixel
            pixel_height_mm = sensorHeight_mm / current_resolution(2); % current_resolution(2) is image height in pixels
            % Calculate image height on the sensor in mm
            image_height_mm = h * pixel_height_mm;

            % Convert camera distance to mm
            camera_distance_mm = cameraDistance_cm * 10;  % 1 cm = 10 mm

            % Calculate real-world object height in mm
            real_height_mm = (image_height_mm * camera_distance_mm) / focalLength_mm;
            real_height_cm = real_height_mm / 10;  % Convert mm to cm

            % Debugging: Print intermediate values
            fprintf('  Image height on sensor: %.6f mm\n', image_height_mm);
            fprintf('  Real height: %.2f cm\n', real_height_cm);

            % Send the calculated height to ThingSpeak Field 3
            thingSpeakWrite(channelID, 'Fields', 3, 'Values', real_height_cm, 'WriteKey', writeAPIKey);
            disp('Plant height successfully written to ThingSpeak Field 2.');
        else
            disp('No image found with the specified title prefix.');
        end
    else
        error('No images found in your Imgur account.');
    end
catch ME
    disp(['Error: ', ME.message]);
end