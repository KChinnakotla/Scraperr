#include "fetcher.hpp"
#include "log.hpp"
#include <curl/curl.h>
#include <chrono>

struct Fetcher::CurlGlobal {
    CurlGlobal() { curl_global_init(CURL_GLOBAL_ALL); }
    ~CurlGlobal() { curl_global_cleanup(); }
};

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* s = static_cast<std::string*>(userdata);
    s->append(ptr, size * nmemb);
    return size * nmemb;
}

Fetcher::Fetcher() : global_(new CurlGlobal()) {}
Fetcher::~Fetcher() { delete global_; }

FetchResult Fetcher::get(const std::string& url, long timeout_ms, bool follow_redirects) {
    using namespace std::chrono;
    FetchResult res;
    CURL* curl = curl_easy_init();
    if (!curl) {
        res.error = "curl_easy_init failed";
        return res;
    }

    char errbuf[CURL_ERROR_SIZE]{0};
    std::string body;
    auto t0 = steady_clock::now();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "cpp-scraper/0.1");
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""); // enable gzip/deflate
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout_ms);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, follow_redirects ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode code = curl_easy_perform(curl);
    auto t1 = steady_clock::now();

    res.duration_ms = duration_cast<milliseconds>(t1 - t0).count();

    if (code != CURLE_OK) {
        res.error = errbuf[0] ? std::string(errbuf) : curl_easy_strerror(code);
        curl_easy_cleanup(curl);
        return res;
    }

    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    res.http_status = status;
    res.body = std::move(body);

    curl_easy_cleanup(curl);
    return res;
}
