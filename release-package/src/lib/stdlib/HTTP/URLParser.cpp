#include "xwift/stdlib/HTTP/URLParser.h"
#include <regex>
#include <sstream>
#include <iomanip>

namespace xwift {
namespace http {

std::string URL::toString() const {
    std::ostringstream oss;
    
    if (!Protocol.empty()) {
        oss << Protocol << "://";
    }
    
    oss << Host;
    
    if (Port > 0) {
        oss << ":" << Port;
    }
    
    oss << Path;
    
    if (!Query.empty()) {
        oss << "?" << Query;
    }
    
    if (!Fragment.empty()) {
        oss << "#" << Fragment;
    }
    
    return oss.str();
}

bool URL::isValid() const {
    return !Host.empty();
}

URL URLParser::parse(const std::string& url) {
    URL result;
    
    std::regex urlRegex(R"(^([^:]+)://([^:/?#]+)(?::(\d+))?([^?#]*)(?:\?([^#]*))?(?:#(.*))?$)");
    std::smatch match;
    
    if (std::regex_match(url, match, urlRegex)) {
        result.Protocol = match[1].str();
        result.Host = match[2].str();
        result.Path = match[4].str();
        result.Query = match[5].str();
        result.Fragment = match[6].str();
        
        if (match[3].matched) {
            result.Port = std::stoi(match[3].str());
        } else {
            if (result.Protocol == "http") {
                result.Port = 80;
            } else if (result.Protocol == "https") {
                result.Port = 443;
            }
        }
        
        if (!result.Query.empty()) {
            parseQuery(result.Query, result.QueryParams);
        }
    }
    
    return result;
}

void URLParser::parseQuery(const std::string& query, std::map<std::string, std::string>& params) {
    std::istringstream iss(query);
    std::string pair;
    
    while (std::getline(iss, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            params[decodeURIComponent(key)] = decodeURIComponent(value);
        } else {
            params[decodeURIComponent(pair)] = "";
        }
    }
}

std::string URLParser::decodeURIComponent(const std::string& str) {
    std::string result;
    char ch;
    int i;
    
    for (i = 0; i < str.length(); i++) {
        if (str[i] != '%') {
            result += str[i];
        } else {
            if (i + 2 < str.length()) {
                std::string hexStr = str.substr(i + 1, 2);
                char ch = static_cast<char>(std::stoi(hexStr, nullptr, 16));
                result += ch;
                i += 2;
            }
        }
    }
    
    return result;
}

}
}
