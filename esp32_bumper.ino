#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "YOUR WIFI NAME";
const char* password = "YOUR WIFI PASSWORD";

// Discord API details
const char* url = "https://discord.com/api/v9/interactions";
const char* authorization = "USER TOKEN"; // Can't be BOT token, must be user token.

// Don't forget to change SERVER and CHANNEL ID!!!

// Constants for timing and retries
const unsigned long WIFI_TIMEOUT = 20000;  // 20 seconds timeout for WiFi connection
const int MAX_WIFI_RETRIES = 5;           // Maximum number of WiFi reconnection attempts
const unsigned long HTTP_TIMEOUT = 10000;  // 10 seconds timeout for HTTP requests
const unsigned long BUMP_INTERVAL = 7210000; // 2 hours and 10 seconds between bumps
const int MAX_HTTP_RETRIES = 3;           // Maximum number of HTTP request retries

// Global variables for state management
unsigned long lastBumpAttempt = 0;
int consecutiveFailures = 0;
bool isFirstBoot = true;

bool connectWiFi() {
  int retryCount = 0;
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < WIFI_TIMEOUT) {
    delay(500);
    Serial.print(".");
    retryCount++;
    
    if (retryCount >= MAX_WIFI_RETRIES) {
      Serial.println("\nFailed to connect to WiFi after maximum retries");
      return false;
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  
  return false;
}

bool createJsonPayload(String& payloadStr) {
  // Increase JSON document size to 4KB
  StaticJsonDocument<4096> payload;
  
  // Debug memory usage
  Serial.printf("JSON Document Capacity: %d bytes\n", payload.capacity());
  Serial.printf("Initial Memory Usage: %d bytes\n", payload.memoryUsage());
  
  // Check if we have enough memory
  if (payload.capacity() - payload.memoryUsage() < 1024) {
    Serial.println("Not enough memory for JSON document");
    return false;
  }

  // Create JSON structure with error checking
  DeserializationError error = deserializeJson(payload, "{}");
  if (error) {
    Serial.print("JSON initialization failed: ");
    Serial.println(error.c_str());
    return false;
  }

  Serial.println("Creating JSON structure...");

  // Basic fields
  payload["type"] = 2;
  payload["application_id"] = "302050872383242240";
  payload["guild_id"] = "ADD SERVER ID";
  payload["channel_id"] = "ADD CHANNEL ID";
  payload["session_id"] = "0";

  // Create data object
  JsonObject data = payload.createNestedObject("data");
  if (data.isNull()) {
    Serial.println("Failed to create data object");
    return false;
  }

  Serial.println("Created data object successfully");

  data["version"] = "1051151064008769576";
  data["id"] = "947088344167366698";
  data["name"] = "bump";
  data["type"] = 1;
  
  // Create empty options array
  JsonArray options = data.createNestedArray("options");
  if (options.isNull()) {
    Serial.println("Failed to create options array");
    return false;
  }

  // Create application command object
  JsonObject appCommand = data.createNestedObject("application_command");
  if (appCommand.isNull()) {
    Serial.println("Failed to create appCommand object");
    return false;
  }

  Serial.println("Created appCommand object successfully");

  appCommand["id"] = "947088344167366698";
  appCommand["type"] = 1;
  appCommand["application_id"] = "302050872383242240";
  appCommand["version"] = "1051151064008769576";
  appCommand["name"] = "bump";
  appCommand["description"] = "Pushes your server to the top of all your server's tags and the front page";
  appCommand["description_default"] = "Pushes your server to the top of all your server's tags and the front page";
  appCommand["dm_permission"] = true;
  
  JsonArray integrationTypes = appCommand.createNestedArray("integration_types");
  if (integrationTypes.isNull()) {
    Serial.println("Failed to create integrationTypes array");
    return false;
  }
  integrationTypes.add(0);
  
  appCommand["global_popularity_rank"] = 1;
  JsonArray appOptions = appCommand.createNestedArray("options");
  if (appOptions.isNull()) {
    Serial.println("Failed to create appCommand options array");
    return false;
  }
  
  appCommand["description_localized"] = "Bump this server.";
  appCommand["name_localized"] = "bump";

  // Generate random nonce
  char nonce[20];
  sprintf(nonce, "%llu", random(1000000000, 9999999999));
  payload["nonce"] = nonce;
  payload["analytics_location"] = "slash_ui";

  // Debug final memory usage
  Serial.printf("Final Memory Usage: %d bytes\n", payload.memoryUsage());
  Serial.printf("Remaining Memory: %d bytes\n", payload.capacity() - payload.memoryUsage());

  // Serialize JSON to string with error checking
  size_t jsonSize = serializeJson(payload, payloadStr);
  if (jsonSize == 0) {
    Serial.println("Failed to serialize JSON");
    return false;
  }

  Serial.printf("JSON serialized successfully. Size: %d bytes\n", jsonSize);
  Serial.println("JSON Payload: " + payloadStr);

  return true;
}

bool bump_server() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting reconnection...");
    if (!connectWiFi()) {
      consecutiveFailures++;
      return false;
    }
  }

  String payloadStr;
  if (!createJsonPayload(payloadStr)) {
    Serial.println("Failed to create JSON payload");
    consecutiveFailures++;
    return false;
  }

  HTTPClient http;
  http.begin(url);
  http.setTimeout(HTTP_TIMEOUT);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", authorization);

  int retryCount = 0;
  int httpResponseCode;
  
  do {
    httpResponseCode = http.POST(payloadStr);
    
    if (httpResponseCode == 204) {
      Serial.println("Server bumped successfully!");
      consecutiveFailures = 0;
      lastBumpAttempt = millis();
      http.end();
      return true;
    }
    
    Serial.printf("HTTP request failed with code %d (attempt %d/%d)\n", 
                 httpResponseCode, retryCount + 1, MAX_HTTP_RETRIES);
    
    if (httpResponseCode > 0) {
      Serial.println("Response: " + http.getString());
    }
    
    delay(1000 * (retryCount + 1));  // Exponential backoff
    retryCount++;
    
  } while (retryCount < MAX_HTTP_RETRIES);

  consecutiveFailures++;
  http.end();
  return false;
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));  // Initialize random number generator
  
  if (!connectWiFi()) {
    Serial.println("Initial WiFi connection failed. Will retry in loop...");
    return;
  }
  
  isFirstBoot = true;
}

void loop() {
  if (isFirstBoot || (millis() - lastBumpAttempt >= BUMP_INTERVAL)) {
    if (bump_server()) {
      Serial.println("Bump successful, waiting for next interval...");
      isFirstBoot = false;
    } else {
      Serial.printf("Bump failed. Consecutive failures: %d\n", consecutiveFailures);
      
      if (consecutiveFailures >= 5) {
        Serial.println("Too many consecutive failures. Waiting 5 minutes before retry...");
        delay(300000);  // 5 minutes
        consecutiveFailures = 0;
      } else {
        delay(60000);  // Wait 1 minute before retry on normal failures
      }
    }
  }
  
  delay(1000);
}
