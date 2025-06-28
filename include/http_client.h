#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <curl/curl.h>

#include <string>

class HTTPClient {
  public:
    HTTPClient();
    ~HTTPClient();

    HTTPClient(const HTTPClient&) = delete;
    HTTPClient& operator=(const HTTPClient&) = delete;

    std::string fetchUrl(const std::string& url) const;

  private:
    CURL* curl_handle;
    bool is_initialized;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                                std::string* response);
};

#endif
