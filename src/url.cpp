#include "url.h"

bool isValidUrl(const std::string& url) {
    if (url.empty()) {
        return false;
    }

    if (url.find("http://") != 0 && url.find("https://") != 0) {
        return false;
    }

    // NOTE: Crude check for presence of a domain.
    const size_t protocol_end = url.find("://");
    if (protocol_end == std::string::npos) {
        return false;
    }
    if (const size_t domain_start = protocol_end + 3;
        url.find('.', domain_start) == std::string::npos) {
        return false;
    }

    return true;
}
