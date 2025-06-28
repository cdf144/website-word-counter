#include <iostream>
#include <string>
#include <vector>

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
            std::cerr << "Warning: Invalid URL '" << argv[i] << "' ignored."
                      << std::endl;
            continue;
        }
        urls.emplace_back(argv[i]);
    }

    try {
        HTTPClient client;

        for (const auto& url : urls) {
            std::cout << "Fetching URL: " << url << std::endl;
            try {
                std::string content = client.fetchUrl(url);
                const int word_count = WordCounter::countWords(
                    WordCounter::extractTextFromHtml(content));
                std::cout << "Success! Content length: " << content.length()
                          << " bytes" << std::endl;
                std::cout << "Word count: " << word_count << " words"
                          << std::endl;
                std::cout << "---" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error fetching " << url << ": " << e.what()
                          << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize HTTP client: " << e.what()
                  << std::endl;
        return 1;
    }

    return 0;
}
