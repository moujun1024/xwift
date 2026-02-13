#ifndef XWIFT_FILESYSTEM_FILESYSTEM_H
#define XWIFT_FILESYSTEM_FILESYSTEM_H

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace xwift {
namespace fs {

enum class FileError {
  None,
  NotFound,
  PermissionDenied,
  InvalidPath,
  IsDirectory,
  NotDirectory,
  AlreadyExists,
  IOError,
  EncodingError
};

struct FileResult {
  bool success;
  FileError error;
  std::string errorMessage;
  
  FileResult() : success(true), error(FileError::None) {}
  FileResult(bool s, FileError e, const std::string& msg) 
    : success(s), error(e), errorMessage(msg) {}
  
  static FileResult ok() { return FileResult(); }
  static FileResult fail(FileError e, const std::string& msg) {
    return FileResult(false, e, msg);
  }
};

class FileSystem {
public:
  static bool exists(const std::string& path);
  static bool isFile(const std::string& path);
  static bool isDirectory(const std::string& path);
  
  static FileResult readFile(const std::string& path, std::string& content, 
                          const std::string& encoding = "utf-8");
  static FileResult writeFile(const std::string& path, const std::string& content,
                           const std::string& encoding = "utf-8");
  static FileResult appendFile(const std::string& path, const std::string& content,
                              const std::string& encoding = "utf-8");
  
  static FileResult readFileChunked(const std::string& path,
                                   std::function<bool(const std::string& chunk)> callback,
                                   size_t chunkSize = 8192,
                                   const std::string& encoding = "utf-8");
  
  static FileResult writeFileChunked(const std::string& path,
                                    std::function<std::string()> chunkProvider,
                                    const std::string& encoding = "utf-8");
  
  static FileResult createDirectory(const std::string& path, bool recursive = true);
  static FileResult deleteFile(const std::string& path);
  static FileResult deleteDirectory(const std::string& path, bool recursive = false);
  
  static std::vector<std::string> listFiles(const std::string& path);
  static std::vector<std::string> listDirectories(const std::string& path);
  
  static FileResult copyFile(const std::string& source, const std::string& destination);
  static FileResult moveFile(const std::string& source, const std::string& destination);
  
  static std::string normalizePath(const std::string& path);
  static std::string getAbsolutePath(const std::string& path);
  static std::string getDirectoryName(const std::string& path);
  static std::string getFileName(const std::string& path);
  static std::string getFileExtension(const std::string& path);
  
  static int64_t getFileSize(const std::string& path);
  static uint64_t getLastModifiedTime(const std::string& path);
  
private:
  static std::string convertEncoding(const std::string& input, 
                                    const std::string& fromEncoding,
                                    const std::string& toEncoding);
  static bool isValidPath(const std::string& path);
};

}
}

#endif
