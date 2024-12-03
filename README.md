# CyberPlants Automated Hydroponics Monitoring and Control System

This repository contains instructions for setting up **ThingSpeak** and **Imgur** integration and system assembly for a circuit with an Arduino Uno R4 Wi-Fi and two ArduCam camera modules.

---

## Hardware Setup

Ensure you have the following hardware ready:
1. **[Arduino Uno R4 Wi-Fi](https://store.arduino.cc/products/uno-r4-wifi)**  
2. **[ArduCam Mini 2MP Plus](https://www.arducam.com/product/arducam-2mp-spi-camera-b0067-arduino/)** (2x)
3. **[Jumper Wires](https://www.amazon.com/EDGELEC-Breadboard-Optional-Assorted-Multicolored/dp/B07GD3KDG9?dib=eyJ2IjoiMSJ9.ARCf8Zjxb1rf-cirLAcgSPqJDTpLsoPXsZMKm5Ox2vFoC0zj9Op4AOOnD0R_g79adQTcAKbplwu0Yz5ffr8vyGDxRPR408nqnbi-_Y-65AxZ3yPW7j-iT4ZOHKgx1XnVW1U-N25yIy39aDcTtyjQvUyWN0mckWizq8ArQeIoD--GEYd8tubYrjacI9TcJNWtGmPSvZEsPeICrbqgxZRlI69H1pDyuKlv_XSD3JqVFP0.KZ7-zTxqR-EtHHiuj_oSUHB2Prq7_L0vrnWHaJCn5O8&dib_tag=se&keywords=long%2Bjumper%2Bwires&qid=1733174193&sr=8-3&th=1)**
   - One set of long (50cm) wire and one set of short (10cm) wires
5. **[Breadboard](https://www.amazon.com/breadboard/s?k=breadboard)**  

6. **Camera and Arduino Stand**: Manufactured by the team at CyberPlants

---

##  Circuit Diagram

1. Connect the camera modules to the Arduino Uno R4 Wi-fi as shown below:

![image](https://github.com/user-attachments/assets/b119f0be-b8dc-4c37-8591-9a2411cdd2c9)

---

## Software Setup

Ensure you have the following ready:
1. **[Hardware Assembly](#)**: Follow the steps shown above in **Hardware Setup Section**
   - Plug the Arduino Uno R4 Wi-Fi into your laptop.
3. **[ThingSpeak Account](https://thingspeak.mathworks.com/)**: Sign up for free.  
4. **[Imgur Account](https://imgur.com/)**: Sign up for free.  
5. **[Postman](https://www.postman.com/)**: Download for free.  
6. **[Arduino IDE](https://www.arduino.cc/en/software)**: Download for free.  


**Note:** 

Changes that need to be made to the provided code are marked with "PLACE". Search through each file and replace necessary values for each file. 

---

## Set Up Imgur Repository

1. Follow this YouTube tutorial from Andy's Tech Tutorials to set up your Imgur account and enable API access: [Imgur API Setup Tutorial](https://www.youtube.com/watch?v=anfNgyplDjI&t=212s).
2. Write down your **Imgur Access Token** in a secure place.

---

## Set Up ThingSpeak Channel

1. Create a free ThingSpeak account.
2. **Create a New Channel**:
   - Navigate to: `Channels > New Channel`.
   - Enable the first five fields.
     ![image](https://github.com/user-attachments/assets/8e0089ce-1eab-40b2-8fe1-a2d695e3a5bd)
3. **Record Channel Details**:
   - Go to `Channel Sharing` and write down your **Channel ID** in a secure place.
   - Go to `API Keys` and write down your **Read** and **Write API Keys** in a secure place.

---

## Set Up ThingSpeak Analyses

This section establishes the calculations for plant area, height, and greenness.

1. Navigate to `Apps > Matlab Analysis`.
2. Create a new Matlab Analysis and name it: **calculate_plant_height**.
3. Open the **calculate_plant_height.m** file from this tutorial, then paste the code into the Matlab Code section.
4. Update the following variables in the code to reflect your setup (marked with "CHANGE HERE"):
   - **[accessToken]**
   - **[channelID]**
   - **[writeAPIKey]**
   - **[distance_to_plant]**
5. Repeat the process for:
   - **calculate_plant_area.m**
   - **calculate_greenness.m**

---

## Set Up ThingSpeak TimeControl

This section automates the height, area, and greenness calculations.

1. Navigate to `Apps > TimeControl > New`.
2. Create an automation named: **automate_plant_height_calculation**.
   - Set **Frequency** to Recurring.
   - Set **Recurrence** to **Every 5 minutes**.
   - Select **MATLAB Analysis** for Action.
   - Choose the **calculate_plant_height** analysis for execution.
3. Repeat this process for:
   - **automate_plant_area_calculation**
   - **automate_plant_greenness_calculation**

---

## Set Up ThingSpeak Plugins

This section enables the display of plant images and camera control features.

1. Navigate to `Apps > Plugins > New`.
2. Create a Plugin named: **display_side_view**.
   - Navigate to the `Plugins` folder in this directory, and enter the `display_side_view` folder.
   - Replace the **HTML** and **CSS** code in the corresponding sections.
   - Replace the **JavaScript** code, and update the `[accessToken]` variable with your **Imgur Access Token**.
   - Select "Show on Channel(s)" and choose your channel.
   - Save the Plugin.
3. Repeat this process for:
   - **display_top_view**
   - **camera_frequency_widget**: note: In the JavaScript section, update the `[writeApiKey]` variable to match yours.
4. Go back to the Channel Dashboard
   - Find the plugins that you created, and edit each field with appropriate titles

---

## Uploading Arduino Code

1. Follow the steps outlined in this guide to set up your Arduino IDE: [Getting Started with Arduino Uno R4 Wi-Fi](https://docs.arduino.cc/tutorials/uno-r4-wifi/r4-wifi-getting-started/).
2. Download **cyberplants_arduinocode.ino** from the Arduino Code folder in this repository.
3. Open the file in the Arduino IDE:
   - Update the Wi-Fi name and password in the credentials section.
   - Replace **[access_token]** with your Imgur Access Token.
   - Update the **read_api_key**, **write_api_key**, and **channel_id** with your ThingSpeak details.
4. Upload the code to the Arduino.

---

## Test Your System!

1. Navigate to your ThingSpeak Dashboard and click the "Manual Image Capture" button.
   - An image will be taken and uploaded.
   - Refresh the page to view the images.
   - Analytical data updates every 5 minutes (as limited by ThingSpeak).
2. Set the Timer Frequency to your desired frequency:
   - The cameras will capture an image, and the analytical data will update at this rate.

---
