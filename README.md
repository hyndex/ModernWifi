# ModernWifi

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)
[![Version: 1.0.0](https://img.shields.io/badge/Version-1.0.0-brightgreen.svg)](https://github.com/yourusername/ModernWifi/releases)

A production-ready WiFi connection manager for ESP32 with captive portal, mDNS, HTTPS, and JSON API endpoints.

## 📋 Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [User Manual](#user-manual)
  - [Basic Usage](#basic-usage)
  - [Configuration Portal](#configuration-portal)
  - [Custom Parameters](#custom-parameters)
  - [Callbacks](#callbacks)
  - [Static IP Configuration](#static-ip-configuration)
  - [mDNS Support](#mdns-support)
  - [HTTPS Support](#https-support)
  - [UI Customization](#ui-customization)
- [API Reference](#api-reference)
  - [WiFiManager Class](#wifimanager-class)
  - [WiFiManagerParameter Class](#wifimanagerparameter-class)
  - [WiFiManagerConfig Struct](#wifimanagerconfig-struct)
- [Customization](#customization)
  - [Build Flags](#build-flags)
  - [Web Interface](#web-interface)
  - [Parameter Types](#parameter-types)
- [Examples](#examples)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## ✨ Features

- **Easy WiFi Configuration**: Set up WiFi credentials without hardcoding
- **Captive Portal**: User-friendly configuration interface
- **Custom Parameters**: Add your own configuration fields
- **Persistent Storage**: Save settings to flash memory
- **Automatic Reconnection**: Handles connection drops gracefully
- **mDNS Support**: Access your device via hostname.local
- **HTTPS Support**: Secure web interface (optional)
- **JSON API**: Programmatic access to configuration
- **Modern UI**: Clean interface with Tailwind CSS
- **Multiple Parameter Types**: Support for various input types (text, number, color, etc.)
- **Validation**: Built-in and custom validation for parameters

## 📥 Installation

### PlatformIO (Recommended)

1. Add ModernWifi to your `platformio.ini` file:

```ini
lib_deps =
    bblanchon/ArduinoJson @ ^6.21.3
    me-no-dev/AsyncTCP @ ^1.1.1
    https://github.com/me-no-dev/ESPAsyncWebServer.git
    # Add ModernWifi repository URL here
```

2. Include the necessary headers in your code:

```cpp
#include <Arduino.h>
#include "WiFiManager.h"
#include <Preferences.h>
```

### Arduino IDE

1. Download this repository as a ZIP file
2. In Arduino IDE, go to Sketch > Include Library > Add .ZIP Library
3. Select the downloaded ZIP file
4. Install the required dependencies:
   - ArduinoJson
   - AsyncTCP
   - ESPAsyncWebServer
   - ESP32 Arduino Core

## 🚀 Quick Start

```cpp
#include <Arduino.h>
#include "WiFiManager.h"

// Create a WiFiManager instance with default configuration
WiFiManagerConfig config;
WiFiManager wifiManager(config);

void setup() {
  Serial.begin(115200);
  
  // Initialize WiFiManager
  wifiManager.begin();
  
  // Start the configuration portal if needed
  // If no stored credentials or connection fails, this will start a captive portal
  if (wifiManager.autoConnect("ESP32_AP", "password")) {
    Serial.println("Connected to WiFi!");
  } else {
    Serial.println("Failed to connect.");
  }
}

void loop() {
  // Required to handle DNS and HTTP requests for the captive portal
  wifiManager.loop();
  
  // Your code here
  delay(10);
}
```

## 📖 User Manual

### Basic Usage

The ModernWifi library provides a simple way to manage WiFi connections on ESP32 devices. It handles the connection process, provides a captive portal for configuration, and offers persistent storage for settings.

#### Initialization

```cpp
// Create a configuration object
WiFiManagerConfig config;

// Customize configuration if needed
config.connectTimeout = 15000;      // 15 seconds to connect to saved WiFi
config.configPortalTimeout = 180000; // 3 minutes for configuration portal
config.httpPort = 80;               // Web server port
config.autoReconnect = true;        // Auto reconnect if connection is lost

// Create WiFiManager instance with the configuration
WiFiManager wifiManager(config);

// Initialize WiFiManager
wifiManager.begin();
```

#### Connecting to WiFi

```cpp
// Method 1: Auto-connect (tries saved credentials, starts portal if needed)
if (wifiManager.autoConnect("ESP32_AP", "password")) {
  Serial.println("Connected to WiFi!");
}

// Method 2: Start configuration portal directly
if (wifiManager.startConfigPortal("ESP32_AP", "password")) {
  Serial.println("Connected after configuration!");
}

// Method 3: Connect to a specific network
if (wifiManager.connectToNetwork("MySSID", "MyPassword")) {
  Serial.println("Connected to specified network!");
}
```

### Configuration Portal

The configuration portal is a captive portal that allows users to:

1. Select a WiFi network from a list of available networks
2. Enter the password for the selected network
3. Configure custom parameters (if defined)
4. Save the configuration

The portal can be accessed in two ways:

1. Automatically when `autoConnect()` is called and no saved credentials exist or connection fails
2. Manually by calling `startConfigPortal()`

When active, the portal creates an access point with the specified name and password. Users can connect to this AP and will be redirected to the configuration page.

### Custom Parameters

You can add custom parameters to the configuration portal to allow users to configure additional settings:

```cpp
// Create custom parameters
WiFiManagerParameter* customMqttServer = new WiFiManagerParameter("mqtt_server", "MQTT Server", "mqtt.example.com", 40);
WiFiManagerParameter* customMqttPort = new WiFiManagerParameter("mqtt_port", "MQTT Port", "1883", 6);

// Add parameters to WiFiManager
wifiManager.addParameter(customMqttServer);
wifiManager.addParameter(customMqttPort);

// Later, retrieve values
Serial.println(customMqttServer->getValue());
Serial.println(customMqttPort->getValue());
```

For advanced parameter types:

```cpp
// Color picker
WiFiManagerParameter* customThemeColor = new WiFiManagerParameter("theme_color", "Theme Color", "#2196F3", 10, "type='color'");

// Number input with constraints
WiFiManagerParameter* customUpdateInterval = new WiFiManagerParameter("update_interval", "Update Interval (s)", "30", 5, "type='number' min='5' max='3600'");
```

### Callbacks

ModernWifi provides several callbacks to handle different events:

```cpp
// Called when the device enters configuration mode (captive portal)
void configModeCallback(WiFiManager* wm) {
  Serial.println("Entered configuration mode.");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

// Called when the configuration is saved after a successful connection
void saveConfigCallback() {
  Serial.println("Configuration saved.");
  // Save custom parameters to persistent storage
  // ...
}

// Called when the configuration portal times out
void configTimeoutCallback() {
  Serial.println("Configuration portal timed out.");
  // Handle timeout (reconnect, restart, etc.)
  // ...
}

// Set callbacks
wifiManager.setAPCallback(configModeCallback);
wifiManager.setSaveConfigCallback(saveConfigCallback);
wifiManager.setConfigPortalTimeoutCallback(configTimeoutCallback);
```

### Static IP Configuration

```cpp
// Set static IP for the access point
wifiManager.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));

// Set static IP for the station (when connected to a router)
wifiManager.setSTAStaticIPConfig(IPAddress(192,168,1,100), IPAddress(192,168,1,1), IPAddress(255,255,255,0), IPAddress(8,8,8,8));
```

### mDNS Support

Multicast DNS allows you to access your device using a hostname instead of an IP address:

```cpp
// Set mDNS hostname
wifiManager.setMDNSHostname("myesp32");

// Now you can access your device at: http://myesp32.local
```

### HTTPS Support

For secure connections, you can enable HTTPS support:

```cpp
// Enable HTTPS
wifiManager.setUseHTTPS(true);

// Set SSL certificates
wifiManager.setSSLCredentials(
  "-----BEGIN CERTIFICATE-----\n...your certificate...\n-----END CERTIFICATE-----",
  "-----BEGIN PRIVATE KEY-----\n...your key...\n-----END PRIVATE KEY-----"
);
```

### UI Customization

You can customize the appearance of the configuration portal:

```cpp
// Add custom HTML to the head section
wifiManager.setCustomHeadElement("<link href='https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css' rel='stylesheet'>");

// Add custom HTML to the body footer
wifiManager.setCustomBodyFooter("<div class='text-center'>My Custom Footer</div>");

// Change the theme colors
wifiManager.setThemeColor("#4F46E5", "#818CF8"); // primary and secondary colors

// Set custom logo
wifiManager.setLogo("/custom_logo.png");
```

#### Interface Screenshots

The ModernWifi configuration portal features a clean, responsive interface:

![Captive Portal Main Screen](screenshots/captive_portal_main.png)
*The main configuration screen with network selection*

![Custom Parameters](screenshots/custom_parameters.png)
*Custom parameter configuration with various input types*

![Connection Success](screenshots/connection_success.png)
*Successful connection screen with device information*

## 📚 API Reference

### WiFiManager Class

#### Constructor

```cpp
WiFiManager(const WiFiManagerConfig& config = WiFiManagerConfig());
```

#### Initialization & Main Loop

```cpp
void begin();   // Initialize HTTP endpoints, DNS, MDNS (if enabled)
void loop();    // Must be called in your main loop
```

#### Connection Management

```cpp
bool autoConnect(const char* apName = nullptr, const char* apPassword = nullptr);
bool startConfigPortal(const char* apName, const char* apPassword = nullptr);
void stopConfigPortal();
bool connectToNetwork(const char* ssid, const char* password);
bool disconnectFromNetwork();
void resetSettings();
```

#### Static IP Configuration

```cpp
void setAPStaticIPConfig(IPAddress ip, IPAddress gateway, IPAddress subnet);
void setSTAStaticIPConfig(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns = IPAddress(0,0,0,0));
```

#### Custom Parameters

```cpp
bool addParameter(WiFiManagerParameter* param);
std::vector<WiFiManagerParameter*> getParameters() const;
```

#### Debug & Callbacks

```cpp
void setDebugOutput(bool debug, Print& debugPort = Serial);
void setAPCallback(std::function<void(WiFiManager*)> callback);
void setSaveConfigCallback(std::function<void()> callback);
void setConfigPortalTimeoutCallback(std::function<void()> callback);
```

#### Custom HTML Injection

```cpp
void setCustomHeadElement(const char* html);
void setCustomBodyFooter(const char* html);
```

#### MDNS Support

```cpp
void setMDNSHostname(const char* hostname);
```

#### HTTPS Support

```cpp
void setUseHTTPS(bool flag);
void setSSLCredentials(const char* cert, const char* key);
```

#### Network Scanning & Status

```cpp
std::vector<WiFiNetwork> scanNetworks(bool forceScan = false);
String getConnectionStatus();
uint8_t getLastConxResult();
```

### WiFiManagerParameter Class

#### Constructors

```cpp
// Basic text parameter
WiFiManagerParameter(const char* id, const char* label, const char* defaultValue, int length, const char* customHTML = "", int labelPlacement = 1);

// Advanced parameter types
WiFiManagerParameter(const char* id, const char* label, const char* defaultValue, ParameterType type, const char* customAttributes = "");
```

#### Getters

```cpp
const char* getID() const;
const char* getLabel() const;
const char* getValue() const;
ParameterType getType() const;
const char* getCustomAttributes() const;
int getLabelPlacement() const;
bool isValid() const;
```

#### Setters

```cpp
void setValue(const char* value);
void setCustomAttributes(const char* attributes);
void setValidation(std::function<bool(const char*)> validator);
```

#### HTML Generation

```cpp
String getHTML() const;
```

### WiFiManagerConfig Struct

```cpp
struct WiFiManagerConfig {
  uint16_t httpPort = 80;                  // Web server port
  unsigned long connectTimeout = 10000;    // in milliseconds
  unsigned long configPortalTimeout = 180000; // in milliseconds (3 minutes)
  bool autoReconnect = true;               // Auto reconnect if connection is lost
};
```

### Parameter Types

```cpp
enum class ParameterType {
  TEXT,
  PASSWORD,
  NUMBER,
  TOGGLE,
  SLIDER,
  SELECT,
  EMAIL,
  URL,
  SEARCH,
  TEL,
  DATE,
  TIME,
  DATETIME_LOCAL,
  MONTH,
  WEEK,
  COLOR,
  FILE,
  HIDDEN,
  TEXTAREA
};
```