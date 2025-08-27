#include "log.hpp"
#include "scraper.hpp"
#include "util.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

struct Args {
    std::string urls_file = "seeds.txt";
    std::string out_dir = "data";
    int concurrency = 16;
    long timeout_ms = 15000;
    int delay_ms = 0;
};

static void usage(const char* prog) {
    std::cout << "Usage: " << prog << " [--urls FILE] [--out DIR] [--concurrency N] [--timeout-ms MS] [--delay-ms MS]\n";
}

static Args parse_args(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string k = argv[i];
        auto need = [&](int& idx) -> const char* {
            if (idx + 1 >= argc) { usage(argv[0]); std::exit(1); }
            return argv[++idx];
        };
        if (k == "--urls") a.urls_file = need(i);
        else if (k == "--out") a.out_dir = need(i);
        else if (k == "--concurrency") a.concurrency = std::stoi(need(i));
        else if (k == "--timeout-ms") a.timeout_ms = std::stol(need(i));
        else if (k == "--delay-ms") a.delay_ms = std::stoi(need(i));
        else if (k == "-h" || k == "--help") { usage(argv[0]); std::exit(0); }
        else { std::cerr << "Unknown flag: " << k << "\n"; usage(argv[0]); std::exit(1); }
    }
    return a;
}

int main(int argc, char** argv) {
    auto args = parse_args(argc, argv);
    auto urls = util::read_lines(args.urls_file);
    if (urls.empty()) {
        logx::error("No URLs found in ", args.urls_file);
        return 2;
    }

    logx::info("Starting scraper: urls=", urls.size(),
               " concurrency=", args.concurrency,
               " timeout_ms=", args.timeout_ms,
               " delay_ms=", args.delay_ms,
               " out_dir=", args.out_dir);

    Scraper::Options opt;
    opt.out_dir = args.out_dir;
    opt.concurrency = args.concurrency;
    opt.timeout_ms = args.timeout_ms;
    opt.delay_ms = args.delay_ms;

    Scraper s(opt);
    s.scrape_urls(urls);

    logx::info("Done.");
    return 0;
}
