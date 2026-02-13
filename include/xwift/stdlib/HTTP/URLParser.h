#ifndef XWIFT_HTTP_URLPARSER_H
#define XWIFT_HTTP_URLPARSER_H

#include <string>
#include <map>

namespace xwift {
namespace http {

struct URL {
    std::string Protocol;
    std::string Host;
    int Port;
    std::string Path;
    std::string Query;
    std::string Fragment;
    std::map<std::string, std::string> QueryParams;
    
    URL() : Port(0) {}
    
    std::string toString() const;
    bool isValid() const;
};

class URLParser {
public:
    static URL parse(const std::string& url);
    
private:
    static void parseQuery(const std::string& query, std::map<std::string, std::string>& params);
    static std::string decodeURIComponent(const std::string& str);
};

}
}

#endif
