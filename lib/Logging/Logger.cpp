#include "xwift/Logging/Logger.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <mutex>

namespace xwift {
namespace logging {

std::string SimpleFormatter::format(const LogEntry& entry) {
  std::ostringstream oss;
  
  std::time_t time = std::chrono::system_clock::to_time_t(entry.timestamp);
  std::tm tm = *std::localtime(&time);
  
  oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
  oss << " [" << Logger::getInstance().levelToString(entry.level) << "] ";
  oss << entry.message;
  
  return oss.str();
}

std::string DetailedFormatter::format(const LogEntry& entry) {
  std::ostringstream oss;
  
  std::time_t time = std::chrono::system_clock::to_time_t(entry.timestamp);
  std::tm tm = *std::localtime(&time);
  
  oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
  oss << "." << std::setw(3) << std::setfill('0') 
      << std::chrono::duration_cast<std::chrono::milliseconds>(
           entry.timestamp.time_since_epoch()).count() % 1000;
  oss << " [" << Logger::getInstance().levelToString(entry.level) << "] ";
  
  if (!entry.file.empty()) {
    oss << entry.file;
    if (entry.line > 0) {
      oss << ":" << entry.line;
    }
    oss << " - ";
  }
  
  if (!entry.function.empty()) {
    oss << entry.function << "() - ";
  }
  
  oss << entry.message;
  
  return oss.str();
}

std::string JsonFormatter::format(const LogEntry& entry) {
  std::ostringstream oss;
  
  std::time_t time = std::chrono::system_clock::to_time_t(entry.timestamp);
  
  oss << "{";
  oss << "\"timestamp\":" << time << ",";
  oss << "\"level\":\"" << Logger::getInstance().levelToString(entry.level) << "\",";
  oss << "\"message\":\"" << entry.message << "\"";
  
  if (!entry.file.empty()) {
    oss << ",\"file\":\"" << entry.file << "\"";
  }
  
  if (entry.line > 0) {
    oss << ",\"line\":" << entry.line;
  }
  
  if (!entry.function.empty()) {
    oss << ",\"function\":\"" << entry.function << "\"";
  }
  
  oss << "}";
  
  return oss.str();
}

ConsoleAppender::ConsoleAppender(std::unique_ptr<LogFormatter> fmt)
  : formatter(std::move(fmt)) {}

void ConsoleAppender::append(const LogEntry& entry) {
  std::lock_guard<std::mutex> lock(mutex);
  
  std::string formatted = formatter->format(entry);
  
  switch (entry.level) {
    case LogLevel::Trace:
    case LogLevel::Debug:
      std::cout << formatted << std::endl;
      break;
    case LogLevel::Info:
      std::cout << formatted << std::endl;
      break;
    case LogLevel::Warning:
      std::cerr << formatted << std::endl;
      break;
    case LogLevel::Error:
    case LogLevel::Fatal:
      std::cerr << formatted << std::endl;
      break;
    default:
      break;
  }
}

void ConsoleAppender::flush() {
  std::lock_guard<std::mutex> lock(mutex);
  std::cout.flush();
  std::cerr.flush();
}

FileAppender::FileAppender(const std::string& filename,
                           std::unique_ptr<LogFormatter> fmt,
                           bool append)
  : filename(filename), formatter(std::move(fmt)) {
  auto mode = append ? std::ios::app : std::ios::trunc;
  file.open(filename, std::ios::out | mode);
}

FileAppender::~FileAppender() {
  if (file.is_open()) {
    file.close();
  }
}

void FileAppender::append(const LogEntry& entry) {
  std::lock_guard<std::mutex> lock(mutex);
  
  if (file.is_open()) {
    std::string formatted = formatter->format(entry);
    file << formatted << std::endl;
  }
}

void FileAppender::flush() {
  std::lock_guard<std::mutex> lock(mutex);
  if (file.is_open()) {
    file.flush();
  }
}

AsyncAppender::AsyncAppender(std::unique_ptr<LogAppender> app,
                             size_t queueSize)
  : appender(std::move(app)), maxQueueSize(queueSize), running(true) {
  workerThread = std::thread(&AsyncAppender::workerLoop, this);
}

AsyncAppender::~AsyncAppender() {
  running = false;
  condition.notify_all();
  
  if (workerThread.joinable()) {
    workerThread.join();
  }
  
  while (!queue.empty()) {
    LogEntry entry = queue.front();
    queue.pop();
    appender->append(entry);
  }
  
  appender->flush();
}

void AsyncAppender::append(const LogEntry& entry) {
  std::unique_lock<std::mutex> lock(queueMutex);
  
  if (queue.size() >= maxQueueSize) {
    return;
  }
  
  queue.push(entry);
  condition.notify_one();
}

void AsyncAppender::flush() {
  std::unique_lock<std::mutex> lock(queueMutex);
  
  while (!queue.empty()) {
    LogEntry entry = queue.front();
    queue.pop();
    lock.unlock();
    appender->append(entry);
    lock.lock();
  }
  
  appender->flush();
}

void AsyncAppender::workerLoop() {
  while (running) {
    std::unique_lock<std::mutex> lock(queueMutex);
    
    condition.wait(lock, [this] {
      return !queue.empty() || !running;
    });
    
    while (!queue.empty() && running) {
      LogEntry entry = queue.front();
      queue.pop();
      lock.unlock();
      appender->append(entry);
      lock.lock();
    }
  }
}

Logger& Logger::getInstance() {
  static Logger instance;
  return instance;
}

Logger::Logger() : level(LogLevel::Info) {
  addAppender(std::make_unique<ConsoleAppender>());
}

Logger::~Logger() {
  flush();
}

void Logger::setLevel(LogLevel lvl) {
  std::lock_guard<std::mutex> lock(mutex);
  level = lvl;
}

LogLevel Logger::getLevel() const {
  return level;
}

void Logger::addAppender(std::unique_ptr<LogAppender> app) {
  std::lock_guard<std::mutex> lock(mutex);
  appenders.push_back(std::move(app));
}

void Logger::clearAppenders() {
  std::lock_guard<std::mutex> lock(mutex);
  appenders.clear();
}

void Logger::log(LogLevel lvl, const std::string& message,
                 const std::string& file, int line,
                 const std::string& function) {
  if (!shouldLog(lvl)) {
    return;
  }
  
  LogEntry entry;
  entry.level = lvl;
  entry.message = message;
  entry.file = file;
  entry.line = line;
  entry.function = function;
  entry.timestamp = std::chrono::system_clock::now();
  
  std::lock_guard<std::mutex> lock(mutex);
  for (auto& appender : appenders) {
    appender->append(entry);
  }
}

void Logger::trace(const std::string& message, const std::string& file,
                   int line, const std::string& function) {
  log(LogLevel::Trace, message, file, line, function);
}

void Logger::debug(const std::string& message, const std::string& file,
                   int line, const std::string& function) {
  log(LogLevel::Debug, message, file, line, function);
}

void Logger::info(const std::string& message, const std::string& file,
                  int line, const std::string& function) {
  log(LogLevel::Info, message, file, line, function);
}

void Logger::warning(const std::string& message, const std::string& file,
                     int line, const std::string& function) {
  log(LogLevel::Warning, message, file, line, function);
}

void Logger::error(const std::string& message, const std::string& file,
                   int line, const std::string& function) {
  log(LogLevel::Error, message, file, line, function);
}

void Logger::fatal(const std::string& message, const std::string& file,
                   int line, const std::string& function) {
  log(LogLevel::Fatal, message, file, line, function);
}

void Logger::flush() {
  std::lock_guard<std::mutex> lock(mutex);
  for (auto& appender : appenders) {
    appender->flush();
  }
}

std::string Logger::levelToString(LogLevel lvl) const {
  switch (lvl) {
    case LogLevel::Trace: return "TRACE";
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info: return "INFO";
    case LogLevel::Warning: return "WARNING";
    case LogLevel::Error: return "ERROR";
    case LogLevel::Fatal: return "FATAL";
    case LogLevel::Off: return "OFF";
    default: return "UNKNOWN";
  }
}

bool Logger::shouldLog(LogLevel lvl) const {
  return lvl >= level && level != LogLevel::Off;
}

}
}
