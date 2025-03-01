#include "WiFiManager.h"
#include <Arduino.h>
#include <cstring>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#ifdef USE_HTTPS
  // Using AsyncWebServer with HTTPS support
  #include <AsyncWebServerSecure.h>
#endif

// ----- WiFiManagerParameter Implementation -----
WiFiManagerParameter::WiFiManagerParameter(const char* id, const char* label, const char* defaultValue, int length, const char* customHTML, int labelPlacement)
  : _id(id), _label(label), _value(defaultValue), _length(length), _customHTML(customHTML), _labelPlacement(labelPlacement) {}

const char* WiFiManagerParameter::getID() const { return _id.c_str(); }
const char* WiFiManagerParameter::getLabel() const { return _label.c_str(); }
const char* WiFiManagerParameter::getValue() const { return _value.c_str(); }
void WiFiManagerParameter::setValue(const char* value) { _value = value; }
const char* WiFiManagerParameter::getCustomHTML() const { return _customHTML.c_str(); }
int WiFiManagerParameter::getLabelPlacement() const { return _labelPlacement; }

// ----- WiFiManager Implementation -----
WiFiManager::WiFiManager(const WiFiManagerConfig& config)
  : _config(config), _server(nullptr), _debug(true), _debugPort(&Serial),
    _customHeadElement(""), _customBodyFooter(""), _useMDNS(false), _mdnsHostname("esp32"),
    _useHTTPS(false), _configPortalStart(0), _lastConxResult(WL_IDLE_STATUS)
#ifdef USE_HTTPS
    , _sslCert(""), _sslKey("")
#endif
{}

WiFiManager::~WiFiManager() {
  if (_server) {
    delete _server;
  }
  for (auto p : _params) {
    delete p;
  }
}

void WiFiManager::debug(String msg) {
  if (_debug && _debugPort) {
    _debugPort->println("*wm: " + msg);
  }
}

void WiFiManager::begin() {
#ifdef USE_HTTPS
  if (_useHTTPS) {
    _server = new AsyncWebServerSecure(_config.httpPort);
    // Set SSL certificates if available
    if (_sslCert.length() > 0 && _sslKey.length() > 0) {
      // Note: In a production environment, you would use proper certificate handling
      // This is just a placeholder for the implementation
      debug("Using HTTPS secure server with certificates.");
    }
  } else {
    _server = new AsyncWebServer(_config.httpPort);
  }
#else
  _server = new AsyncWebServer(_config.httpPort);
#endif

  // Initialize SPIFFS for serving web files
  if (!SPIFFS.begin(true)) {
    debug("An error occurred while mounting SPIFFS");
  } else {
    debug("SPIFFS mounted successfully");
  }

  startDNS();

  // Set up HTTP endpoints with AsyncWebServer
  _server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) { 
    handleRoot(request); 
  });
  
  _server->on("/scan", HTTP_GET, [this](AsyncWebServerRequest *request) { 
    handleScan(request); 
  });
  
  _server->on("/connect", HTTP_POST, [this](AsyncWebServerRequest *request) { 
    handleConnect(request); 
  });
  
  _server->on("/reset", HTTP_GET, [this](AsyncWebServerRequest *request) { 
    handleReset(request); 
  });
  
  _server->on("/status_json", HTTP_GET, [this](AsyncWebServerRequest *request) { 
    handleStatusJSON(request); 
  });
  
  _server->on("/params_json", HTTP_GET, [this](AsyncWebServerRequest *request) { 
    handleParamsJSON(request); 
  });
  
  // Serve static files from SPIFFS
  _server->serveStatic("/", SPIFFS, "/");
  
  _server->onNotFound([this](AsyncWebServerRequest *request) { 
    handleNotFound(request); 
  });
  
  _server->begin();
  debug("Async HTTP server started on port " + String(_config.httpPort));

#ifdef ESP32
  if (_useMDNS) {
    if (MDNS.begin(_mdnsHostname.c_str())) {
      debug("mDNS responder started as " + _mdnsHostname);
      MDNS.addService("http", "tcp", _config.httpPort);
    } else {
      debug("Error starting mDNS responder!");
    }
  }
#endif
}

void WiFiManager::loop() {
  // With AsyncWebServer, we don't need to call handleClient()
  // Just process DNS requests
  _dnsServer.processNextRequest();
  
  // Check for config portal timeout
  if (_configPortalStart && (millis() - _configPortalStart > _config.configPortalTimeout)) {
    debug("Config portal timeout reached.");
    if (_configPortalTimeoutCallback) {
      _configPortalTimeoutCallback();
    }
    stopConfigPortal();
  }
}

