#ifndef WIFI_MANAGER_PARAMETER_H
#define WIFI_MANAGER_PARAMETER_H

#include <Arduino.h>

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

class WiFiManagerParameter {
public:
    // Constructor for basic text parameter
    WiFiManagerParameter(const char* id, const char* label, const char* defaultValue, int length,
                        const char* customHTML = "", int labelPlacement = 1);
    
    // Constructor for advanced parameter types
    WiFiManagerParameter(const char* id, const char* label, const char* defaultValue,
                        ParameterType type, const char* customAttributes = "");
    
    // Getters
    const char* getID() const;
    const char* getLabel() const;
    const char* getValue() const;
    ParameterType getType() const;
    const char* getCustomAttributes() const;
    const char* getCustomHTML() const;
    int getLabelPlacement() const;
    bool isValid() const;
    
    // Setters
    void setValue(const char* value);
    void setCustomAttributes(const char* attributes);
    
    // Validation
    void setValidation(std::function<bool(const char*)> validator);
    
    // HTML Generation
    String getHTML() const;
    
private:
    String _id;
    String _label;
    String _value;
    ParameterType _type;
    String _customAttributes;
    String _customHTML;
    int _length;
    int _labelPlacement;
    std::function<bool(const char*)> _validator;
    
    // Helper methods
    bool validateValue(const char* value) const;
    String generateInputHTML() const;
};

#endif // WIFI_MANAGER_PARAMETER_H