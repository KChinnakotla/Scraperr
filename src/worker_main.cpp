#include "fetcher.hpp"
#include "log.hpp"
#include "scraper.hpp"
#include "util.hpp"
#include <chrono>
#include <regex>
#include <thread>

static std::pair<std::string,std::string> parse_job_line(const std::string& s) {
    auto sp = s.find(' ');
    if (sp == std::string::npos) return {"",""};
    return {s.substr(0, sp), s.substr(sp+1)};
}

int main(int argc, char** argv) {
    if (argc < 2) { logx::error("Usage: worker <COORD_BASE_URL> [concurrency]"); return 1; }
    std::string base = argv[1]; // e.g. http://127.0.0.1:8080
    int concurrency = (argc >= 3) ? std::stoi(argv[2]) : 8;

    Fetcher fx;

    // Local scraper options (reuse our per-process concurrency + storage)
    Scraper::Options opt;
    opt.out_dir = "data_worker";
    opt.concurrency = concurrency;
    opt.timeout_ms = 15000;

    for (;;) {
        // Long-poll for a single job
        auto dq = fx.get(base + "/dequeue?worker=cpp", 30000, true);
        if (!dq.error.empty()) { logx::warn("dequeue error:", dq.error); std::this_thread::sleep_for(std::chrono::seconds(1)); continue; }
        if (dq.http_status != 200) { std::this_thread::sleep_for(std::chrono::seconds(1)); continue; }
        if (dq.body.rfind("EMPTY",0) == 0) { std::this_thread::sleep_for(std::chrono::milliseconds(200)); continue; }

        auto [job_id, url] = parse_job_line(dq.body);
        if (job_id.empty()) { std::this_thread::sleep_for(std::chrono::milliseconds(200)); continue; }

        // Fetch the single URL using our Fetcher directly (or Scraper with 1 url)
        auto r = fx.get(url, opt.timeout_ms, true);

        // Save body locally (optional)
        if (r.error.empty() && r.http_status == 200) {
            util::ensure_dir(opt.out_dir);
            util::ensure_dir(util::join_path(opt.out_dir, "pages"));
            auto fname = util::join_path(opt.out_dir, util::join_path("pages", util::basename_from_url(url)));
            std::ofstream out(fname, std::ios::binary);
            out.write(r.body.data(), (std::streamsize)r.body.size());
        }

        // Report result
        std::ostringstream form;
        form << "job_id=" << job_id
             << "&status=" << r.http_status
             << "&bytes=" << r.body.size()
             << "&ms=" << r.duration_ms
             << "&error=" << (r.error.empty() ? "" : r.error);

        auto rep = fx.post_form(base + "/report", form.str());
        if (!rep.error.empty() || rep.http_status != 200) {
            logx::warn("report failed: ", rep.http_status, " err=", rep.error);
            // let lease expire -> coordinator will requeue (at-least-once)
        }
    }
}
