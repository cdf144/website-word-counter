#include "http_client.h"

#include <stdexcept>

HTTPClient::HTTPClient() : curl_handle(nullptr), is_initialized(false) {
    curl_handle = curl_easy_init();
    if (!curl_handle) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    is_initialized = true;
}

HTTPClient::~HTTPClient() {
    if (is_initialized) {
        curl_easy_cleanup(curl_handle);
        curl_handle = nullptr;
    }
}

size_t HTTPClient::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t total_size = size * nmemb;
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}

std::string HTTPClient::fetchUrl(const std::string& url) const {
    if (!is_initialized) {
        throw std::runtime_error("HTTPClient not properly initialized");
    }

    std::string response;

    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "website-word-counter/1.0");
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 30L);

    if (const CURLcode res = curl_easy_perform(curl_handle); res != CURLE_OK) {
        throw std::runtime_error("CURL error: " + std::string(curl_easy_strerror(res)));
    }

    long response_code;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);

    if (response_code >= 400) {
        throw std::runtime_error("HTTP error: " + std::to_string(response_code));
    }

    return response;
}
