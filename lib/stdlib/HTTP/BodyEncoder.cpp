#include "xwift/stdlib/HTTP/BodyEncoder.h"
#include <sstream>
#include <random>
#include <iomanip>

namespace xwift {
namespace http {

std::string BodyEncoder::encodeJSON(const std::string& json) {
    return json;
}

std::string BodyEncoder::encodeFormURLEncoded(const std::map<std::string, std::string>& params) {
    std::ostringstream oss;
    bool first = true;
    
    for (const auto& pair : params) {
        if (!first) {
            oss << "&";
        }
        first = false;
        oss << urlEncode(pair.first) << "=" << urlEncode(pair.second);
    }
    
    return oss.str();
}

std::string BodyEncoder::encodeMultipartFormData(const std::map<std::string, std::string>& fields, const std::string& boundary) {
    std::ostringstream oss;
    
    for (const auto& field : fields) {
        oss << "--" << boundary << "\r\n";
        oss << "Content-Disposition: form-data; name=\"" << field.first << "\"\r\n\r\n";
        oss << field.second << "\r\n";
    }
    
    oss << "--" << boundary << "--\r\n";
    
    return oss.str();
}

std::string BodyEncoder::getContentTypeString(ContentType type) {
    switch (type) {
        case ContentType::JSON:
            return "application/json";
        case ContentType::FormURLEncoded:
            return "application/x-www-form-urlencoded";
        case ContentType::MultipartFormData:
            return "multipart/form-data";
        case ContentType::TextPlain:
            return "text/plain";
        default:
            return "application/octet-stream";
    }
}

std::string BodyEncoder::generateBoundary() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::ostringstream oss;
    oss << "----WebKitFormBoundary";
    
    for (int i = 0; i < 16; i++) {
        oss << std::hex << std::setw(1) << dis(gen);
    }
    
    return oss.str();
}

std::string BodyDecoder::decodeJSON(const std::string& body) {
    return body;
}

std::map<std::string, std::string> BodyDecoder::decodeFormURLEncoded(const std::string& body) {
    std::map<std::string, std::string> result;
    std::istringstream iss(body);
    std::string pair;
    
    while (std::getline(iss, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = urlDecode(pair.substr(0, pos));
            std::string value = urlDecode(pair.substr(pos + 1));
            result[key] = value;
        }
    }
    
    return result;
}

}
}
