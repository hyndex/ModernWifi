#include "WiFiManager.h"
#include "WiFiManagerParameter.h"
#include <Arduino.h>
#include <cstring>

#ifdef ENABLE_HTTPS
  #include <AsyncWebServerSecure.h>
#endif

#ifdef ENABLE_WEBSOCKETS
  // WebSocket support.
#endif

// ----- Constructor & Destructor -----
WiFiManager::WiFiManager(const WiFiManagerConfig& config)
  : _config(config), _server(nullptr), _debug(true), _debugPort(&Serial),
#ifdef ENABLE_HTML_INTERFACE
    _customHeadElement(""), _customBodyFooter(""),
#endif
#ifdef ENABLE_MDNS
    _useMDNS(false), _mdnsHostname("esp32"),
#endif
#ifdef ENABLE_HTTPS
    _useHTTPS(false), _sslCert(""), _sslKey(""),
#endif
    _configPortalStart(0), _lastConxResult(WL_IDLE_STATUS)
#ifdef ENABLE_AUTH
    , _useAuth(config.useAuth), _portalUsername(config.portalUsername), _portalPassword(config.portalPassword)
#endif
#ifdef ENABLE_SERIAL_MONITOR
    , _enableSerialMonitor(config.enableSerialMonitor), _serialMonitorBuffer(""),
      _serialMonitorBufferSize(config.serialMonitorBufferSize), _lastSerialUpdate(0)
#endif
#ifdef ENABLE_LOCALIZATION
    , _language("en")
#endif
#ifdef ENABLE_WEBSOCKETS
    , _ws(nullptr)
#endif
{}

WiFiManager::~WiFiManager() {
  if (_server) {
    delete _server;
  }
  for (auto p : _params) {
    delete p;
  }
#ifdef ENABLE_WEBSOCKETS
  if (_ws) { delete _ws; }
#endif
}

// ----- Debugging -----
void WiFiManager::debug(String msg) {
  if (_debug && _debugPort) {
    _debugPort->println("*wm: " + msg);
  }
}

// ----- Initialization -----
void WiFiManager::begin() {
#if defined(ENABLE_HTTPS) && defined(HAS_ASYNC_WEBSERVER_SECURE)
  if (_useHTTPS) {
    _server = new AsyncWebServerSecure(_config.httpPort);
    if (_sslCert.length() > 0 && _sslKey.length() > 0) {
      debug("Using HTTPS secure server with certificates.");
    }
  } else {
    _server = new AsyncWebServer(_config.httpPort);
  }
#else
  _server = new AsyncWebServer(_config.httpPort);
#endif

  // Filesystem initialization.
  bool fsInitialized = false;
#ifdef USING_ESP32
  fsInitialized = SPIFFS.begin(true);
  if (!fsInitialized) {
    debug("Error mounting SPIFFS, attempting to format.");
    if (SPIFFS.format() && SPIFFS.begin(true)) {
      fsInitialized = true;
      debug("SPIFFS formatted and mounted successfully");
    } else {
      debug("SPIFFS format failed. Web interface may not work properly.");
    }
  } else {
    debug("SPIFFS mounted successfully");
  }
#elif defined(USING_RP2040)
  #if __has_include(<LittleFS.h>)
    fsInitialized = SPIFFS.begin();
    if (!fsInitialized) {
      debug("Error mounting LittleFS, attempting to format.");
      if (SPIFFS.format() && SPIFFS.begin()) {
        fsInitialized = true;
        debug("LittleFS formatted and mounted successfully");
      } else {
        debug("LittleFS format failed.");
      }
    } else {
      debug("LittleFS mounted successfully");
    }
  #else
    fsInitialized = SPIFFS.begin();
    if (!fsInitialized) { debug("Error mounting filesystem"); }
    else { debug("Filesystem mounted successfully"); }
  #endif
#elif defined(USING_AVR) || defined(USING_STM32) || defined(USING_NXP)
  fsInitialized = SPIFFS.begin();
  if (!fsInitialized) { debug("Error mounting filesystem"); }
  else { debug("Filesystem mounted successfully"); }
#endif

  startDNS();

  // ----- HTTP Endpoints -----
#ifdef ENABLE_HTML_INTERFACE
  _server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) { handleRoot(request); });
