#include <Wire.h>        // I2C for camera
#include <SPI.h>         // SPI for camera
#include <ArduCAM.h>     // Camera library
#include <WiFiS3.h>      // Wi-Fi library
#include <WiFiSSLClient.h> // HTTPS client
#include "memorysaver.h" // Camera configuration


// Wi-Fi credentials
const char* ssid = "PLACE YOUR WIFI NETWORK NAME HERE"; 
const char* password = "PLACE YOUR WIFI PASSWORD HERE";    

// Imgur API details
const char* imgur_host = "api.imgur.com";
const int https_port = 443;
const char* access_token = "PLACE YOUR IMGUR ACCESS TOKEN HERE"; 

// ThingSpeak configuration
const char* thingspeak_host = "api.thingspeak.com";
const String read_api_key = "PLACE YOUR THINGSPEAK READ API KEY HERE";
const String write_api_key = "PLACE YOUR THINGSPEAK WRITE API KEY HERE";
const String channel_id = "PLACE YOUR THINGSPEAK CHANNEL ID HERE";


// Camera configuration
const int CAMERA1_CS = 7; // Chip Select for camera 1
const int CAMERA2_CS = 8; // Chip Select for camera 2
ArduCAM camera1(OV2640, CAMERA1_CS);
ArduCAM camera2(OV2640, CAMERA2_CS);

// Time tracking for periodic image capture
unsigned long lastCaptureTime = 0;
unsigned long captureInterval = 60 * 60 * 1000; // Default: 60 minutes (in milliseconds)

void setup() {
    Serial.begin(115200);
    delay(100);

    // Initialize I2C and SPI
    Wire.begin();
    SPI.begin();

    // Connect to Wi-Fi
    connectToWiFi();

    // Initialize both cameras
    initCamera(CAMERA1_CS);
    initCamera(CAMERA2_CS);

    // Capture and upload images from both cameras with custom titles
    // captureAndUploadImage(camera1, CAMERA1_CS, "side_view");
}

// Global variable to track the previous capture frequency
int previousCaptureFrequency = -1;

void loop() {
    // Ensure Wi-Fi is connected
    if (WiFi.status() != WL_CONNECTED) {
        connectToWiFi();
    }

    // Variables to hold ThingSpeak values
    int captureFrequency = 0;
    bool manualTrigger = false;

    // Check ThingSpeak fields and reset manual trigger if detected
    checkThingSpeakFields(captureFrequency, manualTrigger);

    // Update the capture interval only if the frequency has changed
    if (captureFrequency != previousCaptureFrequency) {
        setCaptureTimer(captureFrequency);
        previousCaptureFrequency = captureFrequency; // Update the stored frequency
        Serial.println("Capture frequency updated. Timer adjusted.");
    } else {
        Serial.println("Capture frequency unchanged. Timer continues.");
    }

    // Handle manual trigger
    if (manualTrigger) {
        Serial.println("Manual trigger: Capturing images...");
        captureAndUploadImage(camera1, CAMERA1_CS, "side_view");
        captureAndUploadImage(camera2, CAMERA2_CS, "top_view");
    }

    // Handle periodic image capture
    unsigned long currentMillis = millis();
    if (currentMillis - lastCaptureTime >= captureInterval) {
        Serial.println("Time-based trigger: Capturing images...");
        captureAndUploadImage(camera1, CAMERA1_CS, "side_view");
        captureAndUploadImage(camera2, CAMERA2_CS, "top_view");
        lastCaptureTime = currentMillis;
    }

    // Add a small delay to reduce API polling frequency
    delay(30000);
}


void connectToWiFi() {
    Serial.print("Connecting to Wi-Fi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi");
}

void initCamera(int csPin) {
    ArduCAM* camera = (csPin == CAMERA1_CS) ? &camera1 : &camera2;

    camera->CS_LOW();
    camera->write_reg(0x07, 0x80); // Reset the camera
    delay(100);
    camera->write_reg(0x07, 0x00); // Exit reset
    delay(100);

    // Test SPI communication
    while (true) {
        camera->write_reg(ARDUCHIP_TEST1, 0x55);
        uint8_t temp = camera->read_reg(ARDUCHIP_TEST1);
        if (temp != 0x55) {
            Serial.print("Camera SPI interface error on CS pin ");
            Serial.println(csPin);
            delay(1000);
        } else {
            Serial.print("Camera SPI interface OK on CS pin ");
            Serial.println(csPin);
            break;
        }
    }

    // Initialize camera settings
    camera->set_format(JPEG);
    camera->InitCAM();
    camera->OV2640_set_JPEG_size(OV2640_1600x1200); // Set resolution (modify as needed)
    delay(1000);
    Serial.print("Camera initialized on CS pin ");
    Serial.println(csPin);
    camera->CS_HIGH();
}

