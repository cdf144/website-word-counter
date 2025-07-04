#include "http_client.h"

#include <curl/easy.h>
#include <curl/multi.h>

#include <stdexcept>

HTTPClient::HTTPClient() {
    curl_global_init(CURL_GLOBAL_ALL);
    multi_handle = curl_multi_init();
    if (!multi_handle) {
        throw std::runtime_error("Failed to initialize CURL multi handle");
    }
}

HTTPClient::~HTTPClient() {
    for (auto const& [handle, context] : contexts) {
        curl_multi_remove_handle(multi_handle, handle);
        curl_easy_cleanup(handle);
    }
    curl_multi_cleanup(multi_handle);
    curl_global_cleanup();
}

size_t HTTPClient::writeCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t total_size = size * nmemb;
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}

void HTTPClient::addRequest(const std::string& url, RequestCallback callback) {
    CURL* easy_handle = curl_easy_init();
    if (!easy_handle) {
        return;
    }

    auto& context = contexts[easy_handle];
    context.url = url;
    context.callback = callback;

    curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &context.response_data);
    curl_easy_setopt(easy_handle, CURLOPT_USERAGENT, "website-word-counter/1.0");
    curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(easy_handle, CURLOPT_PRIVATE, easy_handle);

    curl_multi_add_handle(multi_handle, easy_handle);
}

void HTTPClient::run() {
    int still_running = 0;

    do {
        CURLMcode mc = curl_multi_perform(multi_handle, &still_running);
        if (mc != CURLM_OK) {
            break;
        }

        CURLMsg* msg;
        int msgs_left;
        while ((msg = curl_multi_info_read(multi_handle, &msgs_left))) {
            if (msg->msg == CURLMSG_DONE) {
                processCompletedRequest(msg);
            }
        }

        if (still_running > 0) {
            int numfds;
            mc = curl_multi_wait(multi_handle, nullptr, 0, 100, &numfds);
            if (mc != CURLM_OK) {
                break;
            }
        }
    } while (still_running > 0);
}

void HTTPClient::processCompletedRequest(CURLMsg* msg) {
    CURL* easy_handle = msg->easy_handle;
    auto it = contexts.find(easy_handle);
    if (it == contexts.end()) {
        return;
    }

    RequestContext& context = it->second;
    std::string response_data;

    if (msg->data.result == CURLE_OK) {
        long response_code;
        curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code >= 200 && response_code < 400) {
            response_data = std::move(context.response_data);
        }
    }

    context.callback(context.url, response_data);

    curl_multi_remove_handle(multi_handle, easy_handle);
    curl_easy_cleanup(easy_handle);
    contexts.erase(it);
}
