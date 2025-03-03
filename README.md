# ModernWifi

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)  
[![Platform: ESP32|RP2040](https://img.shields.io/badge/Platform-ESP32%7CRP2040-green.svg)](https://www.espressif.com/en/products/socs/esp32)  
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)  
[![Version: 1.2.0](https://img.shields.io/badge/Version-1.2.0-brightgreen.svg)](https://github.com/hyndex/ModernWifi/releases)

ModernWifi is a production–ready WiFi connection manager designed for ESP32 and RP2040 devices (with experimental support for AVR, STM32, and NXP). It not only handles WiFi provisioning through an intuitive captive portal but also integrates a full suite of advanced features—including over–the–air firmware updates, file system management, backup/restore functionality, multi–credential support, localization, real–time logging via WebSockets, an interactive terminal interface, mDNS, HTTPS, and secure authentication. Best of all, every feature is fully modular and configurable at compile–time using build flags, allowing you to tailor the firmware to exactly what you need.

---

## Table of Contents

- [Features](#features)
- [What's New in v1.2.0](#whats-new-in-v120)
- [Architecture & Modular Design](#architecture--modular-design)
- [Installation](#installation)
  - [PlatformIO (Recommended)](#platformio-recommended)
  - [Arduino IDE](#arduino-ide)
  - [Cross-Platform Compilation](#cross-platform-compilation)
- [Compilation Modes & Build Flags](#compilation-modes--build-flags)
- [Quick Start](#quick-start)
- [User Manual](#user-manual)
  - [Basic Usage](#basic-usage)
  - [Configuration Portal](#configuration-portal)
  - [Custom Parameters & Grouping](#custom-parameters--grouping)
  - [Callbacks](#callbacks)
  - [Static IP & mDNS](#static-ip--mdns)
  - [HTTPS & Authentication](#https--authentication)
  - [Serial Monitor & Terminal Interface](#serial-monitor--terminal-interface)
  - [OTA Updates, File Explorer, and Backup/Restore](#ota-updates-file-explorer-and-backuprestore)
  - [Localization & UI Customization](#localization--ui-customization)
- [API Reference](#api-reference)
- [Examples](#examples)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

---

## Features

ModernWifi is designed to be a one–stop solution for device provisioning and management. Its key features include:

- **Modular & Configurable Design**  
  Every module is controlled by compile–time flags so you can easily build a lightweight or feature–rich version without modifying source code.
  
- **Captive Portal & Multiple UI Modes**  
  Choose between:
  - **Ultra Light Mode**: API–only (JSON endpoints) for headless devices.
  - **Light Mode**: Minimal HTML interface with on–the–fly generated pages.
  - **Normal Mode**: Full–featured branded interface with external assets, dark mode, and advanced UI features.

- **Over–the–Air (OTA) Firmware Updates**  
  Update your firmware through a dedicated web endpoint. (Stub provided—you can integrate your OTA library.)

- **File System Explorer & Backup/Restore**  
  Browse, upload, and delete files on the onboard filesystem. Easily export and import your device configuration via JSON.

- **Multi–Credential Support**  
  Store and manage multiple WiFi credentials to allow for automatic fallback between networks.

- **Custom Parameters with Grouping & Validation**  
  Add extra configuration fields (text, number, color, etc.) that can be grouped logically and validated with built–in or custom logic.

- **Localization Support**  
  Easily deploy your device internationally by setting language and localized parameter labels.

- **Secure Authentication & HTTPS**  
  Lock down your captive portal with username/password protection and secure communications using HTTPS (with optional certificate support).

- **mDNS Support**  
  Access your device on your local network by its hostname (e.g., `mydevice.local`).

- **Real–Time Logging via WebSockets**  
  (Optional) Stream live log messages to a web client for debugging and monitoring.

- **Interactive Terminal & Web-Based Serial Monitor**  
  Remotely view serial logs or interact with your device using a terminal–style interface.

- **Responsive & Modern UI**  
  Built with modern design principles, including dark mode, custom branding, and Tailwind CSS (in Normal Mode).

---

## What's New in v1.2.0

- **Complete Modular Build**: All advanced features are now controlled by compile–time flags.
- **OTA & File Management**: New endpoints for OTA firmware updates and file system explorer.
- **Backup & Restore**: Easily export and import complete device configurations.
- **Enhanced Multi–Credential & Localization Support**: Manage multiple WiFi credentials and support localized configuration.
- **Real–Time WebSocket Logging & Terminal Interface**: New real–time log streaming and interactive terminal.
- **Improved UI & Customization Options**: Fully customizable HTML interfaces with dark mode and branding support.
- **Advanced Security Enhancements**: Optional HTTPS, mDNS, and authentication for secure deployment.

---

## Architecture & Modular Design

ModernWifi is designed with flexibility in mind. Every module—from the captive portal UI to advanced endpoints (OTA, file explorer, backup/restore, etc.)—is wrapped in preprocessor conditionals. This means that by simply defining (or omitting) a build flag, you can include or exclude features without touching the source code.

### Example Build Flags:
- `-DENABLE_HTML_INTERFACE`: Enables the HTML-based configuration portal.
- `-DENABLE_SERIAL_MONITOR`: Includes the web-based serial monitor.
- `-DENABLE_WEBSOCKETS`: Compiles WebSocket support for real-time logging.
- `-DENABLE_OTA`: Activates the OTA firmware update endpoint.
- `-DENABLE_FS_EXPLORER`: Enables the file explorer endpoints.
- `-DENABLE_BACKUP_RESTORE`: Includes backup and restore configuration endpoints.
- `-DENABLE_TERMINAL`: Adds an interactive terminal interface.
- `-DENABLE_MULTI_CRED`: Supports multiple WiFi credentials.
- `-DENABLE_LOCALIZATION`: Provides localization and language support.
- `-DENABLE_MDNS`: Includes mDNS support.
- `-DENABLE_HTTPS`: Activates HTTPS support.
- `-DENABLE_AUTH`: Enables portal authentication.

These flags are set via the PlatformIO configuration file or your build system.

---

## Installation

### PlatformIO (Recommended)

1. **Add ModernWifi to Your Project**  
   Include the library in your `platformio.ini` file along with its dependencies.

2. **Configure Build Flags**  
   Choose your build mode and enable/disable modules by setting build flags (see the provided `platformio.ini` sample).

3. **Include Headers in Your Code**  
   ```cpp
   #include <Arduino.h>
   #include "WiFiManager.h"
   ```

### Arduino IDE

1. Download the repository as a ZIP file.
2. In Arduino IDE, navigate to **Sketch > Include Library > Add .ZIP Library**.
3. Install required dependencies (ArduinoJson, AsyncTCP/AsyncWebServer, etc.).
4. Configure your build flags via the IDE’s compiler options (if supported) or modify the source flags.

### Cross-Platform Compilation

ModernWifi supports:
- **ESP32 & ESP32-S3**: Fully featured with HTTPS, mDNS, and all advanced modules.
- **RP2040 (Raspberry Pi Pico W)**: Uses LittleFS and supports core features along with custom parameters.
- **AVR, STM32, NXP**: Experimental support available in the `platformio.ini` (uncomment and adjust as needed).

---

## Compilation Modes & Build Flags

ModernWifi supports three compilation modes that define the level of UI and asset inclusion:
- **Ultra Light Mode** (`-DBUILD_MODE_ULTRA_LIGHT`): API-only, no HTML interface.
- **Light Mode** (`-DBUILD_MODE_LIGHT`): Basic HTML interface with minimal assets.
- **Normal Mode** (`-DBUILD_MODE_NORMAL`): Full–featured interface with branding, external CSS/JS, dark mode, etc.

Additionally, use the following flags to control advanced features:
- `-DENABLE_HTML_INTERFACE`
- `-DENABLE_BRANDING`
- `-DENABLE_SERIAL_MONITOR`
- `-DENABLE_WEBSOCKETS`
- `-DENABLE_OTA`
- `-DENABLE_FS_EXPLORER`
- `-DENABLE_BACKUP_RESTORE`
- `-DENABLE_TERMINAL`
- `-DENABLE_MULTI_CRED`
- `-DENABLE_LOCALIZATION`
- `-DENABLE_MDNS`
- `-DENABLE_HTTPS`
- `-DENABLE_AUTH`

These flags are defined in your `platformio.ini` as shown in the sample configuration.

---

## Quick Start

```cpp
#include <Arduino.h>
#include "WiFiManager.h"

WiFiManagerConfig config;
#ifdef ENABLE_AUTH
config.useAuth = true;
config.portalUsername = "admin";
config.portalPassword = "password";
#endif
#ifdef ENABLE_SERIAL_MONITOR
config.enableSerialMonitor = true;
config.serialMonitorBufferSize = 5000;
#endif

WiFiManager wifiManager(config);

void setup() {
  Serial.begin(115200);
  wifiManager.begin();
  
  // Auto-connect: tries stored credentials; if fails, launches the captive portal.
  if (wifiManager.autoConnect("ESP32_AP", "password")) {
    Serial.println("Connected to WiFi!");
  } else {
    Serial.println("Failed to connect. Captive portal is active.");
  }
}

void loop() {
  wifiManager.loop();
  delay(10);
}
```

---

## User Manual

### Basic Usage
ModernWifi automatically manages your WiFi connections. It will attempt to reconnect using stored credentials; if unsuccessful, it will launch a captive portal for configuration.

### Configuration Portal
- **Modes**: Ultra Light (JSON API), Light (simple HTML), or Normal (full–featured UI).
- **Functionality**: Scan available networks, enter credentials, and configure custom parameters.
- **Access**: Automatically launched if connection fails, or manually by calling `startConfigPortal()`.

### Custom Parameters & Grouping
- **Adding Fields**: Create custom parameters (e.g., MQTT settings, sensor thresholds).
- **Grouping**: Optionally group parameters (for example, “WiFi Settings” or “Device Info”) to keep the UI organized.
- **Validation**: Use built–in validators or provide custom validation functions.
  
Example:
```cpp
WiFiManagerParameter* mqttServer = new WiFiManagerParameter("mqtt_server", "MQTT Server", "mqtt.example.com", 40, "", 1, "Network");
wifiManager.addParameter(mqttServer);
```

### Callbacks
Set callback functions for various events:
- **AP Mode**: Triggered when the captive portal is activated.
- **Save Config**: Called after a successful configuration.
- **Timeout**: Invoked when the configuration portal times out.
  
Example:
```cpp
void configModeCallback(WiFiManager* wm) {
  Serial.println("Entered configuration mode. AP IP: " + WiFi.softAPIP().toString());
}

wifiManager.setAPCallback(configModeCallback);
```

### Static IP & mDNS
- **Static IP**: Set fixed IP addresses for both access point and station modes.
- **mDNS**: Assign a hostname (e.g., `mydevice.local`) so you can access your device by name.
  
Example:
```cpp
wifiManager.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
wifiManager.setMDNSHostname("myesp32");
```

### HTTPS & Authentication
Secure your portal:
- **HTTPS**: Enable secure connections and set your SSL certificate and private key.
- **Authentication**: Protect your captive portal with a username and password.
  
Example:
```cpp
wifiManager.setUseHTTPS(true);
wifiManager.setSSLCredentials("-----BEGIN CERTIFICATE-----...-----END CERTIFICATE-----",
                              "-----BEGIN PRIVATE KEY-----...-----END PRIVATE KEY-----");
wifiManager.setAuthentication(true, "admin", "secure_password");
```

### Serial Monitor & Terminal Interface
- **Serial Monitor**: Enable the web-based serial monitor to remotely view logs.
- **Terminal Interface**: Access an interactive terminal (stub provided) via the `/terminal` endpoint.
  
Example:
```cpp
// In your config:
config.enableSerialMonitor = true;
config.serialMonitorBufferSize = 5000;
```

### OTA Updates, File Explorer, & Backup/Restore
- **OTA Updates**: Initiate firmware updates via the `/ota` endpoint.
- **File Explorer**: Browse the filesystem with endpoints like `/fs/list`, `/fs/upload`, and `/fs/delete`.
- **Backup/Restore**: Export your entire configuration as JSON through `/backup` and restore it via `/restore`.
  
These endpoints are provided as stubs—you can integrate your own update and file management libraries.

### Localization & UI Customization
- **Localization**: Set the device language to support multiple locales and adjust parameter group labels.
- **UI Customization**: Use `setCustomHeadElement()` and `setCustomBodyFooter()` to add custom HTML (e.g., include external CSS frameworks like Tailwind CSS or custom branding elements).
  
Example:
```cpp
wifiManager.setCustomHeadElement("<link href='https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css' rel='stylesheet'>");
wifiManager.setCustomBodyFooter("<div class='text-center'>© 2023 MyCompany</div>");
#ifdef ENABLE_LOCALIZATION
wifiManager.setLanguage("en");
#endif
```

---

## API Reference

ModernWifi provides a comprehensive API. Refer to the in-code documentation for complete details. Key classes include:

- **WiFiManager**  
  Handles initialization, connection management, captive portal, and advanced endpoints.
  
  Key methods:
  - `begin()`, `loop()`
  - `autoConnect()`, `startConfigPortal()`
  - `connectToNetwork()`, `disconnectFromNetwork()`
  - `setAPStaticIPConfig()`, `setSTAStaticIPConfig()`
  - `addParameter()`, `getParameters()`
  - Advanced endpoints: `handleOTA()`, `handleFSList()`, `handleBackup()`, etc.

- **WiFiManagerParameter**  
  Manages custom configuration fields, including HTML generation and validation.
  
  Key methods:
  - Constructors for basic and advanced types
  - `getValue()`, `getID()`, `getLabel()`
  - `setValue()`, `setValidation()`
  - `getHTML()`

- **WiFiManagerConfig**  
  A structure to configure connection timeouts, authentication, serial monitor settings, etc.

---

## Customization & Build Flags

ModernWifi is fully modular. Customize your firmware by adding (or omitting) the following build flags in your `platformio.ini`:

- `-DENABLE_HTML_INTERFACE` – Include the captive portal HTML interface.
- `-DENABLE_BRANDING` – Enable full–featured branded UI (requires your assets).
- `-DENABLE_SERIAL_MONITOR` – Compile the web–based serial monitor.
- `-DENABLE_WEBSOCKETS` – Enable WebSocket support for real–time logging.
- `-DENABLE_OTA` – Include the OTA update endpoint.
- `-DENABLE_FS_EXPLORER` – Enable file system explorer endpoints.
- `-DENABLE_BACKUP_RESTORE` – Include backup/restore configuration endpoints.
- `-DENABLE_TERMINAL` – Add the interactive terminal interface.
- `-DENABLE_MULTI_CRED` – Support multiple WiFi credentials.
- `-DENABLE_LOCALIZATION` – Enable language support and localized UI.
- `-DENABLE_MDNS` – Include mDNS support.
- `-DENABLE_HTTPS` – Enable secure HTTPS connections.
- `-DENABLE_AUTH` – Compile in authentication for the portal.

Additionally, choose one of three build modes:
- Ultra Light (`-DBUILD_MODE_ULTRA_LIGHT`)
- Light (`-DBUILD_MODE_LIGHT`)
- Normal (`-DBUILD_MODE_NORMAL`)

This flexibility lets you create a tailored firmware that meets your project’s memory and feature requirements.

---

## Examples

The `examples` directory includes sample sketches that demonstrate:
- Basic WiFi setup and auto-connect
- Custom parameter integration and callback usage
- Static IP configuration and mDNS setup
- Secure HTTPS and authentication
- OTA update and file system management
- Real–time logging and terminal interface

---

## Troubleshooting

- **WiFi Connection Failures**: Check credentials, signal strength, and ensure proper configuration.
- **Captive Portal Not Showing**: Ensure that `wifiManager.loop()` is called in the main loop.
- **HTTPS Issues**: Verify that the appropriate build flags are set and that your certificates are valid.
- **Memory Crashes**: Reduce buffer sizes (e.g., for the serial monitor) or disable features that are not required.
- **OTA/File Explorer Not Working**: These endpoints are provided as stubs—integrate your preferred libraries for full functionality.

---

## Contributing

Contributions are welcome! To contribute:
1. Fork the repository.
2. Create your feature branch (`git checkout -b feature/your-feature`).
3. Commit your changes with detailed messages.
4. Push to your branch and open a Pull Request.

For major changes, please open an issue first to discuss what you would like to change.

---

## License

ModernWifi is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