bool WiFiManager::autoConnect(const char* apName, const char* apPassword) {
  WiFi.mode(WIFI_STA);
  if (WiFi.SSID() != "") {
    debug("Attempting connection to saved AP: " + WiFi.SSID());
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < _config.connectTimeout) {
      delay(500);
    }
    _lastConxResult = WiFi.status();
    if (WiFi.status() == WL_CONNECTED) {
      debug("Connected to saved AP.");
      return true;
    }
  }
  debug("No saved credentials or connection failed. Starting config portal.");
  return startConfigPortal(apName ? apName : "ESP_Config", apPassword);
}

bool WiFiManager::startConfigPortal(const char* apName, const char* apPassword) {
  WiFi.mode(WIFI_AP_STA);
  if (!startAPMode(apName, apPassword)) {
    debug("Failed to start AP mode.");
    return false;
  }
  _configPortalStart = millis();
  if (_apCallback) {
    _apCallback(this);
  }
  debug("Config portal running...");
  while (WiFi.status() != WL_CONNECTED && (millis() - _configPortalStart < _config.configPortalTimeout)) {
    loop();
    delay(10);
  }
  if (WiFi.status() != WL_CONNECTED) {
    debug("Config portal timed out without connection.");
    return false;
  }
  if (_saveConfigCallback) {
    _saveConfigCallback();
  }
  return true;
}

void WiFiManager::stopConfigPortal() {
  _server->stop();
  _dnsServer.stop();
  _configPortalStart = 0;
  debug("Config portal stopped.");
}

bool WiFiManager::connectToNetwork(const char* ssid, const char* password) {
  WiFi.begin(ssid, password);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < _config.connectTimeout) {
    delay(500);
  }
  _lastConxResult = WiFi.status();
  return (WiFi.status() == WL_CONNECTED);
}

bool WiFiManager::disconnectFromNetwork() {
  WiFi.disconnect();
  return (WiFi.status() != WL_CONNECTED);
}

void WiFiManager::resetSettings() {
  debug("Resetting settings.");
  WiFi.disconnect(true);
}

void WiFiManager::setAPStaticIPConfig(IPAddress ip, IPAddress gateway, IPAddress subnet) {
  WiFi.softAPConfig(ip, gateway, subnet);
  debug("AP static IP config set.");
}

void WiFiManager::setSTAStaticIPConfig(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns) {
  WiFi.config(ip, gateway, subnet, dns);
  debug("STA static IP config set.");
}

bool WiFiManager::addParameter(WiFiManagerParameter* param) {
  _params.push_back(param);
  debug("Parameter added: " + String(param->getID()));
  return true;
}

std::vector<WiFiManagerParameter*> WiFiManager::getParameters() const {
  return _params;
}

void WiFiManager::setDebugOutput(bool debug, Print& debugPort) {
  _debug = debug;
  _debugPort = &debugPort;
}

void WiFiManager::setAPCallback(std::function<void(WiFiManager*)> callback) {
  _apCallback = callback;
}

void WiFiManager::setSaveConfigCallback(std::function<void()> callback) {
  _saveConfigCallback = callback;
}

void WiFiManager::setConfigPortalTimeoutCallback(std::function<void()> callback) {
  _configPortalTimeoutCallback = callback;
}

void WiFiManager::setCustomHeadElement(const char* html) {
  _customHeadElement = html;
}

void WiFiManager::setCustomBodyFooter(const char* html) {
  _customBodyFooter = html;
}

void WiFiManager::setMDNSHostname(const char* hostname) {
  _mdnsHostname = hostname;
  _useMDNS = true;
}

#ifdef USE_HTTPS
void WiFiManager::setSSLCredentials(const char* cert, const char* key) {
  _sslCert = cert;
  _sslKey = key;
}
#endif

void WiFiManager::setUseHTTPS(bool flag) {
  _useHTTPS = flag;
}

String WiFiManager::getConnectionStatus() {
  switch(WiFi.status()){
    case WL_CONNECTED: return "Connected";
    case WL_CONNECT_FAILED: return "Connect Failed";
    case WL_NO_SSID_AVAIL: return "No SSID Available";
    case WL_IDLE_STATUS: return "Idle";
    default: return "Unknown";
  }
}

uint8_t WiFiManager::getLastConxResult() {
  return _lastConxResult;
}

// ----- New: scanNetworks() Implementation ----- 
std::vector<WiFiNetwork> WiFiManager::scanNetworks(bool forceScan) {
  std::vector<WiFiNetwork> networks;
  int n = WiFi.scanNetworks(forceScan);
  for (int i = 0; i < n; i++) {
    WiFiNetwork net;
    net.ssid = WiFi.SSID(i);
    net.rssi = WiFi.RSSI(i);
    net.encryptionType = WiFi.encryptionType(i);
    networks.push_back(net);
  }
  debug(String(n) + " networks found.");
  return networks;
}

