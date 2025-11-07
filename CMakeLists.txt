cmake_minimum_required(VERSION 3.20)
project(fellowship CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
if(MSVC)
  add_compile_options(/O2 /DNDEBUG)
else()
  add_compile_options(-O3 -DNDEBUG)
endif()

include(FetchContent)

# simdjson
FetchContent_Declare(simdjson
  GIT_REPOSITORY https://github.com/simdjson/simdjson.git
  GIT_TAG v3.10.1
)
FetchContent_MakeAvailable(simdjson)

# nlohmann
FetchContent_Declare(nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(enter src/main.cpp src/io.cpp src/api.cpp src/opt.cpp)

target_link_libraries(enter PRIVATE simdjson::simdjson nlohmann_json::nlohmann_json)

# poppler
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLERCPP REQUIRED poppler-cpp)

target_include_directories(enter PRIVATE ${POPPLERCPP_INCLUDE_DIRS})
target_compile_options(enter PRIVATE ${POPPLERCPP_CFLAGS_OTHER})
target_link_libraries(enter PRIVATE ${POPPLERCPP_LIBRARIES})

# libcurl
find_package(CURL REQUIRED)
target_link_libraries(enter PRIVATE CURL::libcurl)
