#include <Arduino.h>
#include "WiFiManager.h"
#include <Preferences.h>

// Custom parameters for the WiFi Manager
WiFiManagerParameter* customMqttServer = nullptr;
WiFiManagerParameter* customMqttPort = nullptr;
WiFiManagerParameter* customDeviceName = nullptr;
WiFiManagerParameter* customThemeColor = nullptr;
WiFiManagerParameter* customUpdateInterval = nullptr;

// Preferences for persistent storage
Preferences preferences;

// Callback: Called when the device enters configuration mode (captive portal)
void configModeCallback(WiFiManager* wm) {
  Serial.println("Entered configuration mode.");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  
  // Blink LED rapidly to indicate configuration mode
  // You can add LED blinking code here if your board has an LED
}

// Callback: Called when the configuration is saved after a successful connection
void saveConfigCallback() {
  Serial.println("Configuration saved.");
  
  // Save custom parameters to persistent storage
  if (customMqttServer && customMqttPort && customDeviceName && customThemeColor && customUpdateInterval) {
    Serial.println("Saving custom parameters to preferences...");
    
    // Open preferences with namespace "modernwifi"
    preferences.begin("modernwifi", false);
    
    // Save all parameters
    preferences.putString("mqtt_server", customMqttServer->getValue());
    preferences.putString("mqtt_port", customMqttPort->getValue());
    preferences.putString("device_name", customDeviceName->getValue());
    preferences.putString("theme_color", customThemeColor->getValue());
    preferences.putString("update_interval", customUpdateInterval->getValue());
    
    // Close preferences
    preferences.end();
    
    Serial.println("Parameters saved successfully!");
    
    // Log the saved values
    Serial.println("Custom parameters:");
    Serial.print("MQTT Server: ");
    Serial.println(customMqttServer->getValue());
    Serial.print("MQTT Port: ");
    Serial.println(customMqttPort->getValue());
    Serial.print("Device Name: ");
    Serial.println(customDeviceName->getValue());
    Serial.print("Theme Color: ");
    Serial.println(customThemeColor->getValue());
    Serial.print("Update Interval: ");
    Serial.println(customUpdateInterval->getValue());
  }
}

// Callback: Called when the configuration portal times out
void configTimeoutCallback() {
  Serial.println("Configuration portal timed out.");
  
  // Check if we have saved credentials
  if (WiFi.SSID().length() > 0) {
    Serial.println("Saved credentials exist. Attempting to reconnect...");
    WiFi.reconnect();
  } else {
    Serial.println("No saved credentials. Restarting device...");
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    ESP.restart();
#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    ESP.restart();
#elif defined(ARDUINO_ARCH_RP2040)
    // RP2040 reset method
    watchdog_enable(1, 1);
    while(1); // Wait for watchdog to reset
#else
    // Generic reset fallback - most Arduino boards support this
    asm volatile ("jmp 0");
#endif
  }
}

// Global WiFiManager configuration and instance
WiFiManagerConfig config;
WiFiManager wifiManager(config);

void setup() {
  // Start Serial communication
  Serial.begin(115200);
  delay(1000);  // Allow time for Serial to initialize
  
  Serial.println("\n\n--- ModernWifi Starting ---");
  Serial.println("Firmware Version: 1.1.0");

  // Initialize preferences
  preferences.begin("modernwifi", true); // Read-only mode for now

  // Set WiFi mode to Station mode
  WiFi.mode(WIFI_STA);

  // Configure WiFiManager
  config.connectTimeout = 15000;      // 15 seconds to connect to saved WiFi
  config.configPortalTimeout = 180000; // 3 minutes for configuration portal
  config.httpPort = 80;               // Web server port
  config.autoReconnect = true;        // Auto reconnect if connection is lost

  // Configure WiFiManager debug output and callbacks
  wifiManager.setDebugOutput(true, Serial);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeoutCallback(configTimeoutCallback);

  // Initialize the WiFiManager
  wifiManager.begin();

  // Load saved values from preferences
  String savedMqttServer = preferences.getString("mqtt_server", "mqtt.example.com");
  String savedMqttPort = preferences.getString("mqtt_port", "1883");
  String savedDeviceName = preferences.getString("device_name", "esp32-device");
  String savedThemeColor = preferences.getString("theme_color", "#2196F3");
  String savedUpdateInterval = preferences.getString("update_interval", "30");
  
  // Close preferences in read-only mode
  preferences.end();

  // Add custom parameters with saved values
  customMqttServer = new WiFiManagerParameter("mqtt_server", "MQTT Server", savedMqttServer.c_str(), 40);
  customMqttPort = new WiFiManagerParameter("mqtt_port", "MQTT Port", savedMqttPort.c_str(), 6);
  customDeviceName = new WiFiManagerParameter("device_name", "Device Name", savedDeviceName.c_str(), 20);
  customThemeColor = new WiFiManagerParameter("theme_color", "Theme Color", savedThemeColor.c_str(), 10, "type='color'");
  customUpdateInterval = new WiFiManagerParameter("update_interval", "Update Interval (s)", savedUpdateInterval.c_str(), 5, "type='number' min='5' max='3600'");
  
  wifiManager.addParameter(customMqttServer);
  wifiManager.addParameter(customMqttPort);
  wifiManager.addParameter(customDeviceName);
  wifiManager.addParameter(customThemeColor);
  wifiManager.addParameter(customUpdateInterval);

  // Set a static AP IP if desired
  wifiManager.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));

  // Generate a unique hostname based on MAC address
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String hostname = "modernwifi-" + String(mac[4], HEX) + String(mac[5], HEX);
  
  // Enable mDNS with the unique hostname
  wifiManager.setMDNSHostname(hostname.c_str());
  Serial.print("mDNS hostname set to: ");
  Serial.println(hostname + ".local");