// ----- Internal HTTP Handlers -----
void WiFiManager::handleRoot(AsyncWebServerRequest *request) {
  String page = "<html><head>";
  if (_customHeadElement.length() > 0)
    page += _customHeadElement;
  page += "<meta charset='utf-8'><title>ESP32 WiFi Manager</title></head><body>";
  page += "<h1>ESP32 WiFi Manager</h1>";
  page += "<p>Current Status: " + getConnectionStatus() + "</p>";
  page += "</body></html>";
  request->send(200, "text/html", page);
}

void WiFiManager::handleScan(AsyncWebServerRequest *request) {
  auto networks = scanNetworks(true);
  String json = "[";
  for (size_t i = 0; i < networks.size(); i++) {
    json += "{";
    json += "\"ssid\":\"" + networks[i].ssid + "\",";
    json += "\"rssi\":" + String(networks[i].rssi) + ",";
    json += "\"encryptionType\":" + String(networks[i].encryptionType);
    json += "}";
    if (i < networks.size()-1)
      json += ",";
  }
  json += "]";
  request->send(200, "application/json", json);
}

void WiFiManager::handleConnect(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_POST) {
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
      String ssid = request->getParam("ssid", true)->value();
      String password = request->getParam("password", true)->value();
      debug("Connecting to new AP: " + ssid);
      bool connected = connectToNetwork(ssid.c_str(), password.c_str());
      if (connected)
        request->send(200, "application/json", "{\"result\":\"Connected\"}");
      else
        request->send(500, "application/json", "{\"result\":\"Connection Failed\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Missing parameters\"}");
    }
  } else {
    request->send(405, "application/json", "{\"error\":\"Method Not Allowed\"}");
  }
}

void WiFiManager::handleReset(AsyncWebServerRequest *request) {
  resetSettings();
  request->send(200, "application/json", "{\"result\":\"Settings reset\"}");
}

void WiFiManager::handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "application/json", "{\"error\":\"Not found\"}");
}

// ----- JSON API Handlers -----
void WiFiManager::handleStatusJSON(AsyncWebServerRequest *request) {
  String json = "{";
  json += "\"status\":\"" + getConnectionStatus() + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"lastResult\":" + String(getLastConxResult());
  json += "}";
  request->send(200, "application/json", json);
}

void WiFiManager::handleParamsJSON(AsyncWebServerRequest *request) {
  String json = "[";
  for (size_t i = 0; i < _params.size(); i++) {
    json += "{";
    json += "\"id\":\"" + String(_params[i]->getID()) + "\",";
    json += "\"label\":\"" + String(_params[i]->getLabel()) + "\",";
    json += "\"value\":\"" + String(_params[i]->getValue()) + "\",";
    
    // Add parameter type to the JSON response
    String typeStr = "text";
    switch(_params[i]->getType()) {
      case ParameterType::PASSWORD: typeStr = "password"; break;
      case ParameterType::NUMBER: typeStr = "number"; break;
      case ParameterType::TOGGLE: typeStr = "checkbox"; break;
      case ParameterType::SLIDER: typeStr = "range"; break;
      case ParameterType::SELECT: typeStr = "select"; break;
      case ParameterType::EMAIL: typeStr = "email"; break;
      case ParameterType::URL: typeStr = "url"; break;
      case ParameterType::SEARCH: typeStr = "search"; break;
      case ParameterType::TEL: typeStr = "tel"; break;
      case ParameterType::DATE: typeStr = "date"; break;
      case ParameterType::TIME: typeStr = "time"; break;
      case ParameterType::DATETIME_LOCAL: typeStr = "datetime-local"; break;
      case ParameterType::MONTH: typeStr = "month"; break;
      case ParameterType::WEEK: typeStr = "week"; break;
      case ParameterType::COLOR: typeStr = "color"; break;
      case ParameterType::FILE: typeStr = "file"; break;
      case ParameterType::HIDDEN: typeStr = "hidden"; break;
      case ParameterType::TEXTAREA: typeStr = "textarea"; break;
      default: typeStr = "text";
    }
    json += "\"type\":\"" + typeStr + "\",";
    
    // Add custom attributes if available
    json += "\"attributes\":\"" + String(_params[i]->getCustomAttributes()) + "\"";
    
    json += "}";
    if (i < _params.size()-1)
      json += ",";
  }
  json += "]";
  request->send(200, "application/json", json);
}
}

// ----- Internal Helper Methods -----
void WiFiManager::startDNS() {
  _dnsServer.start(53, "*", WiFi.softAPIP());
}

bool WiFiManager::startAPMode(const char* apName, const char* apPassword) {
  bool result;
  if (apPassword && strlen(apPassword) >= 8) {
    result = WiFi.softAP(apName, apPassword);
  } else {
    result = WiFi.softAP(apName);
  }
  delay(1000); // Allow the AP to initialize.
  return result;
}
