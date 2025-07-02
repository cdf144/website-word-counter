#ifndef FETCH_RESULT_H
#define FETCH_RESULT_H

#include <memory>
#include <string>

struct FetchResult {
    std::string url;
    std::shared_ptr<std::string> content;
    int word_count;
    bool success;
    std::string error_message;

    FetchResult(const std::string& url) : url(url), word_count(0), success(false) {}
};

#endif
