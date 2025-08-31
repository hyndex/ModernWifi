# ModernWifi (ESP32)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)

ModernWifi is a production‑ready WiFi manager for the ESP32 family with a sleek, mobile‑first, multi‑step captive portal and a clean JSON API. It handles provisioning, reconnection, and device management (OTA, filesystem explorer, backup/restore, auth, mDNS, optional HTTPS) using an Async web server. The UI is served directly from the device (SPIFFS), so setup is fast and robust — no external services required.

This repository focuses exclusively on ESP32 and ships a single “Full UI + API” build for maximum simplicity and stability.

---

## Table of Contents

- [Features](#features)
- [Diagrams](#diagrams)
- [Architecture](#architecture)
- [Installation](#installation)
  - [PlatformIO](#platformio)
  - [Arduino IDE](#arduino-ide)
- [Build Flags](#build-flags)
- [Quick Start](#quick-start)
- [User Manual](#user-manual)
  - [Basic Usage](#basic-usage)
  - [Captive Portal](#captive-portal)
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
- [Changelog](#changelog)
- [Contributing](#contributing)
- [License](#license)

---

## Features

- **Async Server + Full UI & API**: Single build ships a modern, multi‑step, mobile‑first portal and a full JSON API.
- **Captive Portal**: Auto‑fallback AP with DNS capture for frictionless onboarding.
- **Multi‑Step UX**: Networks → Credentials → Settings/Connect with validation and friendly toasts.
- **Over‑the‑Air (OTA) Firmware Updates**: Endpoint stub ready for integration with your OTA flow.
- **File System Explorer**: Browse, upload, and delete files within SPIFFS.
- **Backup/Restore**: Export and import device configuration (JSON; stub with schema ready).
- **Multi‑Credential Support**: Manage several SSID/password pairs (in‑memory API; persistence optional).
- **Localization & Branding**: Customize labels, language, and branding via FS assets.
- **Auth & HTTPS**: Basic auth with optional HTTPS (when secure server is available).
- **mDNS**: Reach the device at `http://<hostname>.local` on the LAN.
- **WebSockets & Serial Monitor**: Optional live logs and a web‑based serial viewer.

---

## Diagrams

High‑level components and flows.

```mermaid
graph TD
  A[Browser/Phone] -- Captive Portal + API --> B[ESP32 ModernWifi]
  B --> C[AsyncWebServer]
  B --> D[DNSServer]
  B --> E[SPIFFS Assets]
  B --> F[WiFi Station/AP]
  B --> G[mDNS]
  B --> H[Handlers: /scan /connect /status_json /params_json /update_params]
```

```mermaid
sequenceDiagram
  autonumber
  participant User as User (Phone)
  participant ESP as ESP32 ModernWifi
  participant AP as AP Mode + DNS
  participant STA as STA Mode

  ESP->>ESP: Boot
  ESP->>STA: Try connect saved credentials
  alt Connected
    ESP-->>User: UI served from SPIFFS
    User->>ESP: /status_json polls
  else Failed
    ESP->>AP: Start AP + DNS (captive)
    User->>ESP: Open portal (index.html)
    User->>ESP: /scan → choose network
    User->>ESP: /connect (SSID/PASS + params)
    ESP->>STA: Connect to WiFi
    STA-->>ESP: Connected
    ESP-->>User: Status → Connected, IP, RSSI
  end
```

---

## Architecture

- ESPAsyncWebServer (core HTTP, web socket)
- DNSServer (captive portal DNS hijack in AP mode)
- SPIFFS (serves the static UI assets)
- WiFi station/AP and event hooks
- Optional modules: mDNS, HTTPS, WebSockets, Serial monitor, OTA, FS explorer, Backup/Restore

All features are toggled using build flags. There is a single, full‑featured build profile for ESP32 to reduce complexity and improve robustness.

---

## Installation

### PlatformIO

```bash
pio run -e esp32                       # build firmware
pio run -e esp32 --target upload       # flash
pio run -e esp32 --target buildfs      # build SPIFFS image (UI assets)
pio run -e esp32 --target uploadfs     # upload SPIFFS image
```

1. Connect your ESP32 board (ESP32 or ESP32‑S3).
2. Build and upload firmware.
3. Build and upload FS assets from `data/`.

### Arduino IDE

1. Install ESP32 board support and libraries (ArduinoJson, AsyncTCP, ESPAsyncWebServer, etc.).
2. Add project sources to your sketch/libraries.
3. Copy the `data/` folder and upload to SPIFFS with an appropriate uploader plugin.

Supported targets: ESP32, ESP32‑S3.

---

## Build Flags

Use the following flags to control advanced features (see `platformio.ini`):

- `-DENABLE_HTML_INTERFACE`
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

There is a single “Full UI + API” build shipped by default.

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
ModernWifi automatically manages your WiFi connections. It will attempt to reconnect using stored credentials; if unsuccessful, it launches a captive portal for configuration.

### Captive Portal
- Auto starts if connection fails or manually via `startConfigPortal()`.
- Mobile‑first, multi‑step UI: scan networks, set credentials, configure custom params.
- DNS capture ensures the portal opens reliably on phones.

### Custom Parameters & Grouping
- Add extra configuration fields (text, number, color, etc.) that can be grouped logically and validated.

Example:
```cpp
WiFiManagerParameter* mqttServer = new WiFiManagerParameter(
  "mqtt_server", "MQTT Server", "mqtt.example.com", 40
);
wifiManager.addParameter(mqttServer);
```

### Callbacks
Set callback functions for various events:
- **AP Mode**: Triggered when the captive portal is activated.
- **Save Config**: Called after a successful configuration.
- **Timeout**: Invoked when the configuration portal times out.

```cpp
void configModeCallback(WiFiManager* wm) {
  Serial.println("Entered configuration mode. AP IP: " + WiFi.softAPIP().toString());
}

wifiManager.setAPCallback(configModeCallback);
```

### Static IP & mDNS
- **Static IP**: Set fixed IP addresses for AP/STA modes.
- **mDNS**: Assign a hostname (e.g., `mydevice.local`).

```cpp
wifiManager.setAPStaticIPConfig(
  IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0)
);
wifiManager.setMDNSHostname("myesp32");
```

### HTTPS & Authentication
Secure your portal:
- **HTTPS**: Enable secure connections and set your SSL certificate and private key (requires secure server variant).
- **Authentication**: Protect your captive portal with a username and password.

```cpp
wifiManager.setUseHTTPS(true);
wifiManager.setSSLCredentials(
  "-----BEGIN CERTIFICATE-----...-----END CERTIFICATE-----",
  "-----BEGIN PRIVATE KEY-----...-----END PRIVATE KEY-----"
);
wifiManager.setAuthentication(true, "admin", "secure_password");
```

### Serial Monitor & Terminal Interface
- **Serial Monitor**: Enable the web-based serial monitor to remotely view logs.
- **Terminal Interface**: Access an interactive terminal (stub) via the `/terminal` endpoint.

### OTA Updates, File Explorer, & Backup/Restore
- **OTA Updates**: Initiate firmware updates via the `/ota` endpoint.
- **File Explorer**: Browse the filesystem with endpoints like `/fs/list`, `/fs/upload`, and `/fs/delete`.
- **Backup/Restore**: Export your entire configuration as JSON through `/backup` and restore it via `/restore`.

Notes: OTA and Backup/Restore are provided as stubs; wire them to your production libraries and storage if needed.

### Localization & UI Customization
- **Localization**: Set the device language.
- **UI Customization**: Add custom HTML head or footer elements; provide branding via `data/branding.json`.

Example:
```cpp
wifiManager.setCustomHeadElement(
  "<link href='https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css' rel='stylesheet'>"
);
#ifdef ENABLE_LOCALIZATION
wifiManager.setLanguage("en");
#endif
```

---

## API Reference

Endpoints
- `GET /` – Serves the portal UI (index.html) from SPIFFS
- `GET /status_json` – Connection status, IP, RSSI, last result, parameters
- `GET /scan` – WiFi network scan results
- `POST /connect` – Connect to WiFi (form fields: `ssid`, `password`, …)
- `GET /params_json` – List custom parameters (id, label, value, type, attributes)
- `POST /update_params` – Update custom parameter values (form data)
- `GET /reset` – Reset WiFi settings
- `GET /device_info` – Diagnostics (heap, uptime, RSSI, IP)
- `GET /fs/list`, `POST /fs/upload`, `DELETE /fs/delete` – File explorer (if enabled)
- `GET /backup`, `POST /restore` – Backup/Restore (if enabled)
- `GET /ota` – OTA stub (if enabled)

Auth & Security
- When `-DENABLE_AUTH` is set, HTTP Basic Auth protects endpoints (server will challenge).
- Consider rate‑limiting sensitive endpoints and enabling HTTPS in production.

Key classes
- **WiFiManager**: Initialization, connection, captive portal, endpoints.
- **WiFiManagerParameter**: Custom field definition, validation, and HTML generation helpers.
- **WiFiManagerConfig**: Timeouts, auth, serial monitor settings, etc.

---

## Examples

The `examples` directory includes sample sketches that demonstrate:
- Basic WiFi setup and auto-connect
- Custom parameter integration and callback usage (including validation)
- Static IP configuration and mDNS setup
- Secure HTTPS and authentication
- OTA update and file system management
- Real–time logging and terminal interface

---

## Troubleshooting

- **WiFi Connection Failures**: Check credentials, signal strength, and ensure proper configuration.
- **Captive Portal Not Showing**: Ensure that `wifiManager.loop()` is called in the main loop.
- **HTTPS Issues**: Verify that the appropriate build flags are set and that your certificates are valid.
- **Memory Crashes**: Reduce buffer sizes (e.g., for the serial monitor) or disable features not required.
- **OTA/File Explorer Not Working**: Endpoints are stubs—integrate your preferred libraries.
- **SPIFFS UI Missing**: Run `pio run -e esp32 --target buildfs && pio run -e esp32 --target uploadfs` to upload UI assets.

---

## Changelog

Recent changes in this repo:
- Simplified to ESP32 family only; removed AVR/STM32/NXP/RP2040 paths.
- Single “Full UI + API” build (removed multi‑mode builds).
- Overhauled UI: modern, mobile‑first, multi‑step onboarding with better mobile performance.
- Async server alignment, consistent auth gating, and ESP32 FS fixes.

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

