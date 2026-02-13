#include "xwift/Filesystem/Filesystem.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <regex>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace xwift {
namespace fs {

bool FileSystem::exists(const std::string& path) {
  try {
    return std::filesystem::exists(path);
  } catch (...) {
    return false;
  }
}

bool FileSystem::isFile(const std::string& path) {
  try {
    return std::filesystem::is_regular_file(path);
  } catch (...) {
    return false;
  }
}

bool FileSystem::isDirectory(const std::string& path) {
  try {
    return std::filesystem::is_directory(path);
  } catch (...) {
    return false;
  }
}

FileResult FileSystem::readFile(const std::string& path, std::string& content, 
                                const std::string& encoding) {
  if (!isValidPath(path)) {
    return FileResult::fail(FileError::InvalidPath, "Invalid path: " + path);
  }
  
  if (!exists(path)) {
    return FileResult::fail(FileError::NotFound, "File not found: " + path);
  }
  
  if (!isFile(path)) {
    return FileResult::fail(FileError::IsDirectory, "Path is a directory: " + path);
  }
  
  try {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
      return FileResult::fail(FileError::PermissionDenied, "Cannot open file: " + path);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    
    if (encoding != "utf-8") {
      content = convertEncoding(content, "utf-8", encoding);
    }
    
    return FileResult::ok();
  } catch (const std::exception& e) {
    return FileResult::fail(FileError::IOError, std::string("Error reading file: ") + e.what());
  }
}

FileResult FileSystem::writeFile(const std::string& path, const std::string& content,
                               const std::string& encoding) {
  if (!isValidPath(path)) {
    return FileResult::fail(FileError::InvalidPath, "Invalid path: " + path);
  }
  
  std::string contentToWrite = content;
  if (encoding != "utf-8") {
    contentToWrite = convertEncoding(content, encoding, "utf-8");
  }
  
  try {
    std::filesystem::path dirPath = std::filesystem::path(path).parent_path();
    if (!dirPath.empty() && !exists(dirPath.string())) {
      createDirectory(dirPath.string(), true);
    }
    
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
      return FileResult::fail(FileError::PermissionDenied, "Cannot create file: " + path);
    }
    
    file << contentToWrite;
    file.close();
    
    return FileResult::ok();
  } catch (const std::exception& e) {
    return FileResult::fail(FileError::IOError, std::string("Error writing file: ") + e.what());
  }
}

FileResult FileSystem::appendFile(const std::string& path, const std::string& content,
                                  const std::string& encoding) {
  if (!isValidPath(path)) {
    return FileResult::fail(FileError::InvalidPath, "Invalid path: " + path);
  }
  
  std::string contentToWrite = content;
  if (encoding != "utf-8") {
    contentToWrite = convertEncoding(content, encoding, "utf-8");
  }
  
  try {
    std::ofstream file(path, std::ios::binary | std::ios::app);
    if (!file.is_open()) {
      return FileResult::fail(FileError::PermissionDenied, "Cannot open file: " + path);
    }
    
    file << contentToWrite;
    file.close();
    
    return FileResult::ok();
  } catch (const std::exception& e) {
    return FileResult::fail(FileError::IOError, std::string("Error appending to file: ") + e.what());
  }
}

FileResult FileSystem::readFileChunked(const std::string& path,
                                      std::function<bool(const std::string& chunk)> callback,
                                      size_t chunkSize,
                                      const std::string& encoding) {
  if (!isValidPath(path)) {
    return FileResult::fail(FileError::InvalidPath, "Invalid path: " + path);
  }
  
  if (!exists(path)) {
    return FileResult::fail(FileError::NotFound, "File not found: " + path);
  }
  
  if (!isFile(path)) {
    return FileResult::fail(FileError::IsDirectory, "Path is a directory: " + path);
  }
  
  try {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
      return FileResult::fail(FileError::PermissionDenied, "Cannot open file: " + path);
    }
    
    std::vector<char> buffer(chunkSize);
    while (file.read(buffer.data(), chunkSize) || file.gcount() > 0) {
      std::string chunk(buffer.data(), file.gcount());
      if (encoding != "utf-8") {
        chunk = convertEncoding(chunk, "utf-8", encoding);
      }
      if (!callback(chunk)) {
        break;
      }
    }
    
    file.close();
    return FileResult::ok();
  } catch (const std::exception& e) {
    return FileResult::fail(FileError::IOError, std::string("Error reading file: ") + e.what());
  }
}

FileResult FileSystem::writeFileChunked(const std::string& path,
                                       std::function<std::string()> chunkProvider,
                                       const std::string& encoding) {
  if (!isValidPath(path)) {
    return FileResult::fail(FileError::InvalidPath, "Invalid path: " + path);
  }
  
  try {
    std::filesystem::path dirPath = std::filesystem::path(path).parent_path();
    if (!dirPath.empty() && !exists(dirPath.string())) {
      createDirectory(dirPath.string(), true);
    }
    
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
      return FileResult::fail(FileError::PermissionDenied, "Cannot create file: " + path);
    }
    
    while (true) {
      std::string chunk = chunkProvider();
      if (chunk.empty()) {
        break;
      }
      
      if (encoding != "utf-8") {
        chunk = convertEncoding(chunk, encoding, "utf-8");
      }
      
      file << chunk;
    }
    
    file.close();
    return FileResult::ok();
  } catch (const std::exception& e) {
    return FileResult::fail(FileError::IOError, std::string("Error writing file: ") + e.what());
  }
}

FileResult FileSystem::createDirectory(const std::string& path, bool recursive) {
  if (!isValidPath(path)) {
    return FileResult::fail(FileError::InvalidPath, "Invalid path: " + path);
  }
  
  try {
    if (recursive) {
      std::filesystem::create_directories(path);
    } else {
      std::filesystem::create_directory(path);
    }
    return FileResult::ok();
  } catch (const std::filesystem::filesystem_error& e) {
    return FileResult::fail(FileError::IOError, std::string("Error creating directory: ") + e.what());
  }
}

