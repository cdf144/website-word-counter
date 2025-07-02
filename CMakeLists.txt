cmake_minimum_required(VERSION 4.0)

project(website-word-counter
        VERSION 0.1
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(website-word-counter
    src/main.cpp
    src/url.cpp
    src/http_client.cpp
    src/word_counter.cpp
)

find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)
target_link_libraries(website-word-counter
    PRIVATE
        ${CURL_LIBRARIES}
        ${LIBXML2_LIBRARIES}
)

target_include_directories(website-word-counter
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${LIBXML2_INCLUDE_DIR}
)

set_target_properties(website-word-counter
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

target_compile_options(website-word-counter
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>: -Wall -Wextra -Wpedantic -Werror>
)
