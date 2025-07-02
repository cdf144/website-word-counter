#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "fetch_result.h"
#include "http_client.h"
#include "url.h"
#include "word_counter.h"

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
    try {
        HTTPClient client;

        for (const auto& url : urls) {
            auto result = std::make_unique<FetchResult>(url);

            std::cout << "Fetching URL: " << url << std::endl;
            try {
                auto content = std::make_shared<std::string>(client.fetchUrl(url));
                result->content = content;
                result->word_count = WordCounter::countWords(WordCounter::extractTextFromHtml(*content));
                result->success = true;

                std::cout << "Success! Content length: " << content->length() << " bytes" << std::endl;
                std::cout << "Word count: " << result->word_count << " words" << std::endl;
            } catch (const std::exception& e) {
                result->error_message = e.what();
                std::cerr << "Error fetching " << url << ": " << e.what() << std::endl;
            }

            results.push_back(std::move(result));
            std::cout << "---" << std::endl;
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