#else
  _server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) { handleStatusJSON(request); });
#endif
  _server->on("/scan", HTTP_GET, [this](AsyncWebServerRequest *request) { handleScan(request); });
  _server->on("/connect", HTTP_POST, [this](AsyncWebServerRequest *request) { handleConnect(request); });
  _server->on("/reset", HTTP_GET, [this](AsyncWebServerRequest *request) { handleReset(request); });
  _server->on("/status_json", HTTP_GET, [this](AsyncWebServerRequest *request) { handleStatusJSON(request); });
  _server->on("/params_json", HTTP_GET, [this](AsyncWebServerRequest *request) { handleParamsJSON(request); });
  _server->on("/update_params", HTTP_POST, [this](AsyncWebServerRequest *request) { handleUpdateParams(request); });
#ifdef ENABLE_OTA
  _server->on("/ota", HTTP_POST, [this](AsyncWebServerRequest *request) { handleOTA(request); });
#endif
#ifdef ENABLE_FS_EXPLORER
  _server->on("/fs/list", HTTP_GET, [this](AsyncWebServerRequest *request) { handleFSList(request); });
  _server->on("/fs/delete", HTTP_DELETE, [this](AsyncWebServerRequest *request) { handleFSDelete(request); });
  _server->on("/fs/upload", HTTP_POST, [](AsyncWebServerRequest *request) {},
    [this](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
      if(!index){
        debug("Upload Start: " + filename);
        SPIFFS.remove("/" + filename);
      }
      File file = SPIFFS.open("/" + filename, FILE_APPEND);
      if(file){ file.write(data, len); file.close(); }
      if(final){ debug("Upload Complete: " + filename); }
    }
  );
#endif
#ifdef ENABLE_BACKUP_RESTORE
  _server->on("/backup", HTTP_GET, [this](AsyncWebServerRequest *request) { handleBackup(request); });
  _server->on("/restore", HTTP_POST, [this](AsyncWebServerRequest *request) { handleRestore(request); });
#endif
  _server->on("/device_info", HTTP_GET, [this](AsyncWebServerRequest *request) { handleDeviceInfo(request); });
#ifdef ENABLE_TERMINAL
  _server->on("/terminal", HTTP_GET, [this](AsyncWebServerRequest *request) { handleTerminal(request); });
#endif

  // Serve static files.
  _server->serveStatic("/", SPIFFS, "/");
  _server->onNotFound([this](AsyncWebServerRequest *request) { handleNotFound(request); });
  
#ifdef ENABLE_WEBSOCKETS
  _ws = new AsyncWebSocket("/ws");
  _ws->onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                  void *arg, uint8_t *data, size_t len) {
    if(type == WS_EVT_CONNECT){
      Serial.println("WebSocket client connected");
    } else if(type == WS_EVT_DISCONNECT){
      Serial.println("WebSocket client disconnected");
    }
  });
  _server->addHandler(_ws);
#endif

  _server->begin();
  debug("HTTP server started on port " + String(_config.httpPort));

#ifdef ENABLE_MDNS
  #ifdef USING_ESP32
    if (_useMDNS) {
      if (MDNS.begin(_mdnsHostname.c_str())) {
        debug("mDNS responder started as " + _mdnsHostname);
        MDNS.addService("http", "tcp", _config.httpPort);
      } else { debug("Error starting mDNS responder!"); }
    }
  #else
    debug("mDNS not implemented on this platform");
  #endif
#endif
}

