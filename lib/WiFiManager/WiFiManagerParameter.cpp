#include "WiFiManagerParameter.h"

// Constructor for basic text parameter.
WiFiManagerParameter::WiFiManagerParameter(const char* id, const char* label, const char* defaultValue, int length,
                         const char* customHTML, int labelPlacement, const char* group)
    : _id(id), _label(label), _value(defaultValue), _length(length),
      _customHTML(customHTML), _customAttributes(""), _labelPlacement(labelPlacement),
      _type(ParameterType::TEXT), _group(group)
{}

// Constructor for advanced parameter types.
WiFiManagerParameter::WiFiManagerParameter(const char* id, const char* label, const char* defaultValue,
                         ParameterType type, const char* customAttributes, const char* group)
    : _id(id), _label(label), _value(defaultValue), _type(type),
      _customAttributes(customAttributes), _length(0), _labelPlacement(1), _group(group)
{}

const char* WiFiManagerParameter::getID() const { return _id.c_str(); }
const char* WiFiManagerParameter::getLabel() const { return _label.c_str(); }
const char* WiFiManagerParameter::getValue() const { return _value.c_str(); }
ParameterType WiFiManagerParameter::getType() const { return _type; }
const char* WiFiManagerParameter::getCustomAttributes() const { return _customAttributes.c_str(); }
const char* WiFiManagerParameter::getCustomHTML() const { return _customHTML.c_str(); }
int WiFiManagerParameter::getLabelPlacement() const { return _labelPlacement; }
const char* WiFiManagerParameter::getGroup() const { return _group.c_str(); }

void WiFiManagerParameter::setValue(const char* value) {
    if (validateValue(value)) { _value = value; }
}

void WiFiManagerParameter::setCustomAttributes(const char* attributes) {
    _customAttributes = attributes;
}

void WiFiManagerParameter::setValidation(std::function<bool(const char*)> validator) {
    _validator = validator;
}

bool WiFiManagerParameter::isValid() const {
    return validateValue(_value.c_str());
}

bool WiFiManagerParameter::validateValue(const char* value) const {
    if (_validator) return _validator(value);
    switch (_type) {
        case ParameterType::NUMBER:
            for (const char* p = value; *p; p++) { if (*p != '-' && *p != '.' && (*p < '0' || *p > '9')) return false; }
            return true;
        case ParameterType::TOGGLE:
            return strcmp(value, "true") == 0 || strcmp(value, "false") == 0;
        case ParameterType::SLIDER:
            for (const char* p = value; *p; p++) { if (*p != '.' && (*p < '0' || *p > '9')) return false; }
            return true;
        case ParameterType::EMAIL: {
            const char* at = strchr(value, '@');
            if (!at) return false;
            const char* dot = strchr(at, '.');
            return dot != nullptr && dot > at;
        }
        case ParameterType::URL:
            return strncmp(value, "http://", 7) == 0 || strncmp(value, "https://", 8) == 0;
        case ParameterType::TEL:
            for (const char* p = value; *p; p++) { if (!isdigit(*p) && *p != ' ' && *p != '-' && *p != '(' && *p != ')' && *p != '+') return false; }
            return true;
        case ParameterType::DATE:
        case ParameterType::TIME:
        case ParameterType::DATETIME_LOCAL:
        case ParameterType::MONTH:
        case ParameterType::WEEK:
            return true;
        case ParameterType::COLOR: {
            if (*value != '#') return false;
            size_t len = strlen(value);
            return (len == 7 || len == 4);
        }
        default:
            return true;
    }
}

String WiFiManagerParameter::generateInputHTML() const {
    String html = "<div class='form-field'>";
    if (_labelPlacement == 1) { html += "<label for='" + _id + "'>" + _label + "</label>"; }
    switch (_type) {
        case ParameterType::PASSWORD:
            html += "<input type='password' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
        case ParameterType::NUMBER:
            html += "<input type='number' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
        case ParameterType::TOGGLE:
            html += "<input type='checkbox' id='" + _id + "' name='" + _id + "'";
            if (_value == "true") html += " checked";
            break;
        case ParameterType::SLIDER:
            html += "<input type='range' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
        case ParameterType::SELECT:
            html += "<select id='" + _id + "' name='" + _id + "'>" + _customAttributes + "</select>";
            break;
        case ParameterType::EMAIL:
            html += "<input type='email' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
        case ParameterType::URL:
            html += "<input type='url' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
        case ParameterType::SEARCH:
            html += "<input type='search' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
        case ParameterType::TEL:
            html += "<input type='tel' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
        case ParameterType::DATE:
            html += "<input type='date' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
        case ParameterType::TIME:
            html += "<input type='time' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
        case ParameterType::DATETIME_LOCAL:
            html += "<input type='datetime-local' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
        case ParameterType::MONTH:
            html += "<input type='month' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
        case ParameterType::WEEK:
            html += "<input type='week' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
        case ParameterType::COLOR:
            html += "<input type='color' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
        case ParameterType::FILE:
            html += "<input type='file' id='" + _id + "' name='" + _id + "'";
            break;
        case ParameterType::HIDDEN:
            html += "<input type='hidden' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
        case ParameterType::TEXTAREA:
            html += "<textarea id='" + _id + "' name='" + _id + "'" + (_customAttributes.length() > 0 ? " " + _customAttributes : "") + ">" + _value + "</textarea>";
            return html + "</div>";
        default:
            html += "<input type='text' id='" + _id + "' name='" + _id + "' value='" + _value + "'";
            break;
    }
    if (_customAttributes.length() > 0 && _type != ParameterType::SELECT) { html += " " + _customAttributes; }
    if (_type != ParameterType::SELECT && _type != ParameterType::TEXTAREA) { html += ">"; }
    if (_labelPlacement == 2) { html += "<label for='" + _id + "'>" + _label + "</label>"; }
    html += "</div>";
    return html;
}

String WiFiManagerParameter::getHTML() const {
    return generateInputHTML();
}
