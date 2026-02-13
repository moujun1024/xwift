#include "xwift/stdlib/JSON/JSON.h"
#include <sstream>
#include <iomanip>

namespace xwift {
namespace json {

JSONValue::JSONValue() : data(std::monostate()) {}

JSONValue::JSONValue(bool value) : data(value) {}

JSONValue::JSONValue(double value) : data(value) {}

JSONValue::JSONValue(const std::string& value) : data(value) {}

JSONValue::JSONValue(const std::vector<JSONValue>& value) : data(value) {}

JSONValue::JSONValue(const std::map<std::string, JSONValue>& value) : data(value) {}

JSONType JSONValue::getType() const {
  if (std::get_if<std::monostate>(&data)) return JSONType::Null;
  if (std::get_if<bool>(&data)) return JSONType::Boolean;
  if (std::get_if<double>(&data)) return JSONType::Number;
  if (std::get_if<std::string>(&data)) return JSONType::String;
  if (std::get_if<std::vector<JSONValue>>(&data)) return JSONType::Array;
  if (std::get_if<std::map<std::string, JSONValue>>(&data)) return JSONType::Object;
  return JSONType::Null;
}

std::optional<bool> JSONValue::asBool() const {
  if (auto val = std::get_if<bool>(&data)) return *val;
  return std::nullopt;
}

std::optional<double> JSONValue::asDouble() const {
  if (auto val = std::get_if<double>(&data)) return *val;
  return std::nullopt;
}

std::optional<int64_t> JSONValue::asInt() const {
  if (auto val = std::get_if<double>(&data)) return static_cast<int64_t>(*val);
  return std::nullopt;
}

std::optional<std::string> JSONValue::asString() const {
  if (auto val = std::get_if<std::string>(&data)) return *val;
  if (auto val = std::get_if<double>(&data)) return std::to_string(*val);
  if (auto val = std::get_if<bool>(&data)) return *val ? "true" : "false";
  return std::nullopt;
}

std::optional<std::vector<JSONValue>> JSONValue::asArray() const {
  if (auto val = std::get_if<std::vector<JSONValue>>(&data)) return *val;
  return std::nullopt;
}

std::optional<std::map<std::string, JSONValue>> JSONValue::asObject() const {
  if (auto val = std::get_if<std::map<std::string, JSONValue>>(&data)) return *val;
  return std::nullopt;
}

bool JSONValue::has(const std::string& key) const {
  if (auto obj = std::get_if<std::map<std::string, JSONValue>>(&data)) {
    return obj->find(key) != obj->end();
  }
  return false;
}

JSONValue JSONValue::get(const std::string& key) const {
  if (auto obj = std::get_if<std::map<std::string, JSONValue>>(&data)) {
    auto it = obj->find(key);
    if (it != obj->end()) return it->second;
  }
  return JSONValue();
}

JSONValue JSONValue::get(size_t index) const {
  if (auto arr = std::get_if<std::vector<JSONValue>>(&data)) {
    if (index < arr->size()) return (*arr)[index];
  }
  return JSONValue();
}

std::string JSONValue::toString() const {
  std::ostringstream oss;
  
  switch (getType()) {
    case JSONType::Null:
      oss << "null";
      break;
    case JSONType::Boolean:
      if (auto val = std::get_if<bool>(&data)) {
        oss << (*val ? "true" : "false");
      }
      break;
    case JSONType::Number:
      if (auto val = std::get_if<double>(&data)) {
        oss << *val;
      }
      break;
    case JSONType::String:
      if (auto val = std::get_if<std::string>(&data)) {
        oss << "\"" << jsonEscape(*val) << "\"";
      }
      break;
    case JSONType::Array:
      if (auto arr = std::get_if<std::vector<JSONValue>>(&data)) {
        oss << "[";
        for (size_t i = 0; i < arr->size(); i++) {
          if (i > 0) oss << ",";
          oss << (*arr)[i].toString();
        }
        oss << "]";
      }
      break;
    case JSONType::Object:
      if (auto obj = std::get_if<std::map<std::string, JSONValue>>(&data)) {
        oss << "{";
        bool first = true;
        for (const auto& pair : *obj) {
          if (!first) oss << ",";
          oss << "\"" << jsonEscape(pair.first) << "\":" << pair.second.toString();
          first = false;
        }
        oss << "}";
      }
      break;
  }
  
  return oss.str();
}

JSONParser::JSONParser() : pos(0), errorLine(0), errorColumn(0) {}

JSONParser::~JSONParser() {}

JSONValue JSONParser::parse(const std::string& jsonStr) {
  json = jsonStr;
  pos = 0;
  error = "";
  errorLine = 0;
  errorColumn = 0;
  
  skipWhitespace();
  if (pos >= json.length()) {
    setError("Empty JSON string");
    return JSONValue();
  }
  
  parsedValue = parseValue();
  if (!error.empty()) return JSONValue();
  
  skipWhitespace();
  if (pos < json.length()) {
    setError("Unexpected characters at end of JSON");
    return JSONValue();
  }
  
  return parsedValue;
}

bool JSONParser::hasError() const {
  return !error.empty();
}

std::string JSONParser::getError() const {
  return error;
}

size_t JSONParser::getErrorLine() const {
  return errorLine;
}

size_t JSONParser::getErrorColumn() const {
  return errorColumn;
}

bool JSONParser::has(const std::string& key) const {
  return parsedValue.has(key);
}

std::string JSONParser::get(const std::string& key) const {
  JSONValue value = parsedValue.get(key);
  if (auto str = value.asString()) {
    return *str;
  }
  return value.toString();
}

void JSONParser::skipWhitespace() {
  while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' || 
         json[pos] == '\n' || json[pos] == '\r')) {
    if (json[pos] == '\n') {
      errorLine++;
      errorColumn = 0;
    } else {
      errorColumn++;
    }
    pos++;
  }
}

