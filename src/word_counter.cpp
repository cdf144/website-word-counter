#include "word_counter.h"

#include <libxml/HTMLparser.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>
#include <unordered_set>

static const std::unordered_set<std::string> SKIP_ELEMENTS = {"script", "style", "noscript", "head",
                                                              "meta",   "link",  "title"};

std::string WordCounter::extractTextFromHtml(const std::string& html) {
    if (html.empty()) {
        return "";
    }

    const htmlDocPtr doc = htmlReadMemory(html.c_str(), static_cast<int>(html.length()), nullptr, nullptr,
                                          HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (!doc) {
        return "";
    }

    std::string result;
    if (const xmlNode* root = xmlDocGetRootElement(doc)) {
        extractTextRecursive(root, result);
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();
    return result;
}

void WordCounter::extractTextRecursive(const xmlNode* node, std::string& result) {
    if (!node) {
        return;
    }

    if (node->type == XML_ELEMENT_NODE) {
        if (const auto name = reinterpret_cast<const char*>(node->name);
            SKIP_ELEMENTS.find(name) != SKIP_ELEMENTS.end()) {
            return;
        }
    }

    if (node->type == XML_TEXT_NODE) {
        if (const auto content = reinterpret_cast<const char*>(node->content)) {
            result += content;
            result += " ";
        }
    }

    for (const xmlNode* child = node->children; child; child = child->next) {
        extractTextRecursive(child, result);
    }
}

int WordCounter::countWords(const std::string& text) {
    if (text.empty()) {
        return 0;
    }

    std::istringstream stream(text);
    std::string word;
    int count = 0;
    while (stream >> word) {
        std::string cleaned_word = sanitizeWord(word);
        if (!cleaned_word.empty()) {
            count++;
        }
    }

    return count;
}

std::string WordCounter::sanitizeWord(const std::string& word) {
    if (word.empty()) {
        return "";
    }

    auto start = std::find_if(word.begin(), word.end(), [](char c) { return std::isalnum(c); });
    if (start == word.end()) {
        return "";
    }

    auto end = std::find_if(word.rbegin(), word.rend(), [](char c) { return std::isalnum(c); }).base();
    return std::string(start, end);
}
