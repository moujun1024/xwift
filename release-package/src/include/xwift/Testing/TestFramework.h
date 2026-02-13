#ifndef XWIFT_TESTING_TESTFRAMEWORK_H
#define XWIFT_TESTING_TESTFRAMEWORK_H

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <iostream>
#include <sstream>
#include <chrono>

namespace xwift {
namespace testing {

enum class TestResult {
  Passed,
  Failed,
  Skipped
};

struct TestInfo {
  std::string name;
  std::string suite;
  std::string file;
  int line;
  std::chrono::milliseconds duration;
  TestResult result;
  std::string message;
  
  TestInfo() : line(0), duration(0), result(TestResult::Passed) {}
};

class Assertion {
public:
  Assertion(bool condition, const std::string& message, 
           const std::string& file, int line)
    : passed(condition), message(message), file(file), line(line) {}
  
  bool isPassed() const { return passed; }
  std::string getMessage() const { return message; }
  std::string getFile() const { return file; }
  int getLine() const { return line; }
  
private:
  bool passed;
  std::string message;
  std::string file;
  int line;
};

class TestSuite {
public:
  TestSuite(const std::string& name) : name(name), passed(0), failed(0), skipped(0) {}
  
  void addTest(const std::string& testName, 
               std::function<void()> testFunc,
               const std::string& file = "", int line = 0) {
    tests.push_back({testName, testFunc, file, line});
  }
  
  void run() {
    std::cout << "Running test suite: " << name << "\n";
    std::cout << std::string(name.length() + 20, '=') << "\n\n";
    
    for (auto& test : tests) {
      auto start = std::chrono::high_resolution_clock::now();
      
      TestInfo info;
      info.name = test.name;
      info.suite = name;
      info.file = test.file;
      info.line = test.line;
      
      try {
        test.func();
        info.result = TestResult::Passed;
        passed++;
        std::cout << "[PASS] " << test.name << "\n";
      } catch (const std::exception& e) {
        info.result = TestResult::Failed;
        info.message = e.what();
        failed++;
        std::cout << "[FAIL] " << test.name << ": " << e.what() << "\n";
      } catch (...) {
        info.result = TestResult::Failed;
        info.message = "Unknown exception";
        failed++;
        std::cout << "[FAIL] " << test.name << ": Unknown exception\n";
      }
      
      auto end = std::chrono::high_resolution_clock::now();
      info.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
      results.push_back(info);
    }
    
    std::cout << "\n" << std::string(name.length() + 20, '=') << "\n";
    std::cout << "Tests run: " << tests.size() << "\n";
    std::cout << "Passed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    std::cout << "Skipped: " << skipped << "\n\n";
  }
  
  int getPassedCount() const { return passed; }
  int getFailedCount() const { return failed; }
  int getSkippedCount() const { return skipped; }
  const std::vector<TestInfo>& getResults() const { return results; }
  
private:
  struct Test {
    std::string name;
    std::function<void()> func;
    std::string file;
    int line;
  };
  
  std::string name;
  std::vector<Test> tests;
  std::vector<TestInfo> results;
  int passed;
  int failed;
  int skipped;
};

class TestRunner {
public:
  static TestRunner& getInstance() {
    static TestRunner instance;
    return instance;
  }
  
  void addSuite(std::unique_ptr<TestSuite> suite) {
    suites.push_back(std::move(suite));
  }
  
  void runAll() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "       XWift Test Framework\n";
    std::cout << "========================================\n\n";
    
    totalPassed = 0;
    totalFailed = 0;
    totalSkipped = 0;
    
    for (auto& suite : suites) {
      suite->run();
      totalPassed += suite->getPassedCount();
      totalFailed += suite->getFailedCount();
      totalSkipped += suite->getSkippedCount();
    }
    
    std::cout << "========================================\n";
    std::cout << "           Summary\n";
    std::cout << "========================================\n";
    std::cout << "Total tests: " << (totalPassed + totalFailed + totalSkipped) << "\n";
    std::cout << "Total passed: " << totalPassed << "\n";
    std::cout << "Total failed: " << totalFailed << "\n";
    std::cout << "Total skipped: " << totalSkipped << "\n";
    