char JSONParser::peek() {
  if (pos < json.length()) return json[pos];
  return '\0';
}

char JSONParser::consume() {
  if (pos < json.length()) {
    char c = json[pos];
    pos++;
    if (c == '\n') {
      errorLine++;
      errorColumn = 0;
    } else {
      errorColumn++;
    }
    return c;
  }
  return '\0';
}

JSONValue JSONParser::parseValue() {
  skipWhitespace();
  char c = peek();
  
  if (c == '{') return parseObject();
  if (c == '[') return parseArray();
  if (c == '"') return parseString();
  if (c == 't' || c == 'f') return parseBoolean();
  if (c == 'n') return parseNull();
  if (c == '-' || (c >= '0' && c <= '9')) return parseNumber();
  
  setError("Unexpected character: " + std::string(1, c));
  return JSONValue();
}

JSONValue JSONParser::parseObject() {
  consume();
  skipWhitespace();
  
  std::map<std::string, JSONValue> obj;
  
  if (peek() == '}') {
    consume();
    return JSONValue(obj);
  }
  
  while (true) {
    skipWhitespace();
    if (peek() != '"') {
      setError("Expected '\"' in object");
      return JSONValue();
    }
    
    JSONValue keyVal = parseString();
    if (hasError()) return JSONValue();
    
    auto keyOpt = keyVal.asString();
    if (!keyOpt) {
      setError("Object key must be a string");
      return JSONValue();
    }
    std::string key = *keyOpt;
    
    skipWhitespace();
    if (peek() != ':') {
      setError("Expected ':' after key");
      return JSONValue();
    }
    consume();
    
    skipWhitespace();
    JSONValue value = parseValue();
    if (hasError()) return JSONValue();
    
    obj[key] = value;
    
    skipWhitespace();
    if (peek() == '}') {
      consume();
      return JSONValue(obj);
    }
    if (peek() != ',') {
      setError("Expected ',' or '}' in object");
      return JSONValue();
    }
    consume();
  }
}

JSONValue JSONParser::parseArray() {
  consume();
  skipWhitespace();
  
  std::vector<JSONValue> arr;
  
  if (peek() == ']') {
    consume();
    return JSONValue(arr);
  }
  
  while (true) {
    skipWhitespace();
    JSONValue value = parseValue();
    if (hasError()) return JSONValue();
    
    arr.push_back(value);
    
    skipWhitespace();
    if (peek() == ']') {
      consume();
      return JSONValue(arr);
    }
    if (peek() != ',') {
      setError("Expected ',' or ']' in array");
      return JSONValue();
    }
    consume();
  }
}

JSONValue JSONParser::parseString() {
  consume();
  
  std::string result;
  while (pos < json.length()) {
    char c = consume();
    if (c == '"') {
      return JSONValue(jsonUnescape(result));
    }
    if (c == '\\') {
      if (pos >= json.length()) {
        setError("Unterminated escape sequence");
        return JSONValue();
      }
      char next = consume();
      switch (next) {
        case '"': result += '"'; break;
        case '\\': result += '\\'; break;
        case '/': result += '/'; break;
        case 'b': result += '\b'; break;
        case 'f': result += '\f'; break;
        case 'n': result += '\n'; break;
        case 'r': result += '\r'; break;
        case 't': result += '\t'; break;
        case 'u': {
          if (pos + 4 > json.length()) {
            setError("Invalid unicode escape");
            return JSONValue();
          }
          std::string hex = json.substr(pos, 4);
          pos += 4;
          try {
            int code = std::stoi(hex, nullptr, 16);
            result += static_cast<char>(code);
          } catch (...) {
            setError("Invalid unicode escape");
            return JSONValue();
          }
          break;
        }
        default:
          setError("Invalid escape sequence");
          return JSONValue();
      }
    } else {
      result += c;
    }
  }
  
  setError("Unterminated string");
  return JSONValue();
}

