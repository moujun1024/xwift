#ifndef XWIFT_HTTP_HTTP_H
#define XWIFT_HTTP_HTTP_H

#include "xwift/stdlib/HTTP/HTTPClient.h"
#include "xwift/stdlib/HTTP/URLParser.h"
#include "xwift/stdlib/HTTP/BodyEncoder.h"
#include "xwift/Basic/Result.h"
#include <memory>

#ifdef _WIN32
#define XWIFT_PLATFORM_SUFFIX ".dll"
#else
#define XWIFT_PLATFORM_SUFFIX ".so"
#endif

namespace xwift {
namespace http {

using HTTPClient::HTTPClient;

std::string urlEncode(const std::string& str);
std::string urlDecode(const std::string& str);

}
}

#endif
