#ifndef XWIFT_LOGGING_LOGGER_H
#define XWIFT_LOGGING_LOGGER_H

#include <string>
#include <sstream>
#include <fstream>
#include <mutex>
#include <memory>
#include <queue>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <functional>

namespace xwift {
namespace logging {

enum class LogLevel {
  Trace,
  Debug,
  Info,
  Warning,
  Error,
  Fatal,
  Off
};

struct LogEntry {
  LogLevel level;
  std::string message;
  std::string file;
  int line;
  std::string function;
  std::chrono::system_clock::time_point timestamp;
  
  LogEntry() : line(0), timestamp(std::chrono::system_clock::now()) {}
};

class LogFormatter {
public:
  virtual std::string format(const LogEntry& entry) = 0;
  virtual ~LogFormatter() = default;
};

class SimpleFormatter : public LogFormatter {
public:
  std::string format(const LogEntry& entry) override;
};

class DetailedFormatter : public LogFormatter {
public:
  std::string format(const LogEntry& entry) override;
};

class JsonFormatter : public LogFormatter {
public:
  std::string format(const LogEntry& entry) override;
};

class LogAppender {
public:
  virtual void append(const LogEntry& entry) = 0;
  virtual void flush() = 0;
  virtual ~LogAppender() = default;
};

class ConsoleAppender : public LogAppender {
public:
  explicit ConsoleAppender(std::unique_ptr<LogFormatter> formatter = 
                           std::make_unique<SimpleFormatter>());
  void append(const LogEntry& entry) override;
  void flush() override;
  
private:
  std::unique_ptr<LogFormatter> formatter;
  std::mutex mutex;
};

class FileAppender : public LogAppender {
public:
  explicit FileAppender(const std::string& filename,
                      std::unique_ptr<LogFormatter> formatter = 
                        std::make_unique<DetailedFormatter>(),
                      bool append = true);
  ~FileAppender();
  
  void append(const LogEntry& entry) override;
  void flush() override;
  
private:
  std::ofstream file;
  std::unique_ptr<LogFormatter> formatter;
  std::mutex mutex;
  std::string filename;
};

class AsyncAppender : public LogAppender {
public:
  explicit AsyncAppender(std::unique_ptr<LogAppender> appender,
                        size_t queueSize = 1000);
  ~AsyncAppender();
  
  void append(const LogEntry& entry) override;
  void flush() override;
  
private:
  std::unique_ptr<LogAppender> appender;
  std::queue<LogEntry> queue;
  std::mutex queueMutex;
  std::condition_variable condition;
  std::thread workerThread;
  bool running;
  size_t maxQueueSize;
  
  void workerLoop();
};

class Logger {
public:
  static Logger& getInstance();
  
  void setLevel(LogLevel level);
  LogLevel getLevel() const;
  
  void addAppender(std::unique_ptr<LogAppender> appender);
  void clearAppenders();
  
  void log(LogLevel level, const std::string& message,
           const std::string& file = "", int line = 0,
           const std::string& function = "");
  
  void trace(const std::string& message, const std::string& file = "",
             int line = 0, const std::string& function = "");
  void debug(const std::string& message, const std::string& file = "",
             int line = 0, const std::string& function = "");
  void info(const std::string& message, const std::string& file = "",
            int line = 0, const std::string& function = "");
  void warning(const std::string& message, const std::string& file = "",
               int line = 0, const std::string& function = "");
  void error(const std::string& message, const std::string& file = "",
             int line = 0, const std::string& function = "");
  void fatal(const std::string& message, const std::string& file = "",
             int line = 0, const std::string& function = "");
  
  void flush();
  
private:
  Logger();
  ~Logger();
  
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  
  LogLevel level;
  std::vector<std::unique_ptr<LogAppender>> appenders;
  std::mutex mutex;
  
  std::string levelToString(LogLevel level) const;
  bool shouldLog(LogLevel level) const;
};

#define LOG_TRACE(msg) \
  xwift::logging::Logger::getInstance().trace(msg, __FILE__, __LINE__, __FUNCTION__)

#define LOG_DEBUG(msg) \
  xwift::logging::Logger::getInstance().debug(msg, __FILE__, __LINE__, __FUNCTION__)

#define LOG_INFO(msg) \
  xwift::logging::Logger::getInstance().info(msg, __FILE__, __LINE__, __FUNCTION__)

#define LOG_WARNING(msg) \
  xwift::logging::Logger::getInstance().warning(msg, __FILE__, __LINE__, __FUNCTION__)

#define LOG_ERROR(msg) \
  xwift::logging::Logger::getInstance().error(msg, __FILE__, __LINE__, __FUNCTION__)

#define LOG_FATAL(msg) \
  xwift::logging::Logger::getInstance().fatal(msg, __FILE__, __LINE__, __FUNCTION__)

}
}

#endif
