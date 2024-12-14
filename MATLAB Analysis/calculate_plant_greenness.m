% Replace with your Imgur access token
accessToken = 'PLACE YOUR IMGUR ACCESS TOKEN HERE';

% ThingSpeak channel details
channelID = PLACE YOUR THINGSPEAK CHANNEL ID HERE;          % Replace with your ThingSpeak channel ID
writeAPIKey = 'PLACE YOUR THINGSPEAK WRITE API KEY HERE';  % Replace with your ThingSpeak Write API Key

% Imgur API endpoint to get account images
apiUrl = 'https://api.imgur.com/3/account/me/images';

% Set up HTTP headers with the access token
options = weboptions('HeaderFields', {'Authorization', ['Bearer ' accessToken]}, 'Timeout', 60);

try
    % Fetch the JSON response from the Imgur API
    response = webread(apiUrl, options);
    
    if isfield(response, 'data') && ~isempty(response.data)
        % Find the first image with title starting with "top"
        latestTopImage = [];
        for i = 1:length(response.data)
            if startsWith(response.data(i).title, 'top', 'IgnoreCase', true)
                latestTopImage = response.data(i);
                break;
            end
        end

        if isempty(latestTopImage)
            error('No image found with a title starting with "top".');
        end
        
        % Download the latest image
        latestImageUrl = latestTopImage.link;
        disp(['Found Image URL: ', latestImageUrl]);
        outputFile = 'latest_top_image.jpg'; 
        websave(outputFile, latestImageUrl);

        % Read the image
        rgbImage = imread(outputFile);
        if isempty(rgbImage)
            error('Error: Image not found or unable to read.');
        end

        %------------------------------------------------------------------
        % Calculate GCC Index
        %------------------------------------------------------------------
        red = double(rgbImage(:,:,1));
        green = double(rgbImage(:,:,2));
        blue = double(rgbImage(:,:,3));

        % Smooth channels
        red = imgaussfilt(red, 1);
        green = imgaussfilt(green, 1);
        blue = imgaussfilt(blue, 1);

        % Calculate GCC
        GCC = green ./ (red + green + blue);

        % Smooth GCC
        GCC = imgaussfilt(GCC, 1);

        % Extract plant mask
        plantMask = extractPlantMask(rgbImage);

        % Extract GCC values within the plant mask
        maskedGCC = GCC(plantMask);

        % Compute mean GCC
        meanGCC = mean(maskedGCC, 'omitnan');

        % Print mean GCC
        fprintf('Mean GCC Value: %.4f\n', meanGCC);

        % Send the mean GCC to ThingSpeak Field 2
        try
            thingSpeakWrite(channelID, meanGCC, 'Fields', 2, 'WriteKey', writeAPIKey);
            disp('Mean GCC value successfully written to ThingSpeak Field 2.');
        catch ME
            disp(['Error sending data to ThingSpeak: ', ME.message]);
        end

    else
        error('No images found in your Imgur account.');
    end
catch ME
    disp(['Error: ', ME.message]);
end

% ---------------------------------------------------------------
% Helper Functions
% ---------------------------------------------------------------

function plantMask = extractPlantMask(image)
    % Convert RGB image to LAB color space
    LAB = rgb2lab(image);

    % Extract L-channel and normalize
    L = LAB(:, :, 1);
    L = L / 100;

    % Threshold the L-channel to isolate background
    th = graythresh(L);
    M = L < th;

    % Perform morphological closing to refine mask
    M = imclose(M, strel('disk', 11));

    % Invert the mask to focus on the plant region
    P = ~M;

    % Isolate the largest object (assumed to be the plant)
    P = bwareafilt(P, 1);

    % Fill any holes in the mask
    P = imfill(P, 'holes');

    plate = image;
    plate_lab = LAB;

    % Extract the 'a' channel (2nd channel of LAB) for plant segmentation
    rose = plate_lab(:, :, 2);

    % Normalize the 'a' channel to range [0, 1]
    rose = bindVec(rose);

    % Apply thresholding to segment the plant region
    plant_th = graythresh(rose);
    plantMask = rose < plant_th;
end

function out = bindVec(in)
    % Normalize the input 'in' to the range [0, 1]
    out = (in - min(in(:))) / (max(in(:)) - min(in(:)));
end