void WiFiManager::loop() {
  _dnsServer.processNextRequest();
#ifdef ENABLE_WEBSOCKETS
  if(_ws) _ws->cleanupClients();
#endif
  if (_configPortalStart && (millis() - _configPortalStart > _config.configPortalTimeout)) {
    debug("Config portal timeout reached.");
    if (_configPortalTimeoutCallback) { _configPortalTimeoutCallback(); }
    stopConfigPortal();
  }
}

// ----- Connection Management -----
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
#ifdef ENABLE_MULTI_CRED
  for (auto& cred : _wifiCredentials) {
    debug("Attempting connection to credential: " + cred.ssid);
    WiFi.begin(cred.ssid.c_str(), cred.password.c_str());
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < _config.connectTimeout) { delay(500); }
    _lastConxResult = WiFi.status();
    if (WiFi.status() == WL_CONNECTED) {
      debug("Connected using multi-credential: " + cred.ssid);
      return true;
    }
  }
#endif
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
  if (_apCallback) { _apCallback(this); }
  debug("Config portal running...");
  while (WiFi.status() != WL_CONNECTED && (millis() - _configPortalStart < _config.configPortalTimeout)) {
    loop();
    delay(10);
  }
  if (WiFi.status() != WL_CONNECTED) {
    debug("Config portal timed out without connection.");
    return false;
  }
  if (_saveConfigCallback) { _saveConfigCallback(); }
  return true;
}

