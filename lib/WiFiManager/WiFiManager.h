#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

// ---------- Compile-Time Feature Flags ----------
// Define these flags in your build configuration to enable the features:
// #define ENABLE_HTML_INTERFACE
// #define ENABLE_BRANDING
// #define ENABLE_SERIAL_MONITOR
// #define ENABLE_WEBSOCKETS
// #define ENABLE_OTA
// #define ENABLE_FS_EXPLORER
// #define ENABLE_BACKUP_RESTORE
// #define ENABLE_TERMINAL
// #define ENABLE_MULTI_CRED
// #define ENABLE_LOCALIZATION
// #define ENABLE_MDNS
// #define ENABLE_HTTPS
// #define ENABLE_AUTH
// ---------------------------------------------------

#if defined(ESP32) || defined(ESP32S2) || defined(ESP32S3) || defined(ESP32C2) || defined(ESP32C3) || defined(ESP32C5) || defined(ESP32C6)
  #define USING_ESP32
  #include <WiFi.h>
  #include <ESPmDNS.h>
  #include <SPIFFS.h>
  #include <AsyncTCP.h>
  #include <ESPAsyncWebServer.h>
#else
  #error "This library targets ESP32 family only."
#endif

#include <DNSServer.h>
#include <vector>
#include <functional>
#include "WiFiManagerParameter.h"

// Build modes removed; always provide full UI+API via feature flags.

#ifdef ENABLE_HTTPS
  #include <WiFiClientSecure.h>
  #if __has_include(<AsyncWebServerSecure.h>)
    #include <AsyncWebServerSecure.h>
    #define HAS_ASYNC_WEBSERVER_SECURE
  #else
    #warning "AsyncWebServerSecure.h not found. HTTPS support will be disabled."
    #undef ENABLE_HTTPS
  #endif
#endif

#ifdef ENABLE_WEBSOCKETS
  #include <AsyncWebSocket.h>
#endif

// Network scan result structure
struct WiFiNetwork {
  String ssid;
  int32_t rssi;
  uint8_t encryptionType;
};

// ---------- Configuration Structure ----------
struct WiFiManagerConfig {
  uint16_t httpPort = 80;
  unsigned long connectTimeout = 10000;       // in milliseconds
  unsigned long configPortalTimeout = 180000;   // in milliseconds
  bool autoReconnect = true;
#ifdef ENABLE_AUTH
  bool useAuth = false;
  String portalUsername = "";
  String portalPassword = "";
#endif
#ifdef ENABLE_SERIAL_MONITOR
  bool enableSerialMonitor = false;
  unsigned int serialMonitorBufferSize = 5000;
#endif
};

/// Structure for multi-credential support.
#ifdef ENABLE_MULTI_CRED
struct WiFiCredential {
  String ssid;
  String password;
};
#endif

class WiFiManager {
public:
  WiFiManager(const WiFiManagerConfig& config = WiFiManagerConfig());
  ~WiFiManager();

  // Initialization & main loop.
  void begin();
  void loop();

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
#ifdef ENABLE_HTML_INTERFACE
  void setCustomHeadElement(const char* html);
  void setCustomBodyFooter(const char* html);
#endif

  // mDNS support.
#ifdef ENABLE_MDNS
  void setMDNSHostname(const char* hostname);
#endif

  // HTTPS support.
#ifdef ENABLE_HTTPS
  void setUseHTTPS(bool flag);
  void setSSLCredentials(const char* cert, const char* key);
#endif

  // Authentication support.
#ifdef ENABLE_AUTH
  void setAuthentication(bool enable, const char* username = "", const char* password = "");
  bool isAuthenticationEnabled() const;
#endif

  // Serial monitor support.
#ifdef ENABLE_SERIAL_MONITOR
  void enableSerialMonitor(bool enable, unsigned int bufferSize = 5000);
  bool isSerialMonitorEnabled() const;
  String getSerialMonitorBuffer() const;
#endif

