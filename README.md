# cyberplants

This repository contains instructions for setting up ThingSpeak and Imgur integration with camera-enabled Arduino Uno R4 Wi-Fi. 

**Requirements before you begin:**
1) Assembly of hardware outlined in CyberPlants User Manual
2) ThingSpeak account (Free: https://thingspeak.mathworks.com/)
3) Imgur account (Free: https://imgur.com/)
4) Postman (Free Software: https://www.postman.com/)
5) Arduino IDE (Free Software: https://www.arduino.cc/en/software)

**Set Up Imgur Repository:** 

a) Follow this YouTube tutorial from Andy's Tech Tutorials to set up Imgur account and enable API access: https://www.youtube.com/watch?v=anfNgyplDjI&t=212s

b) Write down the Imgur Access Token in a secure place

**Set Up ThingSpeak Channel**

a) Create your free account

b) Create a new channel: Channels>New Channel

c) Enable the first 5 fields:

![image](https://github.com/user-attachments/assets/8e0089ce-1eab-40b2-8fe1-a2d695e3a5bd)

d) Click Channel Sharing. Write down the Channel ID in a secure place.

e) Click API Keys. Write down your Read and Write API Keys in a secure place.

**Set Up ThingSpeak Analyses**: This section will establish the plant area, height, and greenness calculations

a) Click Apps>Matlab Analysis

b) Create a new Matlab Analysis and name it [calculate_plant_height]

c) Open calculate_plant_height.m from this tutorial and paste the code into the Matlab Code section. 

d) Change the [accessToken], [channelID], and [writeAPIKey] variables to represent your own. They are marked with comments "CHANGE HERE".

e) do the same for calculate_plant_area.m and calculate_greenness.m

**Set Up ThingSpeak TimeControl**: This section will automate height, area, and greenness calculations

a) Click Apps>TimeControl>New

b) Name the first one automate_plant_height_calculation

c) Set the frequency to Recurring

d) Set the reccurrece to Minute. Select Every 5 minutes from the drop-down.

e) For Action, select MATLAB Analysis. 

f) For Code to execute, select calculate_plant_height

g) Repeat this process for [automate_plant_area_calculation] and [automate_plant_greenness_calculation]



**Uploading Arduino Code**

a) Read through the following document to set up your Arduino IDE: https://docs.arduino.cc/tutorials/uno-r4-wifi/r4-wifi-getting-started/