void WiFiManager::stopConfigPortal() {
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

// ----- Static IP Configuration -----
void WiFiManager::setAPStaticIPConfig(IPAddress ip, IPAddress gateway, IPAddress subnet) {
  WiFi.softAPConfig(ip, gateway, subnet);
  debug("AP static IP config set.");
}

void WiFiManager::setSTAStaticIPConfig(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns) {
  WiFi.config(ip, gateway, subnet, dns);
  debug("STA static IP config set.");
}

// ----- Parameter Handling -----
bool WiFiManager::addParameter(WiFiManagerParameter* param) {
  _params.push_back(param);
  debug("Parameter added: " + String(param->getID()));
  return true;
}

std::vector<WiFiManagerParameter*> WiFiManager::getParameters() const {
  return _params;
}

// ----- Debug & Callback Setters -----
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

#ifdef ENABLE_HTML_INTERFACE
void WiFiManager::setCustomHeadElement(const char* html) {
  _customHeadElement = html;
}
void WiFiManager::setCustomBodyFooter(const char* html) {
  _customBodyFooter = html;
}
#endif

#ifdef ENABLE_MDNS
void WiFiManager::setMDNSHostname(const char* hostname) {
  _mdnsHostname = hostname;
  _useMDNS = true;
}
#endif

#ifdef ENABLE_HTTPS
void WiFiManager::setUseHTTPS(bool flag) {
  _useHTTPS = flag;
}
void WiFiManager::setSSLCredentials(const char* cert, const char* key) {
  _sslCert = cert;
  _sslKey = key;
}
#endif

#ifdef ENABLE_AUTH
void WiFiManager::setAuthentication(bool enable, const char* username, const char* password) {
  _useAuth = enable;
  if (enable) {
    _portalUsername = username;
    _portalPassword = password;
  }
}
bool WiFiManager::isAuthenticationEnabled() const {
  return _useAuth;
}
bool WiFiManager::checkAuthentication(AsyncWebServerRequest *request) {
  if (!_useAuth) return true;
  if (!request->authenticate(_portalUsername.c_str(), _portalPassword.c_str())) {
    request->requestAuthentication();
    return false;
  }
  return true;
}
#endif

#ifdef ENABLE_SERIAL_MONITOR
void WiFiManager::enableSerialMonitor(bool enable, unsigned int bufferSize) {
  _enableSerialMonitor = enable;
  _serialMonitorBufferSize = bufferSize;
  _serialMonitorBuffer.reserve(bufferSize);
  _lastSerialUpdate = millis();
  if (enable && _server) {
    _server->on("/serial", HTTP_GET, [this](AsyncWebServerRequest *request) { /* Serial monitor page code */ });
    _server->on("/serial_data", HTTP_GET, [this](AsyncWebServerRequest *request) { /* Serial data endpoint */ });
    debug("Serial monitor enabled");
  }
}
bool WiFiManager::isSerialMonitorEnabled() const {
  return _enableSerialMonitor;
}
String WiFiManager::getSerialMonitorBuffer() const {
  return _serialMonitorBuffer;
}
#endif

// ----- Status & Network Scanning -----
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
String WiFiManager::getInputTypeString(WiFiManagerParameterType type) {
  switch(type) {
    case WIFI_MANAGER_PARAM_TEXT: return "text";
    case WIFI_MANAGER_PARAM_PASSWORD: return "password";
    case WIFI_MANAGER_PARAM_NUMBER: return "number";
    case WIFI_MANAGER_PARAM_CHECKBOX: return "checkbox";
    case WIFI_MANAGER_PARAM_SELECT: return "select";
    case WIFI_MANAGER_PARAM_COLOR: return "color";
    case WIFI_MANAGER_PARAM_DATE: return "date";
    case WIFI_MANAGER_PARAM_TIME: return "time";
    case WIFI_MANAGER_PARAM_DATETIME: return "datetime-local";
    case WIFI_MANAGER_PARAM_EMAIL: return "email";
    case WIFI_MANAGER_PARAM_RANGE: return "range";
    default: return "text";
  }
}

// ----- Internal HTTP Handlers -----
#ifdef ENABLE_HTML_INTERFACE
void WiFiManager::handleRoot(AsyncWebServerRequest *request) {
  if (#ifdef ENABLE_AUTH
         !_useAuth || !checkAuthentication(request)
      #else
         false
      #endif) return;
  String page = "<html><head>";
#ifdef ENABLE_HTML_INTERFACE
  if (_customHeadElement.length() > 0) page += _customHeadElement;
#endif
  page += "<meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  page += "<title>WiFi Manager</title>";
  page += "<style>body{font-family:Arial,sans-serif;padding:20px;color:#333} .container{max-width:800px;margin:auto;padding:20px;background:#f9f9f9;border-radius:5px} .btn{padding:8px 16px;background:#0066cc;color:white;border:none;border-radius:4px;cursor:pointer} input,select,textarea{width:100%;padding:8px;margin:5px 0;border:1px solid #ddd;border-radius:4px;}</style>";
  page += "</head><body><div class='container'>";
  page += "<h1>WiFi Manager</h1>";
  page += "<p>Status: " + getConnectionStatus() + "</p>";
  page += "<h2>Available Networks</h2>";
  page += "<button class='btn' onclick='scanNetworks()'>Scan Networks</button>";
  page += "<div id='networks'></div>";
  page += "<h2>Connect to Network</h2>";
  page += "<form id='wifi-form'><label for='ssid'>SSID:</label><input type='text' id='ssid' name='ssid' required>";
  page += "<label for='password'>Password:</label><input type='password' id='password' name='password'>";
  page += "<button type='submit' class='btn'>Connect</button></form>";
  if (_params.size() > 0) {
    page += "<h2>Custom Fields</h2>";
    page += "<form id='custom-form'>";
    for (auto param : _params) {
      page += "<label for='" + String(param->getID()) + "'>" + String(param->getLabel());
#ifdef ENABLE_LOCALIZATION
      if (strlen(param->getGroup()) > 0) { page += " (" + String(param->getGroup()) + ")"; }
#else
      if (strlen(param->getGroup()) > 0) { page += " (" + String(param->getGroup()) + ")"; }
#endif
      page += ":</label>";
      page += "<input type='" + getInputTypeString(param->getType()) + "' id='" + String(param->getID()) +
              "' name='" + String(param->getID()) + "' value='" + String(param->getValue()) + "' " +
              String(param->getCustomAttributes()) + ">";
    }
    page += "<button type='submit' class='btn'>Save Custom Fields</button>";
    page += "</form>";
  }
  page += "<hr><h2>Advanced Tools</h2>";
  page += "<a href='/ota'>OTA Update</a> | ";
#ifdef ENABLE_FS_EXPLORER
  page += "<a href='/fs/list'>File Explorer</a> | ";
#endif
#ifdef ENABLE_BACKUP_RESTORE
  page += "<a href='/backup'>Backup Config</a> | ";
#endif
  page += "<a href='/device_info'>Device Info</a>";
#ifdef ENABLE_TERMINAL
  page += " | <a href='/terminal'>Terminal</a>";
#endif
  page += "</div>";
  page += "<script>";
  page += "function scanNetworks(){ document.getElementById('networks').innerHTML='Scanning...';";
  page += "fetch('/scan').then(r=>r.json()).then(data=>{ let html='<ul>'; data.forEach(n=>{ html+='<li><a href=\"#\" onclick=\"document.getElementById(\\'ssid\\').value=\\''+n.ssid+'\\'\">'+n.ssid+' ('+n.rssi+'dBm)</a></li>'; }); html+='</ul>'; document.getElementById('networks').innerHTML=html; }); }";
  page += "document.getElementById('wifi-form').onsubmit=function(e){ e.preventDefault(); let formData=new FormData(e.target);";
  page += "fetch('/connect',{method:'POST',body:formData}).then(r=>r.json()).then(data=>{ alert(data.result || 'Connected!'); }).catch(err=>{ alert('Connection failed'); }); };";
  page += "if(document.getElementById('custom-form')){ document.getElementById('custom-form').onsubmit=function(e){ e.preventDefault();";
  page += "let formData=new FormData(e.target); fetch('/update_params',{method:'POST',body:formData}).then(r=>r.json()).then(data=>{ alert(data.result || 'Updated'); }).catch(err=>{ alert('Update failed'); }); }; }";
  page += "</script></body></html>";
  request->send(200, "text/html", page);
}
#else
void WiFiManager::handleRoot(AsyncWebServerRequest *request) {
  handleStatusJSON(request);
}
#endif

void WiFiManager::handleScan(AsyncWebServerRequest *request) {
  if (/*if authentication enabled*/ false) { if(!checkAuthentication(request)) return; }
  auto networks = scanNetworks(true);
  String json = "[";
  for (size_t i = 0; i < networks.size(); i++) {
    json += "{";
    json += "\"ssid\":\"" + networks[i].ssid + "\",";
    json += "\"rssi\":" + String(networks[i].rssi) + ",";
    json += "\"encryptionType\":" + String(networks[i].encryptionType);
    json += "}";
    if (i < networks.size()-1) json += ",";
  }
  json += "]";
  request->send(200, "application/json", json);
}

void WiFiManager::handleConnect(AsyncWebServerRequest *request) {
  if(/*if authentication enabled*/ false){ if(!checkAuthentication(request)) return; }
  if (request->method() == HTTP_POST) {
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
      String ssid = request->getParam("ssid", true)->value();
      String password = request->getParam("password", true)->value();
      debug("Connecting to AP: " + ssid);
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
  if(/*if authentication enabled*/ false){ if(!checkAuthentication(request)) return; }
  resetSettings();
  request->send(200, "application/json", "{\"result\":\"Settings reset\"}");
}

void WiFiManager::handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "application/json", "{\"error\":\"Not found\"}");
}

void WiFiManager::handleStatusJSON(AsyncWebServerRequest *request) {
  if(/*if authentication enabled*/ false){ if(!checkAuthentication(request)) return; }
  String json = "{";
  json += "\"status\":\"" + getConnectionStatus() + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"lastResult\":" + String(getLastConxResult()) + ",";
  json += "\"params\":[";
  for (size_t i = 0; i < _params.size(); i++) {
    json += "{";
    json += "\"id\":\"" + String(_params[i]->getID()) + "\",";
    json += "\"label\":\"" + String(_params[i]->getLabel()) + "\",";
    json += "\"value\":\"" + String(_params[i]->getValue()) + "\"";
#ifdef ENABLE_LOCALIZATION
    json += ",\"group\":\"" + String(_params[i]->getGroup()) + "\"";
#endif
    json += "}";
    if (i < _params.size()-1) json += ",";
  }
  json += "]";
  json += "}";
  request->send(200, "application/json", json);
}

void WiFiManager::handleParamsJSON(AsyncWebServerRequest *request) {
  if(/*if authentication enabled*/ false){ if(!checkAuthentication(request)) return; }
  String json = "[";
  for (size_t i = 0; i < _params.size(); i++) {
    json += "{";
    json += "\"id\":\"" + String(_params[i]->getID()) + "\",";
    json += "\"label\":\"" + String(_params[i]->getLabel()) + "\",";
    json += "\"value\":\"" + String(_params[i]->getValue()) + "\"";
#ifdef ENABLE_LOCALIZATION
    json += ",\"group\":\"" + String(_params[i]->getGroup()) + "\"";
#endif
    json += "}";
    if (i < _params.size()-1) json += ",";
  }
  json += "]";
  request->send(200, "application/json", json);
}

void WiFiManager::handleUpdateParams(AsyncWebServerRequest *request) {
  if(/*if authentication enabled*/ false){ if(!checkAuthentication(request)) return; }
  if (request->method() == HTTP_POST) {
    bool updated = false;
    for (size_t i = 0; i < _params.size(); i++) {
      if (request->hasParam(_params[i]->getID(), true)) {
        String newValue = request->getParam(_params[i]->getID(), true)->value();
        _params[i]->setValue(newValue.c_str());
        updated = true;
        debug("Updated param: " + String(_params[i]->getID()) + " to " + newValue);
      }
    }
    if (updated)
      request->send(200, "application/json", "{\"result\":\"Custom fields updated\"}");
    else
      request->send(400, "application/json", "{\"error\":\"No parameters updated\"}");
  } else {
    request->send(405, "application/json", "{\"error\":\"Method Not Allowed\"}");
  }
}

#ifdef ENABLE_OTA
void WiFiManager::handleOTA(AsyncWebServerRequest *request) {
  if(/*if authentication enabled*/ false){ if(!checkAuthentication(request)) return; }
  debug("OTA update initiated (stub).");
  request->send(200, "application/json", "{\"result\":\"OTA update initiated (stub)\"}");
}
#endif

#ifdef ENABLE_FS_EXPLORER
void WiFiManager::handleFSList(AsyncWebServerRequest *request) {
  if(/*if authentication enabled*/ false){ if(!checkAuthentication(request)) return; }
  String fileList = "[";
  Dir dir = SPIFFS.openDir("/");
  bool first = true;
  while (dir.next()) {
    if (!first) fileList += ",";
    fileList += "{\"name\":\"" + dir.fileName() + "\",\"size\":" + String(dir.fileSize()) + "}";
    first = false;
  }
  fileList += "]";
  request->send(200, "application/json", fileList);
}

void WiFiManager::handleFSDelete(AsyncWebServerRequest *request) {
  if(/*if authentication enabled*/ false){ if(!checkAuthentication(request)) return; }
  if (request->hasParam("path")) {
    String path = request->getParam("path")->value();
    if (SPIFFS.exists(path)) {
      SPIFFS.remove(path);
      request->send(200, "application/json", "{\"result\":\"File deleted\"}");
      debug("Deleted file: " + path);
    } else {
      request->send(404, "application/json", "{\"error\":\"File not found\"}");
    }
  } else {
    request->send(400, "application/json", "{\"error\":\"Missing path parameter\"}");
  }
}
#endif

#ifdef ENABLE_BACKUP_RESTORE
void WiFiManager::handleBackup(AsyncWebServerRequest *request) {
  if(/*if authentication enabled*/ false){ if(!checkAuthentication(request)) return; }
  String backup = "{";
#ifdef ENABLE_MULTI_CRED
  backup += "\"wifi_credentials\":[";
  for (size_t i = 0; i < _wifiCredentials.size(); i++) {
    backup += "{\"ssid\":\"" + _wifiCredentials[i].ssid + "\",\"password\":\"" + _wifiCredentials[i].password + "\"}";
    if (i < _wifiCredentials.size()-1) backup += ",";
  }
  backup += "],";
#endif
  backup += "\"parameters\":[";
  for (size_t i = 0; i < _params.size(); i++) {
    backup += "{\"id\":\"" + String(_params[i]->getID()) + "\",\"label\":\"" + String(_params[i]->getLabel()) +
              "\",\"value\":\"" + String(_params[i]->getValue()) + "\"";
#ifdef ENABLE_LOCALIZATION
    backup += ",\"group\":\"" + String(_params[i]->getGroup()) + "\"";
#endif
    backup += "}";
    if (i < _params.size()-1) backup += ",";
  }
  backup += "]";
  backup += "}";
  request->send(200, "application/json", backup);
}

void WiFiManager::handleRestore(AsyncWebServerRequest *request) {
  if(/*if authentication enabled*/ false){ if(!checkAuthentication(request)) return; }
  if (request->hasParam("backup", true)) {
    String backup = request->getParam("backup", true)->value();
    debug("Restore configuration (stub): " + backup);
    request->send(200, "application/json", "{\"result\":\"Restore initiated (stub)\"}");
  } else {
    request->send(400, "application/json", "{\"error\":\"Missing backup data\"}");
  }
}
#endif

void WiFiManager::handleDeviceInfo(AsyncWebServerRequest *request) {
  if(/*if authentication enabled*/ false){ if(!checkAuthentication(request)) return; }
  String info = "{";
#ifdef ESP.getFreeHeap
  info += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
#endif
  info += "\"uptime_ms\":" + String(millis()) + ",";
  info += "\"rssi\":\"" + String(WiFi.RSSI()) + "\",";
  info += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  info += "}";
  request->send(200, "application/json", info);
}

#ifdef ENABLE_TERMINAL
void WiFiManager::handleTerminal(AsyncWebServerRequest *request) {
  if(/*if authentication enabled*/ false){ if(!checkAuthentication(request)) return; }
  String page = "<html><head><meta charset='utf-8'><title>Terminal</title></head><body>";
  page += "<h1>Terminal (stub)</h1>";
  page += "<p>This is a stub for an interactive terminal interface.</p>";
  page += "</body></html>";
  request->send(200, "text/html", page);
}
#endif

// ----- Helper: Start AP Mode -----
bool WiFiManager::startAPMode(const char* apName, const char* apPassword) {
  bool result;
  if (apPassword && strlen(apPassword) >= 8) result = WiFi.softAP(apName, apPassword);
  else result = WiFi.softAP(apName);
  delay(1000);
  return result;
}

#ifdef ENABLE_MULTI_CRED
bool WiFiManager::addWiFiCredential(const char* ssid, const char* password) {
  WiFiCredential cred;
  cred.ssid = ssid;
  cred.password = password;
  _wifiCredentials.push_back(cred);
  debug("Added WiFi credential: " + String(ssid));
  return true;
}
#endif

#ifdef ENABLE_LOCALIZATION
void WiFiManager::setLanguage(const char* lang) {
  _language = lang;
  debug("Language set to: " + _language);
}
#endif

#ifdef ENABLE_WEBSOCKETS
AsyncWebSocket* WiFiManager::getWebSocket() {
  return _ws;
}
#endif

// ----- Internal Helper Methods -----
void WiFiManager::startDNS() {
  _dnsServer.start(53, "*", WiFi.softAPIP());
}

void WiFiManager::debug(String msg) {
  if (_debug && _debugPort) { _debugPort->println("*wm: " + msg); }
}
