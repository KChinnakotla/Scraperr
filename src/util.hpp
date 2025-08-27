#pragma once
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace util {

inline std::vector<std::string> read_lines(const std::string& path) {
    std::ifstream in(path);
    std::vector<std::string> lines;
    std::string s;
    while (std::getline(in, s)) {
        if (!s.empty()) lines.push_back(s);
    }
    return lines;
}

inline std::string sanitize_filename(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (std::isalnum(static_cast<unsigned char>(c))) out.push_back(c);
        else out.push_back('_');
    }
    return out;
}

inline std::string join_path(const std::string& a, const std::string& b) {
    return (std::filesystem::path(a) / std::filesystem::path(b)).string();
}

inline void ensure_dir(const std::string& dir) {
    std::filesystem::create_directories(dir);
}

inline std::string basename_from_url(const std::string& url) {
    // crude: use sanitized url tail; collisions acceptable for v0
    auto pos = url.find("://");
    std::string tail = (pos == std::string::npos) ? url : url.substr(pos + 3);
    return sanitize_filename(tail) + ".html";
}

} // namespace util