JSONValue JSONParser::parseNumber() {
  size_t start = pos;
  size_t startLine = errorLine;
  size_t startColumn = errorColumn;
  
  if (peek() == '-') consume();
  
  if (peek() == '0') {
    consume();
  } else if (peek() >= '1' && peek() <= '9') {
    while (peek() >= '0' && peek() <= '9') consume();
  } else {
    setError("Invalid number");
    return JSONValue();
  }
  
  if (peek() == '.') {
    consume();
    if (!(peek() >= '0' && peek() <= '9')) {
      setError("Invalid number");
      return JSONValue();
    }
    while (peek() >= '0' && peek() <= '9') consume();
  }
  
  if (peek() == 'e' || peek() == 'E') {
    consume();
    if (peek() == '+' || peek() == '-') consume();
    if (!(peek() >= '0' && peek() <= '9')) {
      setError("Invalid number");
      return JSONValue();
    }
    while (peek() >= '0' && peek() <= '9') consume();
  }
  
  std::string numStr = json.substr(start, pos - start);
  double value = parseDouble(numStr);
  return JSONValue(value);
}

double JSONParser::parseDouble(const std::string& str) {
  double result = 0.0;
  int sign = 1;
  size_t i = 0;
  
  if (!str.empty() && str[0] == '-') {
    sign = -1;
    i++;
  }
  
  while (i < str.length() && str[i] >= '0' && str[i] <= '9') {
    result = result * 10.0 + (str[i] - '0');
    i++;
  }
  
  if (i < str.length() && str[i] == '.') {
    i++;
    double fraction = 0.0;
    double divisor = 10.0;
    while (i < str.length() && str[i] >= '0' && str[i] <= '9') {
      fraction += (str[i] - '0') / divisor;
      divisor *= 10.0;
      i++;
    }
    result += fraction;
  }
  
  if (i < str.length() && (str[i] == 'e' || str[i] == 'E')) {
    i++;
    int expSign = 1;
    if (i < str.length() && (str[i] == '+' || str[i] == '-')) {
      if (str[i] == '-') expSign = -1;
      i++;
    }
    int exponent = 0;
    while (i < str.length() && str[i] >= '0' && str[i] <= '9') {
      exponent = exponent * 10 + (str[i] - '0');
      i++;
    }
    if (expSign > 0) {
      while (exponent > 0) {
        result *= 10.0;
        exponent--;
      }
    } else {
      while (exponent > 0) {
        result /= 10.0;
        exponent--;
      }
    }
  }
  
  return sign * result;
}

JSONValue JSONParser::parseBoolean() {
  if (json.substr(pos, 4) == "true") {
    pos += 4;
    return JSONValue(true);
  }
  if (json.substr(pos, 5) == "false") {
    pos += 5;
    return JSONValue(false);
  }
  setError("Invalid boolean value");
  return JSONValue();
}

JSONValue JSONParser::parseNull() {
  if (json.substr(pos, 4) == "null") {
    pos += 4;
    return JSONValue();
  }
  setError("Invalid null value");
  return JSONValue();
}

void JSONParser::setError(const std::string& msg) {
  error = msg;
}

std::string jsonEscape(const std::string& str) {
  std::ostringstream oss;
  for (size_t i = 0; i < str.length(); i++) {
    unsigned char c = str[i];
    
    if (c > 127) {
      throw std::runtime_error("JSON library currently only supports ASCII characters. Non-ASCII character found at position " + std::to_string(i));
    }
    
    switch (c) {
      case '"': oss << "\\\""; break;
      case '\\': oss << "\\\\"; break;
      case '\b': oss << "\\b"; break;
      case '\f': oss << "\\f"; break;
      case '\n': oss << "\\n"; break;
      case '\r': oss << "\\r"; break;
      case '\t': oss << "\\t"; break;
      default:
        if (c < 32) {
          oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
        } else {
          oss << c;
        }
        break;
    }
  }
  return oss.str();
}

std::string jsonUnescape(const std::string& str) {
  std::string result;
  for (size_t i = 0; i < str.length(); i++) {
    if (str[i] == '\\' && i + 1 < str.length()) {
      char next = str[i + 1];
      switch (next) {
        case '"': result += '"'; i++; break;
        case '\\': result += '\\'; i++; break;
        case '/': result += '/'; i++; break;
        case 'b': result += '\b'; i++; break;
        case 'f': result += '\f'; i++; break;
        case 'n': result += '\n'; i++; break;
        case 'r': result += '\r'; i++; break;
        case 't': result += '\t'; i++; break;
        case 'u':
          if (i + 5 < str.length()) {
            std::string hex = str.substr(i + 2, 4);
            try {
              unsigned int code = std::stoul(hex, nullptr, 16);
              if (code > 127) {
                throw std::runtime_error("JSON library currently only supports ASCII characters. Non-ASCII Unicode escape \\u" + hex + " found");
              }
              result += static_cast<char>(code);
            } catch (...) {
              throw std::runtime_error("Invalid Unicode escape sequence: \\u" + hex);
            }
            i += 5;
          }
          break;
        default:
          result += str[i];
          break;
      }
    } else {
      result += str[i];
    }
  }
  return result;
}

}
}
