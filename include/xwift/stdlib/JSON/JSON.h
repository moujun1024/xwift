#ifndef XWIFT_JSON_JSON_H
#define XWIFT_JSON_JSON_H

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <memory>
#include <optional>

namespace xwift {
namespace json {

enum class JSONType {
  Null,
  Boolean,
  Number,
  String,
  Array,
  Object
};

class JSONValue {
public:
  JSONValue();
  explicit JSONValue(bool value);
  explicit JSONValue(double value);
  explicit JSONValue(const std::string& value);
  explicit JSONValue(const std::vector<JSONValue>& value);
  explicit JSONValue(const std::map<std::string, JSONValue>& value);
  
  JSONType getType() const;
  
  std::optional<bool> asBool() const;
  std::optional<double> asDouble() const;
  std::optional<int64_t> asInt() const;
  std::optional<std::string> asString() const;
  std::optional<std::vector<JSONValue>> asArray() const;
  std::optional<std::map<std::string, JSONValue>> asObject() const;
  
  bool has(const std::string& key) const;
  JSONValue get(const std::string& key) const;
  JSONValue get(size_t index) const;
  
  std::string toString() const;
  
private:
  std::variant<std::monostate, bool, double, std::string, 
              std::vector<JSONValue>, std::map<std::string, JSONValue>> data;
};

class JSONParser {
public:
  JSONParser();
  ~JSONParser();
  
  JSONValue parse(const std::string& json);
  bool hasError() const;
  std::string getError() const;
  size_t getErrorLine() const;
  size_t getErrorColumn() const;
  
  bool has(const std::string& key) const;
  std::string get(const std::string& key) const;
  
private:
  std::string json;
  size_t pos;
  std::string error;
  size_t errorLine;
  size_t errorColumn;
  JSONValue parsedValue;
  
  void skipWhitespace();
  char peek();
  char consume();
  
  JSONValue parseValue();
  JSONValue parseObject();
  JSONValue parseArray();
  JSONValue parseString();
  JSONValue parseNumber();
  JSONValue parseBoolean();
  JSONValue parseNull();
  
  void setError(const std::string& msg);
  double parseDouble(const std::string& str);
};

std::string jsonEscape(const std::string& str);
std::string jsonUnescape(const std::string& str);

}
}

#endif
