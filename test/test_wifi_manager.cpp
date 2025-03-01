#include <Arduino.h>
#include <unity.h>
#include "WiFiManager.h"

WiFiManagerConfig config;
WiFiManager wifiManager(config);

void setUp(void) {
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(1000);
}

void tearDown(void) {
    // Clean up after each test
    wifiManager.resetSettings();
}

void test_wifi_manager_initialization() {
    TEST_ASSERT_NOT_NULL(&wifiManager);
}

void test_wifi_manager_config_portal() {
    // Test that config portal starts successfully
    bool portalStarted = wifiManager.startConfigPortal("TEST_AP", "test123");
    TEST_ASSERT_TRUE(portalStarted);
    
    // Verify AP is active
    TEST_ASSERT_EQUAL(WiFi.getMode(), WIFI_AP_STA);
    
    // Test portal timeout
    delay(config.configPortalTimeout + 100);
    TEST_ASSERT_FALSE(wifiManager.autoConnect());
}

void test_wifi_manager_connection() {
    // Test connection with invalid credentials
    bool connected = wifiManager.connectToNetwork("invalid_ssid", "invalid_pass");
    TEST_ASSERT_FALSE(connected);
    
    // Test connection status
    String status = wifiManager.getConnectionStatus();
    TEST_ASSERT_TRUE(status == "Connect Failed" || status == "No SSID Available");
}

void test_wifi_manager_parameters() {
    // Test adding custom parameters
    WiFiManagerParameter* param = new WiFiManagerParameter("test_param", "Test Parameter", "default", 40);
    TEST_ASSERT_TRUE(wifiManager.addParameter(param));
    
    // Verify parameter was added
    auto params = wifiManager.getParameters();
    TEST_ASSERT_EQUAL(1, params.size());
    TEST_ASSERT_EQUAL_STRING("test_param", params[0]->getID());
    
    delete param;
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_wifi_manager_initialization);
    RUN_TEST(test_wifi_manager_config_portal);
    RUN_TEST(test_wifi_manager_connection);
    RUN_TEST(test_wifi_manager_parameters);
    UNITY_END();
}

void loop() {
    // Empty loop
}