FileResult FileSystem::deleteFile(const std::string& path) {
  if (!isValidPath(path)) {
    return FileResult::fail(FileError::InvalidPath, "Invalid path: " + path);
  }
  
  if (!exists(path)) {
    return FileResult::fail(FileError::NotFound, "File not found: " + path);
  }
  
  if (!isFile(path)) {
    return FileResult::fail(FileError::IsDirectory, "Path is a directory: " + path);
  }
  
  try {
    std::filesystem::remove(path);
    return FileResult::ok();
  } catch (const std::filesystem::filesystem_error& e) {
    return FileResult::fail(FileError::PermissionDenied, std::string("Error deleting file: ") + e.what());
  }
}

FileResult FileSystem::deleteDirectory(const std::string& path, bool recursive) {
  if (!isValidPath(path)) {
    return FileResult::fail(FileError::InvalidPath, "Invalid path: " + path);
  }
  
  if (!exists(path)) {
    return FileResult::fail(FileError::NotFound, "Directory not found: " + path);
  }
  
  if (!isDirectory(path)) {
    return FileResult::fail(FileError::NotDirectory, "Path is not a directory: " + path);
  }
  
  try {
    if (recursive) {
      std::filesystem::remove_all(path);
    } else {
      std::filesystem::remove(path);
    }
    return FileResult::ok();
  } catch (const std::filesystem::filesystem_error& e) {
    return FileResult::fail(FileError::PermissionDenied, std::string("Error deleting directory: ") + e.what());
  }
}

std::vector<std::string> FileSystem::listFiles(const std::string& path) {
  std::vector<std::string> files;
  
  if (!isValidPath(path) || !isDirectory(path)) {
    return files;
  }
  
  try {
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
      if (std::filesystem::is_regular_file(entry)) {
        files.push_back(entry.path().filename().string());
      }
    }
  } catch (...) {
  }
  
  return files;
}

std::vector<std::string> FileSystem::listDirectories(const std::string& path) {
  std::vector<std::string> directories;
  
  if (!isValidPath(path) || !isDirectory(path)) {
    return directories;
  }
  
  try {
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
      if (std::filesystem::is_directory(entry)) {
        directories.push_back(entry.path().filename().string());
      }
    }
  } catch (...) {
  }
  
  return directories;
}

FileResult FileSystem::copyFile(const std::string& source, const std::string& destination) {
  if (!isValidPath(source) || !isValidPath(destination)) {
    return FileResult::fail(FileError::InvalidPath, "Invalid path");
  }
  
  if (!exists(source)) {
    return FileResult::fail(FileError::NotFound, "Source file not found: " + source);
  }
  
  try {
    std::filesystem::copy_file(source, destination, 
                              std::filesystem::copy_options::overwrite_existing);
    return FileResult::ok();
  } catch (const std::filesystem::filesystem_error& e) {
    return FileResult::fail(FileError::IOError, std::string("Error copying file: ") + e.what());
  }
}

FileResult FileSystem::moveFile(const std::string& source, const std::string& destination) {
  if (!isValidPath(source) || !isValidPath(destination)) {
    return FileResult::fail(FileError::InvalidPath, "Invalid path");
  }
  
  if (!exists(source)) {
    return FileResult::fail(FileError::NotFound, "Source file not found: " + source);
  }
  
  try {
    std::filesystem::rename(source, destination);
    return FileResult::ok();
  } catch (const std::filesystem::filesystem_error& e) {
    return FileResult::fail(FileError::IOError, std::string("Error moving file: ") + e.what());
  }
}

std::string FileSystem::normalizePath(const std::string& path) {
  try {
    std::filesystem::path p(path);
    return p.lexically_normal().string();
  } catch (...) {
    return path;
  }
}

std::string FileSystem::getAbsolutePath(const std::string& path) {
  try {
    std::filesystem::path p(path);
    return std::filesystem::absolute(p).string();
  } catch (...) {
    return path;
  }
}

std::string FileSystem::getDirectoryName(const std::string& path) {
  try {
    std::filesystem::path p(path);
    return p.parent_path().string();
  } catch (...) {
    return "";
  }
}

std::string FileSystem::getFileName(const std::string& path) {
  try {
    std::filesystem::path p(path);
    return p.filename().string();
  } catch (...) {
    return "";
  }
}

std::string FileSystem::getFileExtension(const std::string& path) {
  try {
    std::filesystem::path p(path);
    return p.extension().string();
  } catch (...) {
    return "";
  }
}

int64_t FileSystem::getFileSize(const std::string& path) {
  if (!exists(path) || !isFile(path)) {
    return -1;
  }
  
  try {
    return std::filesystem::file_size(path);
  } catch (...) {
    return -1;
  }
}

uint64_t FileSystem::getLastModifiedTime(const std::string& path) {
  if (!exists(path)) {
    return 0;
  }
  
  try {
    auto ftime = std::filesystem::last_write_time(path);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
      ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    return std::chrono::system_clock::to_time_t(sctp);
  } catch (...) {
    return 0;
  }
}

std::string FileSystem::convertEncoding(const std::string& input, 
                                       const std::string& fromEncoding,
                                       const std::string& toEncoding) {
  if (fromEncoding == toEncoding) {
    return input;
  }
  
  return input;
}

bool FileSystem::isValidPath(const std::string& path) {
  if (path.empty()) {
    return false;
  }
  
  std::regex invalidChars(R"([<>:"|?*])");
  if (std::regex_search(path, invalidChars)) {
    return false;
  }
  
  return true;
}

}
}
