#include <Arduino.h>
#include <unity.h>
#include "WiFiManager.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>

WiFiManagerConfig config;
WiFiManager wifiManager(config);

void setUp(void) {
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(1000);
    
    // Initialize SPIFFS for file operations
    if(!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
}

void tearDown(void) {
    // Clean up after each test
    wifiManager.resetSettings();
}

// Test that branding.json file exists and is valid JSON
void test_branding_file_exists() {
    TEST_ASSERT_TRUE(SPIFFS.exists("/branding.json"));
    
    File file = SPIFFS.open("/branding.json", "r");
    TEST_ASSERT_TRUE(file);
    
    String content = file.readString();
    file.close();
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, content);
    TEST_ASSERT_FALSE(error);
    
    // Verify required sections exist
    TEST_ASSERT_TRUE(doc.containsKey("brand"));
    TEST_ASSERT_TRUE(doc.containsKey("theme"));
    TEST_ASSERT_TRUE(doc.containsKey("portal"));
    TEST_ASSERT_TRUE(doc.containsKey("ui"));
}

// Test that dark-mode.css file exists
void test_dark_mode_css_exists() {
    TEST_ASSERT_TRUE(SPIFFS.exists("/dark-mode.css"));
    
    File file = SPIFFS.open("/dark-mode.css", "r");
    TEST_ASSERT_TRUE(file);
    String content = file.readString();
    file.close();
    
    // Verify file contains dark mode styles
    TEST_ASSERT_TRUE(content.indexOf(".dark-mode") >= 0);
}

// Test that the index.html file contains theme toggle button
void test_theme_toggle_exists() {
    TEST_ASSERT_TRUE(SPIFFS.exists("/index.html"));
    
    File file = SPIFFS.open("/index.html", "r");
    TEST_ASSERT_TRUE(file);
    String content = file.readString();
    file.close();
    
    // Verify theme toggle button exists
    TEST_ASSERT_TRUE(content.indexOf("theme-toggle") >= 0);
    // Verify dark mode CSS is included
    TEST_ASSERT_TRUE(content.indexOf("dark-mode.css") >= 0);
}

// Test that script.js contains theme switching functionality
void test_theme_switching_functionality() {
    TEST_ASSERT_TRUE(SPIFFS.exists("/script.js"));
    
    File file = SPIFFS.open("/script.js", "r");
    TEST_ASSERT_TRUE(file);
    String content = file.readString();
    file.close();
    
    // Verify theme functions exist
    TEST_ASSERT_TRUE(content.indexOf("applyTheme") >= 0);
    TEST_ASSERT_TRUE(content.indexOf("initializeTheme") >= 0);
    TEST_ASSERT_TRUE(content.indexOf("loadBrandingConfig") >= 0);
    
    // Verify auto theme detection
    TEST_ASSERT_TRUE(content.indexOf("prefers-color-scheme") >= 0);
}

// Test white labeling functionality
void test_white_labeling() {
    // Modify branding.json to test white labeling
    DynamicJsonDocument doc(1024);
    doc["brand"]["name"] = "TestBrand";
    doc["brand"]["version"] = "2.0.0";
    doc["theme"]["primary_color"] = "#FF0000";
    doc["portal"]["subtitle"] = "Test Subtitle";
    
    File file = SPIFFS.open("/branding.json", "w");
    TEST_ASSERT_TRUE(file);
    serializeJson(doc, file);
    file.close();
    
    // In a real test, we would now load the web interface and verify
    // that the branding elements are correctly displayed
    // Since we can't do that in a unit test, we'll just verify the file was written
    file = SPIFFS.open("/branding.json", "r");
    TEST_ASSERT_TRUE(file);
    String content = file.readString();
    file.close();
    
    TEST_ASSERT_TRUE(content.indexOf("TestBrand") >= 0);
    TEST_ASSERT_TRUE(content.indexOf("2.0.0") >= 0);
    TEST_ASSERT_TRUE(content.indexOf("#FF0000") >= 0);
    TEST_ASSERT_TRUE(content.indexOf("Test Subtitle") >= 0);
}

// Test theme persistence
void test_theme_persistence() {
    // This would normally test localStorage persistence
    // Since we can't do that in a unit test, we'll just verify the code exists
    TEST_ASSERT_TRUE(SPIFFS.exists("/script.js"));
    
    File file = SPIFFS.open("/script.js", "r");
    TEST_ASSERT_TRUE(file);
    String content = file.readString();
    file.close();
    
    // Verify localStorage is used for theme persistence
    TEST_ASSERT_TRUE(content.indexOf("localStorage.getItem('theme')") >= 0);
    TEST_ASSERT_TRUE(content.indexOf("localStorage.setItem('theme')") >= 0);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_branding_file_exists);
    RUN_TEST(test_dark_mode_css_exists);
    RUN_TEST(test_theme_toggle_exists);
    RUN_TEST(test_theme_switching_functionality);
    RUN_TEST(test_white_labeling);
    RUN_TEST(test_theme_persistence);
    UNITY_END();
}

void loop() {
    // Empty loop
}