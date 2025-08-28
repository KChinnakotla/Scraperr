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
    FetchResult post_form(const std::string& url, const std::string& form_kv_pairs);

    // Optional global politeness delay (milliseconds) between requests (applied by caller).
    private:
        // all this does is ensure curl_global_init is called once and also calls curl_global_cleanup on dtor
        struct CurlGlobal;
        CurlGlobal* global_;
};