void captureAndUploadImage(ArduCAM& camera, int csPin, const String& imageTitle) {
    // Select the correct camera
    camera.CS_LOW();

    // Capture the image
    camera.clear_fifo_flag();
    camera.start_capture();

    while (!camera.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {
        delay(10);
    }
    Serial.println("Image captured.");

    // Connect to Imgur
    WiFiSSLClient client;
    if (!client.connect(imgur_host, https_port)) {
        Serial.println("Connection to Imgur failed!");
        camera.CS_HIGH();
        return;
    }
    Serial.println("Connected to Imgur.");

    // Prepare multipart form-data request
    String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    String lineEnd = "\r\n";
    String twoHyphens = "--";

    // Send HTTP headers
    client.println("POST /3/image HTTP/1.1");
    client.println("Host: api.imgur.com");
    client.print("Authorization: Bearer ");
    client.println(access_token);
    client.println("User-Agent: Arduino/1.0");
    client.print("Content-Type: multipart/form-data; boundary=");
    client.println(boundary);

    // Calculate exact content length
    uint32_t imageSize = camera.read_fifo_length();
    if (imageSize == 0 || imageSize >= MAX_FIFO_SIZE) {
        Serial.println("Invalid image size. Aborting.");
        camera.CS_HIGH();
        return;
    }
    size_t contentLength = calculateExactContentLength(boundary, imageSize, imageTitle);
    client.print("Content-Length: ");
    client.println(contentLength);
    client.println(); // End of headers

    // Start boundary
    client.print(twoHyphens + boundary + lineEnd);

    // Content-Disposition for title
    client.print("Content-Disposition: form-data; name=\"title\"" + lineEnd + lineEnd);
    client.print(imageTitle + lineEnd);

    // Boundary before image data
    client.print(twoHyphens + boundary + lineEnd);

    // Content-Disposition for image
    client.print("Content-Disposition: form-data; name=\"image\"; filename=\"image.jpg\"" + lineEnd);
    client.print("Content-Type: image/jpeg" + lineEnd + lineEnd);

    // Stream image data in chunks
    sendImageChunks(client, camera, imageSize);

    // End boundary
    client.print(lineEnd + twoHyphens + boundary + twoHyphens + lineEnd);

    // Read and print the response
    readResponse(client);
    client.stop();

    // Deselect the camera
    camera.CS_HIGH();
}




size_t calculateExactContentLength(String boundary, uint32_t imageSize, const String& imageTitle) {
    String lineEnd = "\r\n";
    String twoHyphens = "--";
    size_t length = 0;

    // Start boundary
    length += twoHyphens.length() + boundary.length() + lineEnd.length();

    // Headers for 'title' field
    String contentDispositionTitle = "Content-Disposition: form-data; name=\"title\"";
    length += contentDispositionTitle.length() + lineEnd.length() + lineEnd.length();

    // 'title' value
    length += imageTitle.length() + lineEnd.length();

    // Boundary before image data
    length += twoHyphens.length() + boundary.length() + lineEnd.length();

    // Headers for 'image' field
    String contentDispositionImage = "Content-Disposition: form-data; name=\"image\"; filename=\"image.jpg\"";
    String contentType = "Content-Type: image/jpeg";
    length += contentDispositionImage.length() + lineEnd.length();
    length += contentType.length() + lineEnd.length() + lineEnd.length();

    // Image data
    length += imageSize;

    // End boundary
    length += lineEnd.length();
    length += twoHyphens.length() + boundary.length() + twoHyphens.length() + lineEnd.length();

    return length;
}


void sendImageChunks(WiFiSSLClient& client, ArduCAM& camera, uint32_t imageSize) {
    camera.CS_LOW();
    camera.set_fifo_burst();

    const size_t chunkSize = 1024; // Adjust chunk size if needed
    uint8_t buffer[chunkSize];
    uint32_t bytesRead = 0;
    uint32_t totalSize = imageSize;
    unsigned long lastDotTime = millis();

    while (imageSize > 0) {
        size_t toRead = min(chunkSize, imageSize);
        for (size_t i = 0; i < toRead; i++) {
            buffer[i] = SPI.transfer(0x00);
        }
        client.write(buffer, toRead);
        imageSize -= toRead;
        bytesRead += toRead;

        // Print "." every second
        if (millis() - lastDotTime >= 1000) {
            Serial.print(".");
            lastDotTime = millis();
        }
    }

    camera.CS_HIGH();
    Serial.println(); // Move to a new line after all dots
    Serial.print("Image upload completed. Total bytes uploaded: ");
    Serial.println(totalSize);
}


void readResponse(WiFiSSLClient& client) {
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            break;
        }
    }

    String response = client.readString();
    Serial.println("Response:");
    Serial.println(response);
}

