#ifndef ORACON_AUTO_HTTP_CLIENT_H
#define ORACON_AUTO_HTTP_CLIENT_H

#include "oracon/core/types.h"
#include <map>
#include <functional>

namespace oracon {
namespace auto_ns {

using core::String;

// HTTP response
struct HttpResponse {
    int statusCode = 0;
    String body;
    std::map<String, String> headers;
    bool success = false;
    String error;

    bool isSuccess() const { return success && statusCode >= 200 && statusCode < 300; }
};

// Simple HTTP client interface
class HttpClient {
public:
    virtual ~HttpClient() = default;

    // POST request with JSON body
    virtual HttpResponse post(
        const String& url,
        const String& jsonBody,
        const std::map<String, String>& headers
    ) = 0;

    // POST request with streaming response callback
    virtual HttpResponse postStreaming(
        const String& url,
        const String& jsonBody,
        const std::map<String, String>& headers,
        std::function<void(const String&)> chunkCallback
    ) = 0;
};

} // namespace auto_ns
} // namespace oracon

#endif // ORACON_AUTO_HTTP_CLIENT_H