  // JSON API endpoints and network scan.
  std::vector<WiFiNetwork> scanNetworks(bool forceScan = false);

  // Status methods.
  String getConnectionStatus();
  uint8_t getLastConxResult();

  // Advanced endpoints:
#ifdef ENABLE_OTA
  void handleOTA(AsyncWebServerRequest *request);
#endif
#ifdef ENABLE_FS_EXPLORER
  void handleFSList(AsyncWebServerRequest *request);
  void handleFSUpload(AsyncWebServerRequest *request); // via onUpload callback
  void handleFSDelete(AsyncWebServerRequest *request);
#endif
#ifdef ENABLE_BACKUP_RESTORE
  void handleBackup(AsyncWebServerRequest *request);
  void handleRestore(AsyncWebServerRequest *request);
#endif
  void handleDeviceInfo(AsyncWebServerRequest *request);
#ifdef ENABLE_TERMINAL
  void handleTerminal(AsyncWebServerRequest *request);
#endif

  // Multi-credential support.
#ifdef ENABLE_MULTI_CRED
  bool addWiFiCredential(const char* ssid, const char* password);
#endif

  // Localization support.
#ifdef ENABLE_LOCALIZATION
  void setLanguage(const char* lang);
#endif

#ifdef ENABLE_WEBSOCKETS
  AsyncWebSocket* getWebSocket();
#endif

private:
  WiFiManagerConfig _config;
  AsyncWebServer* _server; // HTTP/HTTPS server pointer.
  DNSServer _dnsServer;
  std::vector<WiFiManagerParameter*> _params;
#ifdef ENABLE_MULTI_CRED
  std::vector<WiFiCredential> _wifiCredentials;
#endif
  std::function<void(WiFiManager*)> _apCallback;
  std::function<void()> _saveConfigCallback;
  std::function<void()> _configPortalTimeoutCallback;
  bool _debug;
  Print* _debugPort;
#ifdef ENABLE_HTML_INTERFACE
  String _customHeadElement;
  String _customBodyFooter;
#endif
#ifdef ENABLE_MDNS
  bool _useMDNS;
  String _mdnsHostname;
#endif
#ifdef ENABLE_HTTPS
  bool _useHTTPS;
  String _sslCert;
  String _sslKey;
#endif
  unsigned long _configPortalStart;
  uint8_t _lastConxResult;
#ifdef ENABLE_AUTH
  // Authentication variables.
  bool _useAuth;
  String _portalUsername;
  String _portalPassword;
#endif
#ifdef ENABLE_SERIAL_MONITOR
  bool _enableSerialMonitor;
  String _serialMonitorBuffer;
  unsigned int _serialMonitorBufferSize;
  unsigned long _lastSerialUpdate;
#endif
#ifdef ENABLE_LOCALIZATION
  String _language;  // e.g., "en", "es", etc.
#endif
#ifdef ENABLE_WEBSOCKETS
  AsyncWebSocket* _ws;
#endif

  // Helper functions.
  String getInputTypeString(ParameterType type);
  
  // Internal HTTP handlers.
  void handleRoot(AsyncWebServerRequest *request);
  void handleScan(AsyncWebServerRequest *request);
  void handleConnect(AsyncWebServerRequest *request);
  void handleReset(AsyncWebServerRequest *request);
  void handleNotFound(AsyncWebServerRequest *request);
  void handleStatusJSON(AsyncWebServerRequest *request);
  void handleParamsJSON(AsyncWebServerRequest *request);
  void handleUpdateParams(AsyncWebServerRequest *request);

  // Authentication helper.
#ifdef ENABLE_AUTH
  bool checkAuthentication(AsyncWebServerRequest *request);
#else
  bool checkAuthentication(AsyncWebServerRequest *request) { return true; }
#endif

  // Internal helper methods.
  void debug(String msg);
  void startDNS();
  bool startAPMode(const char* apName, const char* apPassword);
};

#endif // WIFI_MANAGER_H
