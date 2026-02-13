#ifndef XWIFT_HTTP_BODYENCODER_H
#define XWIFT_HTTP_BODYENCODER_H

#include <string>
#include <map>

namespace xwift {
namespace http {

enum class ContentType {
    JSON,
    FormURLEncoded,
    MultipartFormData,
    TextPlain
};

class BodyEncoder {
public:
    static std::string encodeJSON(const std::string& json);
    static std::string encodeFormURLEncoded(const std::map<std::string, std::string>& params);
    static std::string encodeMultipartFormData(const std::map<std::string, std::string>& fields, const std::string& boundary);
    
    static std::string getContentTypeString(ContentType type);
    static std::string generateBoundary();
};

class BodyDecoder {
public:
    static std::string decodeJSON(const std::string& body);
    static std::map<std::string, std::string> decodeFormURLEncoded(const std::string& body);
};

}
}

#endif