#ifdef ENABLE_HTTPS
  // Enable HTTPS support if compiled with ENABLE_HTTPS and secure server available
  wifiManager.setUseHTTPS(true);
  // Provide actual SSL certificates for production use
  wifiManager.setSSLCredentials("-----BEGIN CERTIFICATE-----\n...your certificate...\n-----END CERTIFICATE-----",
                               "-----BEGIN PRIVATE KEY-----\n...your key...\n-----END PRIVATE KEY-----");
  Serial.println("HTTPS support enabled");
#endif

  // Custom HTML elements - we'll use Tailwind CSS from CDN
#ifdef ENABLE_HTML_INTERFACE
  wifiManager.setCustomHeadElement("<link href='https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css' rel='stylesheet'>");
#endif

  // AutoConnect: Try connecting with stored credentials; if that fails, launch the captive portal
  Serial.println("Attempting to autoConnect...");
  if (wifiManager.autoConnect(hostname.c_str(), "modernwifi")) {
    Serial.println("Connected to WiFi!");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    // If mDNS is enabled, print the mDNS address
    Serial.print("mDNS Address: ");
    Serial.println(hostname + ".local");
  } else {
    Serial.println("Failed to connect; configuration portal timed out.");
    // The configTimeoutCallback will handle this situation
  }
}

// Clean up WiFiManagerParameter objects to prevent memory leaks
void cleanupParameters() {
  if (customMqttServer) {
    delete customMqttServer;
    customMqttServer = nullptr;
  }
  if (customMqttPort) {
    delete customMqttPort;
    customMqttPort = nullptr;
  }
  if (customDeviceName) {
    delete customDeviceName;
    customDeviceName = nullptr;
  }
  if (customThemeColor) {
    delete customThemeColor;
    customThemeColor = nullptr;
  }
  if (customUpdateInterval) {
    delete customUpdateInterval;
    customUpdateInterval = nullptr;
  }
}

void loop() {
  // Process incoming HTTP and DNS requests (for the captive portal, JSON API, etc.)
  wifiManager.loop();
  
  // Get the update interval from preferences (default to 30 seconds if not set)
  static int updateInterval = 30;
  static bool updateIntervalLoaded = false;
  
  if (!updateIntervalLoaded) {
    preferences.begin("modernwifi", true); // Read-only mode
    String savedInterval = preferences.getString("update_interval", "30");
    preferences.end();
    
    updateInterval = savedInterval.toInt();
    if (updateInterval < 5) updateInterval = 5; // Minimum 5 seconds
    if (updateInterval > 3600) updateInterval = 3600; // Maximum 1 hour
    
    updateIntervalLoaded = true;
    Serial.print("Update interval set to: ");
    Serial.print(updateInterval);
    Serial.println(" seconds");
  }
  
  // Check WiFi connection status and reconnect if needed
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > (updateInterval * 1000)) { // Check based on user-defined interval
    lastCheck = millis();
    
    if (WiFi.status() == WL_CONNECTED) {
      // Log connection stats periodically
      Serial.print("WiFi connected - RSSI: ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");
    } else if (config.autoReconnect) {
      Serial.println("WiFi connection lost. Attempting to reconnect...");
      WiFi.reconnect();
    }
  }
  
  // Insert your main application code here
  // For example, read sensors, control actuators, communicate with MQTT, etc.
  
  // Small delay to prevent watchdog issues
  delay(10);
}

// Call this function when the program ends or during deep sleep
void onShutdown() {
  cleanupParameters();
}