    if (totalFailed > 0) {
      std::cout << "\nFailed tests:\n";
      for (auto& suite : suites) {
        for (const auto& result : suite->getResults()) {
          if (result.result == TestResult::Failed) {
            std::cout << "  - " << result.suite << "::" << result.name;
            if (!result.file.empty()) {
              std::cout << " (" << result.file << ":" << result.line << ")";
            }
            std::cout << "\n";
            if (!result.message.empty()) {
              std::cout << "    " << result.message << "\n";
            }
          }
        }
      }
    }
    
    std::cout << "\n";
  }
  
  int getTotalPassed() const { return totalPassed; }
  int getTotalFailed() const { return totalFailed; }
  int getTotalSkipped() const { return totalSkipped; }
  
private:
  TestRunner() : totalPassed(0), totalFailed(0), totalSkipped(0) {}
  
  std::vector<std::unique_ptr<TestSuite>> suites;
  int totalPassed;
  int totalFailed;
  int totalSkipped;
};

class Assert {
public:
  static void isTrue(bool condition, const std::string& message = "",
                     const std::string& file = "", int line = 0) {
    if (!condition) {
      std::string msg = message.empty() ? "Expected true, got false" : message;
      throw std::runtime_error(msg + " at " + file + ":" + std::to_string(line));
    }
  }
  
  static void isFalse(bool condition, const std::string& message = "",
                      const std::string& file = "", int line = 0) {
    if (condition) {
      std::string msg = message.empty() ? "Expected false, got true" : message;
      throw std::runtime_error(msg + " at " + file + ":" + std::to_string(line));
    }
  }
  
  static void equals(int expected, int actual, const std::string& message = "",
                    const std::string& file = "", int line = 0) {
    if (expected != actual) {
      std::ostringstream oss;
      oss << "Expected " << expected << ", got " << actual;
      std::string msg = message.empty() ? oss.str() : message + " (" + oss.str() + ")";
      throw std::runtime_error(msg + " at " + file + ":" + std::to_string(line));
    }
  }
  
  static void equals(const std::string& expected, const std::string& actual,
                    const std::string& message = "",
                    const std::string& file = "", int line = 0) {
    if (expected != actual) {
      std::ostringstream oss;
      oss << "Expected \"" << expected << "\", got \"" << actual << "\"";
      std::string msg = message.empty() ? oss.str() : message + " (" + oss.str() + ")";
      throw std::runtime_error(msg + " at " + file + ":" + std::to_string(line));
    }
  }
  
  static void throws(std::function<void()> func, const std::string& message = "",
                    const std::string& file = "", int line = 0) {
    bool threw = false;
    try {
      func();
    } catch (...) {
      threw = true;
    }
    
    if (!threw) {
      std::string msg = message.empty() ? "Expected exception to be thrown" : message;
      throw std::runtime_error(msg + " at " + file + ":" + std::to_string(line));
    }
  }
};

#define XWIFT_TEST(suiteName, testName) \
  void test_##suiteName##_##testName(); \
  struct RegisterTest_##suiteName##_##testName { \
    RegisterTest_##suiteName##_##testName() { \
      auto& runner = xwift::testing::TestRunner::getInstance(); \
      static auto suite = std::make_unique<xwift::testing::TestSuite>(#suiteName); \
      suite->addTest(#testName, test_##suiteName##_##testName, __FILE__, __LINE__); \
      runner.addSuite(std::move(suite)); \
    } \
  }; \
  RegisterTest_##suiteName##_##testName register_##suiteName##_##testName; \
  void test_##suiteName##_##testName()

#define XWIFT_ASSERT_TRUE(condition) \
  xwift::testing::Assert::isTrue(condition, "", __FILE__, __LINE__)

#define XWIFT_ASSERT_FALSE(condition) \
  xwift::testing::Assert::isFalse(condition, "", __FILE__, __LINE__)

#define XWIFT_ASSERT_EQ(expected, actual) \
  xwift::testing::Assert::equals(expected, actual, "", __FILE__, __LINE__)

#define XWIFT_ASSERT_THROWS(expr) \
  xwift::testing::Assert::throws([&]() { expr; }, "", __FILE__, __LINE__)

}
}

#endif
