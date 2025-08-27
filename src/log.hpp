#pragma once
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace logx {
inline std::mutex& mu() { static std::mutex m; return m; }

inline std::string now() {
    using namespace std::chrono;
    auto t = system_clock::to_time_t(system_clock::now());
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%F %T");
    return oss.str();
}

template <typename... Args>
inline void info(Args&&... args) {
    std::lock_guard<std::mutex> lock(mu());
    std::cout << "[" << now() << "] [INFO] ";
    (std::cout << ... << args) << std::endl;
}

template <typename... Args>
inline void warn(Args&&... args) {
    std::lock_guard<std::mutex> lock(mu());
    std::cerr << "[" << now() << "] [WARN] ";
    (std::cerr << ... << args) << std::endl;
}

template <typename... Args>
inline void error(Args&&... args) {
    std::lock_guard<std::mutex> lock(mu());
    std::cerr << "[" << now() << "] [ERR ] ";
    (std::cerr << ... << args) << std::endl;
}
} // namespace logx
