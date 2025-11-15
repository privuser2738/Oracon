#include "http_client.h"
#include "oracon/core/logger.h"
#include <curl/curl.h>
#include <sstream>
#include <cstring>
#include <functional>

namespace oracon {
namespace auto_ns {

// Callback for receiving response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    String* response = static_cast<String*>(userp);
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Callback for streaming response data
static size_t StreamCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    auto* callback = static_cast<std::function<void(const String&)>*>(userp);

    if (callback && totalSize > 0) {
        String chunk(static_cast<char*>(contents), totalSize);
        (*callback)(chunk);
    }

    return totalSize;
}

// Callback for reading headers
static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    size_t totalSize = size * nitems;
    auto* headers = static_cast<std::map<String, String>*>(userdata);

    String header(buffer, totalSize);
    size_t colonPos = header.find(':');

    if (colonPos != String::npos) {
        String key = header.substr(0, colonPos);
        String value = header.substr(colonPos + 1);

        // Trim whitespace
        size_t start = value.find_first_not_of(" \t\r\n");
        size_t end = value.find_last_not_of(" \t\r\n");

        if (start != String::npos && end != String::npos) {
            value = value.substr(start, end - start + 1);
        }

        (*headers)[key] = value;
    }

    return totalSize;
}

// Curl-based HTTP client implementation
class CurlHttpClient : public HttpClient {
public:
    CurlHttpClient() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~CurlHttpClient() {
        curl_global_cleanup();
    }

    HttpResponse post(
        const String& url,
        const String& jsonBody,
        const std::map<String, String>& headers
    ) override {
        HttpResponse response;
        CURL* curl = curl_easy_init();

        if (!curl) {
            response.error = "Failed to initialize CURL";
            return response;
        }

        String responseBody;
        struct curl_slist* headersList = nullptr;

        // Set headers
        for (const auto& [key, value] : headers) {
            String header = key + ": " + value;
            headersList = curl_slist_append(headersList, header.c_str());
        }

        // Configure CURL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonBody.length());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headersList);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Perform request
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            response.error = curl_easy_strerror(res);
            response.success = false;
        } else {
            long httpCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            response.statusCode = static_cast<int>(httpCode);
            response.body = responseBody;
            response.success = true;
        }

        // Cleanup
        curl_slist_free_all(headersList);
        curl_easy_cleanup(curl);

        return response;
    }

    HttpResponse postStreaming(
        const String& url,
        const String& jsonBody,
        const std::map<String, String>& headers,
        std::function<void(const String&)> chunkCallback
    ) override {
        HttpResponse response;
        CURL* curl = curl_easy_init();

        if (!curl) {
            response.error = "Failed to initialize CURL";
            return response;
        }

        struct curl_slist* headersList = nullptr;

        // Set headers
        for (const auto& [key, value] : headers) {
            String header = key + ": " + value;
            headersList = curl_slist_append(headersList, header.c_str());
        }

        // Configure CURL for streaming
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonBody.length());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headersList);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StreamCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunkCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);  // Longer timeout for streaming
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Perform request
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            response.error = curl_easy_strerror(res);
            response.success = false;
        } else {
            long httpCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            response.statusCode = static_cast<int>(httpCode);
            response.success = true;
        }

        // Cleanup
        curl_slist_free_all(headersList);
        curl_easy_cleanup(curl);

        return response;
    }
};

// Factory function
std::unique_ptr<HttpClient> createHttpClient() {
    return std::make_unique<CurlHttpClient>();
}

} // namespace auto_ns
} // namespace oracon
