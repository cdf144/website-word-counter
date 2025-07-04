#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <curl/curl.h>

#include <functional>
#include <map>
#include <string>

class HTTPClient {
  public:
    using RequestCallback = std::function<void(const std::string&, const std::string&)>;

    HTTPClient();
    ~HTTPClient();

    HTTPClient(const HTTPClient&) = delete;
    HTTPClient& operator=(const HTTPClient&) = delete;

    void addRequest(const std::string& url, RequestCallback callback);
    void run();

  private:
    struct RequestContext {
        std::string url;
        std::string response_data;
        RequestCallback callback;
    };

    CURLM* multi_handle;
    std::map<CURL*, RequestContext> contexts;

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* response);
    void processCompletedRequest(CURLMsg* msg);
};

#endif
