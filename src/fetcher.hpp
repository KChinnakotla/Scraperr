#pragma once
#include <string>
#include <vector>

struct FetchResult {
    long http_status = 0;
    std::string body;
    std::string error;
    long long duration_ms = 0;
};

class Fetcher {
public:
    Fetcher();
    ~Fetcher();
    FetchResult get(const std::string& url, long timeout_ms = 15000, bool follow_redirects = true);

    // Optional global politeness delay (milliseconds) between requests (applied by caller).
private:
    // libcurl is initialized globally in constructor/destructor via RAII
    struct CurlGlobal;
    CurlGlobal* global_;
};
