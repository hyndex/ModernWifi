# ModernWifi

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: ESP32|RP2040](https://img.shields.io/badge/Platform-ESP32%7CRP2040-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)
[![Version: 1.1.0](https://img.shields.io/badge/Version-1.1.0-brightgreen.svg)](https://github.com/yourusername/ModernWifi/releases)

A production-ready WiFi connection manager for ESP32 and RP2040 with captive portal, mDNS, HTTPS, secure authentication, and a sleek terminal-style serial monitor.

## 📋 Table of Contents

- [Features](#features)
- [What's New in v1.1.0](#whats-new-in-v110)
- [Installation](#installation)
  - [PlatformIO (Recommended)](#platformio-recommended)
  - [Arduino IDE](#arduino-ide)
  - [Cross-Platform Compilation](#cross-platform-compilation)
- [Quick Start](#quick-start)
- [User Manual](#user-manual)
  - [Basic Usage](#basic-usage)
  - [Configuration Portal](#configuration-portal)
  - [Custom Parameters](#custom-parameters)
  - [Callbacks](#callbacks)
  - [Static IP Configuration](#static-ip-configuration)
  - [mDNS Support](#mdns-support)
  - [HTTPS Support](#https-support)
  - [Authentication](#authentication)
  - [Serial Monitor](#serial-monitor)
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
- **Cross-Platform Support**: Works on ESP32 variants and RP2040 (Raspberry Pi Pico W)
- **Memory Management**: Proper cleanup of resources to prevent memory leaks
- **Filesystem Resilience**: Automatic recovery from filesystem mount failures
- **Authentication**: Secure portal access with username/password protection with modern UI
- **Serial Monitor**: Web-based terminal-style serial monitor for remote debugging with syntax highlighting
- **Dark Mode Support**: Elegant dark mode across all interfaces
- **Responsive Design**: Mobile-friendly interface that works on all devices
- **Visual Feedback**: Intuitive visual cues and animations for better user experience

## 🔄 What's New in v1.1.0

- **Enhanced Platform Support**: Added support for RP2040 (Raspberry Pi Pico W) and improved ESP32 variant compatibility
- **Memory Management Improvements**: Added proper cleanup of WiFiManagerParameter objects to prevent memory leaks
- **Platform-Specific Restart Handling**: Implemented platform-specific device restart methods for better compatibility
- **Filesystem Initialization Improvements**: Added automatic recovery from filesystem mount failures with formatting capability
- **Parameter Type System**: Enhanced parameter type handling with proper validation
- **Authentication Support**: Added secure portal access with username/password protection
- **Serial Monitor**: Added web-based terminal-style serial monitor with syntax highlighting and command history
- **Bug Fixes**:
  - Fixed syntax error in WiFiManager.cpp
  - Resolved compatibility issues between WiFiManager and WiFiManagerParameter classes
  - Improved error handling throughout the codebase

## 📥 Installation

### PlatformIO (Recommended)

1. Add ModernWifi to your `platformio.ini` file:

```ini
[env]
; Common settings for all environments
framework = arduino
monitor_speed = 115200

; Common library dependencies
lib_deps =
    https://github.com/bblanchon/ArduinoJson
    DNSServer
    ; Add ModernWifi repository URL here

[env:esp32]
platform = espressif32
board = esp32dev
lib_deps =
    ${env.lib_deps}
    https://github.com/ESP32Async/AsyncTCP
    https://github.com/ESP32Async/ESPAsyncWebServer
    Update
    FS
    SPIFFS
build_flags = 
    -DUSE_HTTPS
    -DWEBSERVER_HTTPS
    -DENABLE_MDNS
    -DENABLE_STATISTICS
    -DENABLE_CUSTOM_PARAMETERS
    -DENABLE_SERIAL_MONITOR
board_build.filesystem = spiffs
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

### Cross-Platform Compilation

ModernWifi is designed to work on multiple platforms. Here's how to compile for different boards using PlatformIO:

#### ESP32 (Fully Supported)

```bash
pio run -e esp32
```

This will compile the project for ESP32 with all features enabled, including HTTPS, mDNS support, statistics tracking, and custom parameters. The ESP32 environment uses SPIFFS for filesystem storage and includes the AsyncTCP and ESPAsyncWebServer libraries.

#### ESP32-S3 (Fully Supported)

```bash
pio run -e esp32-s3-devkitc-1
```

The ESP32-S3 environment extends the ESP32 configuration, inheriting all its settings while specifying the appropriate board.

#### RP2040 (Raspberry Pi Pico W - Partial Support)

```bash
pio run -e pico
```

The RP2040 environment uses LittleFS instead of SPIFFS and includes platform-specific libraries (AsyncTCP_RP2040W and AsyncWebServer_RP2040W). It supports statistics tracking and custom parameters but does not include HTTPS or mDNS support due to platform limitations.

**Note:** When compiling for RP2040, you may encounter issues with WiFi libraries and filesystem support. We're actively working to improve compatibility.

#### Other Platforms (Experimental)

The library includes configurations for AVR, STM32, and NXP platforms in the platformio.ini file, but these are currently experimental. To use these platforms, uncomment the relevant sections in platformio.ini and adjust as needed:

```ini
; AVR environment (Arduino Uno, etc.)
[env:avr]
platform = atmelavr
board = uno
lib_deps =
    ${env.lib_deps}
    WiFi101
    SD

; STM32 environment
[env:stm32]
platform = ststm32
board = nucleo_f401re
lib_deps =
    ${env.lib_deps}
    STM32duino WiFi
    SD

; NXP environment
[env:nxp]
platform = nxplpc
board = lpc1768
lib_deps =
    ${env.lib_deps}
    WiFi
    SD
```

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

// Enable authentication for the portal
config.useAuth = true;
config.portalUsername = "admin";
config.portalPassword = "password";

// Enable serial monitor in the portal
config.enableSerialMonitor = true;
config.serialMonitorBufferSize = 5000; // Buffer size for serial monitor

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

### Authentication

To secure your configuration portal with username and password authentication:

```cpp
// Enable authentication
wifiManager.setAuthentication(true, "admin", "secure_password");

// Check if authentication is enabled
if (wifiManager.isAuthenticationEnabled()) {
  Serial.println("Portal is secured with authentication");
}
```

The login interface features:
- Modern, responsive design with animated elements
- Dark mode support
- Password visibility toggle
- Visual feedback on input focus
- Animated particle background
- Secure credential handling

![Login Interface](screenshots/login.png)
*The secure login interface with animated particle background*

### Serial Monitor

The web-based serial monitor allows you to view device logs remotely:

```cpp
// Enable serial monitor in configuration
config.enableSerialMonitor = true;
config.serialMonitorBufferSize = 5000; // Buffer size in characters

// Or enable it directly
wifiManager.enableSerialMonitor(true, 5000);

// Access the serial monitor at: http://device-ip/serial
```

The serial monitor features:
- Syntax highlighting for code snippets
- Command history with up/down arrow navigation
- Adjustable baud rate
- Timestamps toggle
- Auto-scroll option
- Matrix-style visual effect toggle
- Log saving functionality

![Terminal Monitor](screenshots/terminal.png)
*The terminal-style serial monitor with syntax highlighting and command history*

### UI Customization

You can customize the appearance of the configuration portal:

```cpp
// Add custom HTML to the head section
wifiManager.setCustomHeadElement("<link href='https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css' rel='stylesheet'>");

// Add custom HTML to the body footer
wifiManager.setCustomBodyFooter("<div class='text-center'>My Custom Footer</div>");
```

You can also customize the branding by editing the `branding.json` file in the data directory:

```json
{
  "brand": {
    "name": "MyDevice",
    "logo": "/logo.png",
    "favicon": "/favicon.ico",
    "version": "1.0.0"
  },
  "theme": {
    "primary_color": "#3B82F6",
    "secondary_color": "#818CF8"
  }
}
```

#### Interface Screenshots

The ModernWifi configuration portal features a clean, responsive interface:

![PC Interface](screenshots/PC.png)
*The configuration portal as viewed on a desktop browser*

![Mobile Interface](screenshots/Mobile.png)
*The responsive mobile interface for on-the-go configuration*

![Dark Mode](screenshots/Darkmode.png)
*Dark mode interface for comfortable nighttime use*

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

#### Authentication

```cpp
void setAuthentication(bool enable, const char* username = "", const char* password = "");
bool isAuthenticationEnabled() const;
```

#### Serial Monitor

```cpp
void enableSerialMonitor(bool enable, unsigned int bufferSize = 5000);
bool isSerialMonitorEnabled() const;
String getSerialMonitorBuffer() const;
```

#### Network Scanning & Status

```cpp
std::vector<WiFiNetwork> scanNetworks(bool forceScan = false);
String getConnectionStatus();
uint8_t getLastConxResult();
```

### WiFiManagerParameter Class

```cpp
// Basic constructor
WiFiManagerParameter(const char* id, const char* label, const char* defaultValue, int length, const char* customHTML = "");

// Get the parameter value
const char* getValue() const;

// Get the parameter ID
const char* getID() const;

// Get the parameter label
const char* getLabel() const;
```

### WiFiManagerConfig Struct

```cpp
struct WiFiManagerConfig {
  uint16_t httpPort = 80;                  // Web server port
  unsigned long connectTimeout = 10000;    // WiFi connection timeout (ms)
  unsigned long configPortalTimeout = 180000; // Portal timeout (ms)
  bool autoReconnect = true;               // Auto reconnect if connection lost
  bool useAuth = false;                    // Enable authentication
  String portalUsername = "";              // Auth username
  String portalPassword = "";              // Auth password
  bool enableSerialMonitor = false;        // Enable web serial monitor
  unsigned int serialMonitorBufferSize = 5000; // Serial buffer size
};
```

## 🎨 Customization

### Build Flags

ModernWifi supports several build flags to enable or disable features:

```ini
-DUSE_HTTPS              # Enable HTTPS support
-DWEBSERVER_HTTPS        # Use AsyncWebServerSecure
-DENABLE_MDNS            # Enable mDNS support
-DENABLE_STATISTICS      # Enable statistics tracking
-DENABLE_CUSTOM_PARAMETERS # Enable custom parameters
-DENABLE_SERIAL_MONITOR  # Enable web-based serial monitor
```

### Web Interface

The web interface is built with:
- Tailwind CSS for styling
- Font Awesome for icons
- Vanilla JavaScript for interactivity

All web assets are stored in the `data` directory and should be uploaded to the device's filesystem.

### Parameter Types

ModernWifi supports various HTML input types for custom parameters:

- text (default)
- password
- number
- checkbox
- radio
- color
- date
- time
- range
- select
- textarea

Example:

```cpp
// Color picker
new WiFiManagerParameter("color", "Theme Color", "#ff0000", 10, "type='color'");

// Number with range
new WiFiManagerParameter("temp", "Temperature", "22", 5, "type='number' min='0' max='40' step='0.5'");
```

## 📝 Examples

Check the `examples` directory for sample code demonstrating various features:

- Basic WiFi configuration
- Custom parameters
- Static IP configuration
- HTTPS setup
- Authentication
- Serial monitor usage

## ❓ Troubleshooting

### Common Issues

1. **Cannot connect to WiFi**: Ensure your credentials are correct and the network is in range.

2. **Configuration portal not appearing**: Make sure you're calling `wifiManager.loop()` in your main loop.

3. **HTTPS not working**: Verify that you've compiled with `-DUSE_HTTPS` and `-DWEBSERVER_HTTPS` flags.

4. **Serial monitor not showing data**: Check that you've enabled it with `-DENABLE_SERIAL_MONITOR` and called `wifiManager.enableSerialMonitor(true)`.

5. **Memory issues**: If you're experiencing crashes, try reducing the serial monitor buffer size or disabling features you don't need.

## 👥 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.