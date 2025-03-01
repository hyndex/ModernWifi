#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

// Platform detection
#if defined(ESP32) || defined(ESP32S2) || defined(ESP32S3) || defined(ESP32C2) || defined(ESP32C3) || defined(ESP32C5) || defined(ESP32C6)
  #define USING_ESP32
  #include <WiFi.h>
  #include <ESPmDNS.h>
  #include <SPIFFS.h>
  #include <AsyncTCP.h>
  #include <ESPAsyncWebServer.h>
#elif defined(ARDUINO_ARCH_RP2040)
  // RP2040 (Raspberry Pi Pico W, RP2350)
  #include <WiFi.h>
  #include <LittleFS.h>
  #define USING_RP2040
  // Use appropriate libraries for RP2040
  #include <AsyncTCP_RP2040W.h>
  #include <ESPAsyncWebServer.h>
  #define SPIFFS LittleFS
#elif defined(ARDUINO_ARCH_AVR)
  // Atmel AVR microcontrollers
  #define USING_AVR
  #include <WiFi101.h> // For AVR with WiFi shield
  #include <SPI.h>
  #include <SD.h>
  #define SPIFFS SD
#elif defined(ARDUINO_ARCH_STM32)
  // STM32 microcontrollers
  #define USING_STM32
  #include <WiFi.h>
  #include <SPI.h>
  #include <SD.h>
  #define SPIFFS SD
#elif defined(ARDUINO_ARCH_NXP)
  // NXP microcontrollers
  #define USING_NXP
  #include <WiFi.h>
  #include <SPI.h>
  #include <SD.h>
  #define SPIFFS SD
#else
  #error "Unsupported platform. Please check compatibility."
#endif

#include <DNSServer.h>
#include <vector>
#include <functional>

#ifdef USE_HTTPS
  #include <WiFiClientSecure.h>
  // Using AsyncWebServer with HTTPS support
  #include <AsyncWebServerSecure.h>
#endif

// Structure holding scanned network information.
struct WiFiNetwork {
  String ssid;
  int32_t rssi;
  uint8_t encryptionType;
};

// Forward declaration of ParameterType enum class
enum class ParameterType;

// WiFiManagerParameter: For adding extra configuration fields.
class WiFiManagerParameter {
public:
  // labelPlacement: 0 = no label, 1 = before field, 2 = after field.
  WiFiManagerParameter(const char* id, const char* label, const char* defaultValue, int length, const char* customHTML = "", int labelPlacement = 1);
  
  // Constructor for advanced parameter types
  WiFiManagerParameter(const char* id, const char* label, const char* defaultValue,
                      ParameterType type, const char* customAttributes = "");
  
  const char* getID() const;
  const char* getLabel() const;
  const char* getValue() const;
  void setValue(const char* value);
  const char* getCustomHTML() const;
  const char* getCustomAttributes() const;
  ParameterType getType() const;
  int getLabelPlacement() const;
  bool isValid() const;
  
  // Validation
  void setValidation(std::function<bool(const char*)> validator);
  
private:
  String _id;
  String _label;
  String _value;
  int _length;
  String _customHTML;
  String _customAttributes;
  int _labelPlacement;
  ParameterType _type;
  std::function<bool(const char*)> _validator;
  
  // Helper methods
  bool validateValue(const char* value) const;
};

// Configuration structure.
struct WiFiManagerConfig {
  uint16_t httpPort = 80;
  unsigned long connectTimeout = 10000;       // in milliseconds
  unsigned long configPortalTimeout = 180000;   // in milliseconds (3 minutes)
  bool autoReconnect = true;
};

class WiFiManager {
public:
  WiFiManager(const WiFiManagerConfig& config = WiFiManagerConfig());
  ~WiFiManager();

  // Initialization & main loop.
  void begin();   // Initialize HTTP endpoints, DNS, MDNS (if enabled).
  void loop();    // Must be called in your main loop.

  // Connection management.
  bool autoConnect(const char* apName = nullptr, const char* apPassword = nullptr);
  bool startConfigPortal(const char* apName, const char* apPassword = nullptr);
  void stopConfigPortal();
  bool connectToNetwork(const char* ssid, const char* password);
  bool disconnectFromNetwork();
  void resetSettings();

  // Static IP configuration.
  void setAPStaticIPConfig(IPAddress ip, IPAddress gateway, IPAddress subnet);
  void setSTAStaticIPConfig(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns = IPAddress(0,0,0,0));

  // Custom parameters.
  bool addParameter(WiFiManagerParameter* param);
  std::vector<WiFiManagerParameter*> getParameters() const;

  // Debug & callbacks.
  void setDebugOutput(bool debug, Print& debugPort = Serial);
  void setAPCallback(std::function<void(WiFiManager*)> callback);
  void setSaveConfigCallback(std::function<void()> callback);
  void setConfigPortalTimeoutCallback(std::function<void()> callback);

  // Custom HTML injection.
  void setCustomHeadElement(const char* html);
  void setCustomBodyFooter(const char* html);

  // MDNS support.
  void setMDNSHostname(const char* hostname);

  // HTTPS support.
  void setUseHTTPS(bool flag);
#ifdef USE_HTTPS
  void setSSLCredentials(const char* cert, const char* key);
#endif

  // JSON API endpoints are built into HTTP handlers.
  // Additionally, scanNetworks() is provided to scan for available networks.
  std::vector<WiFiNetwork> scanNetworks(bool forceScan = false);

  // Status methods.
  String getConnectionStatus();
  uint8_t getLastConxResult();

private:
  WiFiManagerConfig _config;
  AsyncWebServer* _server; // Pointer to allow HTTP or HTTPS mode.
  DNSServer _dnsServer;
  std::vector<WiFiManagerParameter*> _params;
  std::function<void(WiFiManager*)> _apCallback;
  std::function<void()> _saveConfigCallback;
  std::function<void()> _configPortalTimeoutCallback;
  bool _debug;
  Print* _debugPort;
  String _customHeadElement;
  String _customBodyFooter;
  bool _useMDNS;
  String _mdnsHostname;
  bool _useHTTPS;
#ifdef USE_HTTPS
  String _sslCert;
  String _sslKey;
#endif
  unsigned long _configPortalStart;
  uint8_t _lastConxResult;

  // Internal HTTP handlers.
  void handleRoot(AsyncWebServerRequest *request);
  void handleScan(AsyncWebServerRequest *request);
  void handleConnect(AsyncWebServerRequest *request);
  void handleReset(AsyncWebServerRequest *request);
  void handleNotFound(AsyncWebServerRequest *request);
  // JSON endpoints.
  void handleStatusJSON(AsyncWebServerRequest *request);
  void handleParamsJSON(AsyncWebServerRequest *request);

  // Internal helper methods.
  void debug(String msg);
  void startDNS();
  bool startAPMode(const char* apName, const char* apPassword);
};

#endif // WIFI_MANAGER_H
