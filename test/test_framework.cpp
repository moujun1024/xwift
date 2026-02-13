#include "xwift/Testing/TestFramework.h"
#include "xwift/stdlib/JSON/JSON.h"
#include "xwift/Filesystem/Filesystem.h"
#include <fstream>

using namespace xwift::testing;

XWIFT_TEST(JSONParser, ParseSimpleObject) {
  json::JSONParser parser;
  json::JSONValue result = parser.parse("{\"name\":\"test\",\"value\":42}");
  
  XWIFT_ASSERT_TRUE(!parser.hasError());
  XWIFT_ASSERT_TRUE(result.has("name"));
  XWIFT_ASSERT_TRUE(result.has("value"));
}

XWIFT_TEST(JSONParser, ParseArray) {
  json::JSONParser parser;
  json::JSONValue result = parser.parse("[1,2,3,4,5]");
  
  XWIFT_ASSERT_TRUE(!parser.hasError());
  auto arr = result.asArray();
  XWIFT_ASSERT_TRUE(arr.has_value());
  XWIFT_ASSERT_EQ(5, arr->size());
}

XWIFT_TEST(JSONParser, ParseNestedObject) {
  json::JSONParser parser;
  json::JSONValue result = parser.parse("{\"user\":{\"name\":\"John\",\"age\":30}}");
  
  XWIFT_ASSERT_TRUE(!parser.hasError());
  XWIFT_ASSERT_TRUE(result.has("user"));
  
  auto user = result.get("user");
  XWIFT_ASSERT_TRUE(user.has("name"));
  XWIFT_ASSERT_TRUE(user.has("age"));
}

XWIFT_TEST(JSONParser, ParseStringWithEscapes) {
  json::JSONParser parser;
  json::JSONValue result = parser.parse("{\"text\":\"Hello\\nWorld\\t!\"}");
  
  XWIFT_ASSERT_TRUE(!parser.hasError());
  XWIFT_ASSERT_TRUE(result.has("text"));
  
  auto text = result.get("text").asString();
  XWIFT_ASSERT_TRUE(text.has_value());
  XWIFT_ASSERT_EQ("Hello\nWorld\t!", *text);
}

XWIFT_TEST(JSONParser, ParseInvalidJSON) {
  json::JSONParser parser;
  json::JSONValue result = parser.parse("{\"invalid\":}");
  
  XWIFT_ASSERT_TRUE(parser.hasError());
}

XWIFT_TEST(JSONValue, CustomTypeSerialization) {
  std::map<std::string, json::JSONValue> fields;
  fields["name"] = json::JSONValue("Test");
  fields["value"] = json::JSONValue(42.0);
  
  json::JSONValue custom = json::JSONValue::fromCustom("MyType", fields);
  auto type = custom.getCustomType();
  
  XWIFT_ASSERT_TRUE(type.has_value());
  XWIFT_ASSERT_EQ("MyType", *type);
  
  auto customFields = custom.getCustomFields();
  XWIFT_ASSERT_TRUE(customFields.find("name") != customFields.end());
  XWIFT_ASSERT_TRUE(customFields.find("value") != customFields.end());
}

XWIFT_TEST(FileSystem, PathNormalization) {
  std::string path1 = fs::FileSystem::normalizePath("a/b/../c");
  std::string path2 = fs::FileSystem::normalizePath("./test/../file.txt");
  
  XWIFT_ASSERT_TRUE(!path1.empty());
  XWIFT_ASSERT_TRUE(!path2.empty());
}

XWIFT_TEST(FileSystem, PathExtraction) {
  std::string path = "C:/Users/Test/Documents/file.txt";
  
  std::string dir = fs::FileSystem::getDirectoryName(path);
  std::string name = fs::FileSystem::getFileName(path);
  std::string ext = fs::FileSystem::getFileExtension(path);
  
  XWIFT_ASSERT_TRUE(!dir.empty());
  XWIFT_ASSERT_EQ("file.txt", name);
  XWIFT_ASSERT_EQ(".txt", ext);
}

XWIFT_TEST(FileSystem, FileOperations) {
  std::string testFile = "test_filesystem.tmp";
  std::string testContent = "Hello, World!";
  
  auto writeResult = fs::FileSystem::writeFile(testFile, testContent);
  XWIFT_ASSERT_TRUE(writeResult.success);
  
  XWIFT_ASSERT_TRUE(fs::FileSystem::exists(testFile));
  XWIFT_ASSERT_TRUE(fs::FileSystem::isFile(testFile));
  
  std::string readContent;
  auto readResult = fs::FileSystem::readFile(testFile, readContent);
  XWIFT_ASSERT_TRUE(readResult.success);
  XWIFT_ASSERT_EQ(testContent, readContent);
  
  int64_t fileSize = fs::FileSystem::getFileSize(testFile);
  XWIFT_ASSERT_EQ(testContent.length(), fileSize);
  
  auto deleteResult = fs::FileSystem::deleteFile(testFile);
  XWIFT_ASSERT_TRUE(deleteResult.success);
  
  XWIFT_ASSERT_FALSE(fs::FileSystem::exists(testFile));
}

XWIFT_TEST(FileSystem, InvalidPathHandling) {
  std::string invalidPath = "test<>|?*.txt";
  
  bool isValid = fs::FileSystem::isValidPath(invalidPath);
  XWIFT_ASSERT_FALSE(isValid);
}

int main() {
  auto& runner = TestRunner::getInstance();
  runner.runAll();
  
  return runner.getTotalFailed() > 0 ? 1 : 0;
}
