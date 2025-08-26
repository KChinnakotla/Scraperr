cmake_minimum_required(VERSION 3.15)
project(cpp_scraper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(CURL REQUIRED)

add_executable(scraper
  src/main.cpp
  src/scraper.cpp
  src/fetcher.cpp
)

target_include_directories(scraper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(scraper PRIVATE CURL::libcurl)
