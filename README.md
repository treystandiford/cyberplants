# CyberPlants

This repository contains instructions for setting up **ThingSpeak** and **Imgur** integration with a camera-enabled Arduino Uno R4 Wi-Fi.

---

## Requirements Before You Begin

Ensure you have the following ready:
1. **Hardware Assembly**: Follow the steps in the CyberPlants User Manual.
2. **ThingSpeak Account**: [Sign up for free](https://thingspeak.mathworks.com/).
3. **Imgur Account**: [Sign up for free](https://imgur.com/).
4. **Postman**: [Download for free](https://www.postman.com/).
5. **Arduino IDE**: [Download for free](https://www.arduino.cc/en/software).

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

## Uploading Arduino Code

Follow the steps outlined in this guide to set up your Arduino IDE: [Getting Started with Arduino Uno R4 Wi-Fi](https://docs.arduino.cc/tutorials/uno-r4-wifi/r4-wifi-getting-started/).

---
