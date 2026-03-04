cmake_minimum_required(VERSION 3.14)

project(
    VirtaParser
    VERSION 0.2.1
    DESCRIPTION "Parser for economical game 'Virtonomics'. Currently parses companies and retail markets."
    LANGUAGES CXX
)

add_library(CompanyParse OBJECT src/company.cpp src/unit.cpp)
add_executable(VirtaParser)

target_sources(VirtaParser PRIVATE src/menu.cpp src/virta.cpp src/retail.cpp)

target_link_libraries(VirtaParser PRIVATE curl)
target_link_libraries(VirtaParser PRIVATE CompanyParse)
target_compile_features(VirtaParser PRIVATE cxx_std_20)
