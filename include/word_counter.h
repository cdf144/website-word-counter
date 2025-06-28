#ifndef WORD_COUNTER_H
#define WORD_COUNTER_H

#include <libxml/tree.h>

#include <string>

class WordCounter {
  public:
    static std::string extractTextFromHtml(const std::string&);
    static int countWords(const std::string& text);

  private:
    static void extractTextRecursive(const xmlNode* node, std::string& result);
    static std::string sanitizeWord(const std::string& word);
};

#endif
