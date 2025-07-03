#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "fetch_result.h"
#include "http_client.h"
#include "url.h"
#include "word_counter.h"

std::mutex cout_mut;
std::mutex cerr_mut;

void print_stdout(const std::string& message) {
    std::lock_guard<std::mutex> lock(cout_mut);
    std::cout << message;
}

void print_stderr(const std::string& message) {
    std::lock_guard<std::mutex> lock(cerr_mut);
    std::cerr << message;
}

std::unique_ptr<FetchResult> processUrl(const std::string& url) {
    auto result = std::make_unique<FetchResult>(url);

    print_stdout("Fetching URL: " + url + "\n");
    try {
        thread_local HTTPClient client;

        auto content = std::make_shared<std::string>(client.fetchUrl(url));
        result->content = content;
        result->word_count = WordCounter::countWords(WordCounter::extractTextFromHtml(*content));
        result->success = true;

        print_stdout("Success! URL: " + url + " Content length: " + std::to_string(content->length()) +
                     " bytes, Word count: " + std::to_string(result->word_count) + " words\n");
    } catch (const std::exception& e) {
        result->error_message = e.what();
        print_stderr("Error fetching " + url + ": " + e.what() + "\n");
    }

    return result;
}

int main(const int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <url1> <url2> ..." << std::endl;
        return 1;
    }

    std::vector<std::string> urls;
    urls.reserve(argc - 1);
    for (int i = 1; i < argc; i++) {
        if (!isValidUrl(argv[i])) {
            std::cerr << "Warning: Invalid URL '" << argv[i] << "' ignored." << std::endl;
            continue;
        }
        urls.emplace_back(argv[i]);
    }

    std::vector<std::unique_ptr<FetchResult>> results;
    results.reserve(urls.size());
    std::vector<std::future<std::unique_ptr<FetchResult>>> futures;
    futures.reserve(urls.size());
    try {
        for (const auto& url : urls) {
            futures.push_back(std::async(std::launch::async, [url]() { return processUrl(url); }));
        }

        for (auto& future : futures) {
            results.push_back(future.get());
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize HTTP client: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\nSummary:\n";
    for (const auto& result : results) {
        std::cout << "URL: " << result->url << "\n";
        if (result->success) {
            std::cout << "  Word count: " << result->word_count << "\n";
        } else {
            std::cout << "  Error: " << result->error_message << "\n";
        }
        std::cout << std::endl;
    }

    return 0;
}