void checkThingSpeakFields(int& captureFrequency, bool& manualTrigger) {
    WiFiSSLClient client;

    // Connect to ThingSpeak
    if (!client.connect(thingspeak_host, https_port)) {
        Serial.println("Connection to ThingSpeak failed!");
        return;
    }

    // Request the latest data from the channel
    String url = "/channels/" + channel_id + "/feeds/last.json?api_key=" + read_api_key;
    client.println("GET " + url + " HTTP/1.1");
    client.println("Host: " + String(thingspeak_host));
    client.println("Connection: close");
    client.println();

    // Wait for the response
    while (client.connected() && !client.available()) {
        delay(10);
    }

    String response = "";
    while (client.available()) {
        response += char(client.read());
    }
    client.stop();

    // Parse field 1 for capture frequency (in minutes)
    String field1Value = parseThingSpeakField(response, "field1");
    if (field1Value.length() > 0) {
        captureFrequency = field1Value.toInt();
    }

    // Parse field 4 for manual trigger
    String field4Value = parseThingSpeakField(response, "field4");
    manualTrigger = (field4Value == "1");

    // Print the results for debugging
    Serial.print("Capture frequency: ");
    Serial.print(captureFrequency);
    Serial.print(" minutes, Manual trigger: ");
    Serial.println(manualTrigger ? "ON" : "OFF");

    // Reset field 4 to 0 if manual trigger was detected
    if (manualTrigger) {
        resetThingSpeakField(4, 0);
    }
}


void setCaptureTimer(int captureFrequency) {
    if (captureFrequency > 0) {
        captureInterval = captureFrequency * 60 * 1000; // Convert minutes to milliseconds
        Serial.print("Capture timer updated to ");
        Serial.print(captureFrequency);
        Serial.println(" minutes.");
    }
}

String parseThingSpeakField(const String& response, const String& fieldName) {
    String searchStr = "\"" + fieldName + "\":";
    int startIndex = response.indexOf(searchStr);
    if (startIndex != -1) {
        startIndex += searchStr.length();
        int endIndex = response.indexOf(",", startIndex);
        if (endIndex == -1) {
            endIndex = response.indexOf("}", startIndex);
        }
        String value = response.substring(startIndex, endIndex);
        value.trim(); // Remove any whitespace
        value.replace("\"", ""); // Remove quotes if present
        return value;
    }
    return ""; // Return an empty string if the field is not found
}


void resetThingSpeakField(int fieldToReset, int resetValue) {
    WiFiSSLClient client;

    // Step 1: Fetch current field values
    if (!client.connect(thingspeak_host, https_port)) {
        Serial.println("Failed to connect to ThingSpeak to fetch fields.");
        return;
    }

    String url = "/channels/" + channel_id + "/feeds/last.json?api_key=" + read_api_key;
    client.println("GET " + url + " HTTP/1.1");
    client.println("Host: " + String(thingspeak_host));
    client.println("Connection: close");
    client.println();

    // Wait for response
    while (client.connected() && !client.available()) {
        delay(10);
    }

    String response = "";
    while (client.available()) {
        response += char(client.read());
    }
    client.stop();

    // Parse current field values
    String field1Value = parseThingSpeakField(response, "field1");
    String field2Value = parseThingSpeakField(response, "field2"); // Add more fields as needed
    String field3Value = parseThingSpeakField(response, "field3");
    String field4Value = parseThingSpeakField(response, "field4");

    // Step 2: Prepare updated field values
    if (fieldToReset == 1) field1Value = String(resetValue);
    if (fieldToReset == 2) field2Value = String(resetValue);
    if (fieldToReset == 3) field3Value = String(resetValue);
    if (fieldToReset == 4) field4Value = String(resetValue);

    // Step 3: Build POST data with all fields
    String postData = "api_key=" + write_api_key +
                      "&field1=" + field1Value +
                      "&field2=" + field2Value +
                      "&field3=" + field3Value +
                      "&field4=" + field4Value;

    // Step 4: Send POST request
    if (!client.connect(thingspeak_host, https_port)) {
        Serial.println("Failed to connect to ThingSpeak to reset fields.");
        return;
    }

    String postUrl = "/update.json";
    client.println("POST " + postUrl + " HTTP/1.1");
    client.println("Host: " + String(thingspeak_host));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println("Connection: close");
    client.println();
    client.print(postData);

    // Wait for response
    while (client.connected() && !client.available()) {
        delay(10);
    }

    while (client.available()) {
        client.read(); // Read and discard response
    }
    client.stop();

    Serial.print("Field ");
    Serial.print(fieldToReset);
    Serial.print(" reset to value ");
    Serial.println(resetValue);
}




