# Discord Server Bumper (Educational Purposes Only)

## Overview
This project demonstrates how to interact with Discord's API to automate server "bumping" for Disboard. **This script is strictly for educational purposes only.** The author is not responsible for any misuse or consequences of using this script. Note that this script violates the Terms of Service (ToS) of both Discord and Disboard. Proceed at your own risk.

---

## Features
- Connects to WiFi and ensures a stable connection.
- Automatically sends bump requests at ~2-hour intervals.
- Implements retry logic for WiFi and HTTP requests to handle failures.
- Generates a custom JSON payload for interacting with the Discord API.
- Uses a user token (not a bot token) for authentication.

---

## Prerequisites
- An ESP32 microcontroller.
- Arduino IDE or equivalent development environment.
- WiFi network credentials.
- A Discord user token (NOT a bot token).

---

## Setup Instructions
1. **Install Dependencies**: Ensure the following Arduino libraries are installed:
   - `WiFi.h`
   - `HTTPClient.h`
   - `ArduinoJson.h`

2. **Configure WiFi Credentials**:
   Replace the placeholders in the code with your WiFi name and password:
   ```cpp
   const char* ssid = "YOUR WIFI NAME";
   const char* password = "YOUR WIFI PASSWORD";
   ```

3. **Set Discord API Details**:
   - Replace `USER TOKEN` with your **Discord user token**:
     ```cpp
     const char* authorization = "USER TOKEN";
     ```
   - Update the `guild_id` and `channel_id` with the corresponding values for your server and channel:
     ```cpp
     payload["guild_id"] = "ADD SERVER ID";
     payload["channel_id"] = "ADD CHANNEL ID";
     ```

4. **Upload the Code**:
   - Use the Arduino IDE to upload the script to your ESP32.

---

## Warnings
- **Educational Use Only**: This script is provided solely for learning purposes. Using it may result in the suspension of your Discord account or server.
- **Responsibility Disclaimer**: The author is not liable for any bans, suspensions, or other consequences resulting from the use of this script.
- **Violations**: Automating requests using a user token is against Discord’s Terms of Service and could lead to permanent account suspension.

---

## How It Works
1. **WiFi Connection**:
   - The script attempts to connect to the WiFi network. If the connection fails, it retries up to 5 times with a 20-second timeout.

2. **Payload Generation**:
   - A custom JSON payload is created to simulate the "bump" command for Disboard.

3. **HTTP Request**:
   - The payload is sent to Discord’s API endpoint (`https://discord.com/api/v9/interactions`) using the `HTTPClient` library.
   - The script handles failed requests with exponential backoff and retries up to 3 times.

4. **Bumping Interval**:
   - The server bump command is executed every ~2 hours.

5. **Error Handling**:
   - The script tracks consecutive failures and pauses for 5 minutes if there are more than 5 consecutive failures.

---

## Customization
- Modify `BUMP_INTERVAL` to adjust the time between bump attempts (default: ~2 hours).
- Update `MAX_WIFI_RETRIES` and `MAX_HTTP_RETRIES` to customize retry logic.

---

## Legal Disclaimer
This script is for **educational purposes only**. The use of this script to automate Discord interactions:
- Violates the Terms of Service of both Discord and Disboard.
- Could result in penalties, including account bans and server suspensions.

By using this code, you agree that you:
1. Understand the risks involved.
2. Take full responsibility for any consequences arising from its use.

---

## License
No license. Use at your own risk.

