#pragma once
#include <atomic>
#include <mutex>
#include <string>
#include <vector>

class Scraper {
public:
    struct Options {
        std::string out_dir = "data";
        int concurrency = 16;
        long timeout_ms = 15000;
        int delay_ms = 0; // optional politeness
    };

    explicit Scraper(Options opt);
    void scrape_urls(const std::vector<std::string>& urls);

private:
    struct Row {
        std::string url;
        long status = 0;
        std::string filename;
        size_t bytes = 0;
        long long ms = 0;
        std::string error;
    };

    void save_manifest();
    void save_body(const std::string& filename, const std::string& body);

    Options opt_;
    std::vector<Row> results_;
    std::mutex results_mu_;
};
