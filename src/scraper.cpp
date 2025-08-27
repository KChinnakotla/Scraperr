#include "scraper.hpp"
#include "fetcher.hpp"
#include "log.hpp"
#include "thread_pool.hpp"
#include "util.hpp"
#include <chrono>
#include <fstream>
#include <thread>

Scraper::Scraper(Options opt) : opt_(std::move(opt)) {
    util::ensure_dir(opt_.out_dir);
    util::ensure_dir(util::join_path(opt_.out_dir, "pages"));
}

void Scraper::save_body(const std::string& filename, const std::string& body) {
    std::ofstream out(filename, std::ios::binary);
    out.write(body.data(), static_cast<std::streamsize>(body.size()));
}

void Scraper::save_manifest() {
    const std::string manifest = util::join_path(opt_.out_dir, "manifest.csv");
    std::ofstream out(manifest);
    out << "url,status,filename,bytes,duration_ms,error\n";
    for (const auto& r : results_) {
        out << '"' << r.url << '"' << ','
            << r.status << ','
            << '"' << r.filename << '"' << ','
            << r.bytes << ','
            << r.ms << ','
            << '"' << r.error << '"' << '\n';
    }
    logx::info("Wrote manifest: ", manifest, " (", results_.size(), " rows)");
}

void Scraper::scrape_urls(const std::vector<std::string>& urls) {
    {
        ThreadPool pool(static_cast<size_t>(opt_.concurrency));
        Fetcher fetcher;

        std::atomic<size_t> done{0};
        const size_t total = urls.size();

        for (const auto& url : urls) {
            pool.enqueue([&, url] {
                if (opt_.delay_ms > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(opt_.delay_ms));

                auto r = fetcher.get(url, opt_.timeout_ms, true);

                Row row;
                row.url = url;
                row.status = r.http_status;
                row.ms = r.duration_ms;

                std::string base = util::basename_from_url(url);
                std::string outpath = util::join_path(opt_.out_dir, util::join_path("pages", base));

                if (!r.error.empty()) {
                    row.error = r.error;
                    row.filename = "";
                    row.bytes = 0;
                    logx::warn("FAIL ", url, " err=", r.error);
                } else {
                    row.filename = util::join_path("pages", base);
                    row.bytes = r.body.size();
                    save_body(outpath, r.body);
                    logx::info("OK   ", url, " -> ", row.filename, " [", row.bytes, " B, ", row.ms, " ms, ", row.status, "]");
                }

                {
                    std::lock_guard<std::mutex> lk(results_mu_);
                    results_.push_back(std::move(row));
                }

                size_t k = ++done;
                if (k % 5 == 0 || k == total) {
                    logx::info("Progress: ", k, "/", total);
                }
            });
        }
    } // pool destructs here (joins all threads)

    save_manifest();
}

