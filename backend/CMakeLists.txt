cmake_minimum_required(VERSION 3.10)
project(LogAnalyzerBackend VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SQLite3 package
find_package(SQLite3 REQUIRED)

# Include directories
include_directories(include third_party)

# Source files
set(SOURCES
    src/main.cpp
    src/HashUtils.cpp
    src/BloomFilter.cpp
    src/CountMinSketch.cpp
    src/HyperLogLog.cpp
    src/SpaceSaving.cpp
    src/DatabaseManager.cpp
    src/LogReceiver.cpp
)

# Executable configuration
add_executable(log_analyzer ${SOURCES})

# Link libraries (SQLite3 and standard system threads for the HTTP server library)
target_link_libraries(log_analyzer PRIVATE sqlite3 pthread)