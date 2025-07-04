#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "fetch_result.h"
#include "http_client.h"
#include "thread_pool.h"
#include "url.h"
#include "word_counter.h"

std::mutex cout_mut;

void thread_safe_cout(const std::string& message) {
    std::lock_guard<std::mutex> lock(cout_mut);
    std::cout << message;
}

void process_content(const std::string& url, const std::string& content,
                     std::vector<std::unique_ptr<FetchResult>>& results, std::mutex& results_mutex) {
    auto result = std::make_unique<FetchResult>(url);
    if (content.empty()) {
        result->success = false;
        result->error_message = "Failed to fetch content or HTTP error.";
        thread_safe_cout("Error fetching " + url + ": " + result->error_message + "\n");
    } else {
        result->content = std::make_shared<std::string>(content);
        result->word_count = WordCounter::countWords(WordCounter::extractTextFromHtml(*result->content));
        result->success = true;
        thread_safe_cout("Success! URL: " + url + " Content length: " + std::to_string(content.length()) +
                         " bytes, Word count: " + std::to_string(result->word_count) + " words\n");
    }

    std::lock_guard<std::mutex> lock(results_mutex);
    results.push_back(std::move(result));
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

    const unsigned int num_threads = std::thread::hardware_concurrency();
    ThreadPool pool(num_threads);
    HTTPClient client;

    std::vector<std::unique_ptr<FetchResult>> results;
    std::mutex results_mutex;
    std::atomic<size_t> tasks_done = 0;

    for (const auto& url : urls) {
        client.addRequest(url, [&](const std::string& req_url, const std::string& content) {
            pool.enqueue(process_content, req_url, content, std::ref(results), std::ref(results_mutex));
            tasks_done++;
        });
    }

    thread_safe_cout("Fetching " + std::to_string(urls.size()) + " URLs using up to " + std::to_string(num_threads) +
                     " threads...\n");

    client.run();

    while (tasks_done < urls.size()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
