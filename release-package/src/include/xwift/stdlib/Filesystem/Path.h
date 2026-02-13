#ifndef XWIFT_STDLIB_FILESYSTEM_PATH_H
#define XWIFT_STDLIB_FILESYSTEM_PATH_H

#include "xwift/AST/Type.h"
#include "xwift/Basic/LLVM.h"
#include <string>
#include <vector>
#include <memory>

#ifdef _WIN32
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
namespace fs {
    struct path {
        std::string p;
        
        path(const std::string& path) : p(path) {}
        
        std::string string() const { return p; }
        
        path parent_path() const {
            size_t pos = p.find_last_of("/\\");
            if (pos == std::string::npos) {
                return path("");
            }
            return p.substr(0, pos);
        }
        
        path filename() const {
            size_t pos = p.find_last_of("/\\");
            if (pos == std::string::npos) {
                return p;
            }
            return p.substr(pos + 1);
        }
        
        path extension() const {
            size_t pos = p.find_last_of('.');
            if (pos == std::string::npos) {
                return "";
            }
            return p.substr(pos + 1);
        }
        
        bool exists() const {
            struct stat buffer;
            return (stat(p.c_str(), &buffer) == 0);
        }
        
        bool is_directory() const {
            struct stat buffer;
            if (stat(p.c_str(), &buffer) != 0) {
                return false;
            }
            return S_ISDIR(buffer.st_mode);
        }
        
        bool is_file() const {
            struct stat buffer;
            if (stat(p.c_str(), &buffer) != 0) {
                return false;
            }
            return S_ISREG(buffer.st_mode);
        }
        
        uintmax_t file_size() const {
            struct stat buffer;
            if (stat(p.c_str(), &buffer) != 0) {
                return 0;
            }
            return buffer.st_size;
        }
        
        std::vector<path> list_directory() const {
            std::vector<path> result;
            
            DIR* dir = opendir(p.c_str());
            if (dir == nullptr) {
                return result;
            }
            
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string name = entry->d_name;
                if (name != "." && name != "..") {
                    result.push_back(path(p + "/" + name));
                }
            }
            
            closedir(dir);
            return result;
        }
    };
}
#endif

namespace xwift {

// Cross-platform Path class
class Path {
private:
    std::string pathString;
    
public:
    Path(const std::string& path) : pathString(path) {}
    
    std::string toString() const { return pathString; }
    
    Path parent() const {
#ifdef _WIN32
        fs::path p(pathString);
        return Path(p.parent_path().string());
#else
        fs::path p(pathString);
        return Path(p.parent_path().string());
#endif
    }
    
    Path filename() const {
#ifdef _WIN32
        fs::path p(pathString);
        return Path(p.filename().string());
#else
        fs::path p(pathString);
        return Path(p.filename().string());
#endif
    }
    
    std::string extension() const {
#ifdef _WIN32
        fs::path p(pathString);
        return p.extension().string();
#else
        fs::path p(pathString);
        return p.extension().string();
#endif
    }
    
    bool exists() const {
#ifdef _WIN32
        fs::path p(pathString);
        return fs::exists(p);
#else
        struct stat buffer;
        return stat(pathString.c_str(), &buffer) == 0;
#endif
    }
    
    bool isDirectory() const {
#ifdef _WIN32
        fs::path p(pathString);
        return fs::is_directory(p);
#else
        struct stat buffer;
        if (stat(pathString.c_str(), &buffer) != 0) {
            return false;
        }
        return S_ISDIR(buffer.st_mode);
#endif
    }
    
    bool isFile() const {
#ifdef _WIN32
        fs::path p(pathString);
        return fs::is_regular_file(p);
#else
        struct stat buffer;
        if (stat(pathString.c_str(), &buffer) != 0) {
            return false;
        }
        return S_ISREG(buffer.st_mode);
#endif
    }
    
    uintmax_t fileSize() const {
#ifdef _WIN32
        fs::path p(pathString);
        return fs::file_size(p);
#else
        struct stat buffer;
        if (stat(pathString.c_str(), &buffer) != 0) {
            return 0;
        }
        return buffer.st_size;
#endif
    }
    
    std::vector<Path> listDirectory() const {
        std::vector<Path> result;
        
#ifdef _WIN32
        fs::path p(pathString);
        if (fs::is_directory(p)) {
            for (const auto& entry : fs::directory_iterator(p)) {
                result.push_back(Path(entry.path().string()));
            }
        }
#else
        DIR* dir = opendir(pathString.c_str());
        if (dir == nullptr) {
            return result;
        }
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name != "." && name != "..") {
                result.push_back(Path(pathString + "/" + name));
            }
        }
        
        closedir(dir);
#endif
        return result;
    }
    
    Path join(const Path& other) const {
#ifdef _WIN32
        fs::path p1(pathString);
        fs::path p2(other.toString());
        return Path((p1 / p2).string());
#else
        std::string separator = (pathString.back() == '/') ? "" : "/";
        return Path(pathString + separator + other.toString());
#endif
    }
    
    static Path current() {
#ifdef _WIN32
        return Path(fs::current_path().string());
#else
        char* cwd = getcwd(nullptr, 0);
        if (cwd == nullptr) {
            return Path("");
        }
        std::string result(cwd);
        free(cwd);
        return Path(result);
#endif
    }
};

}

#endif