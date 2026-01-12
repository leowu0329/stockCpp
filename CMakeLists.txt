cmake_minimum_required(VERSION 3.10)
project(StockQuery)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows 靜態連結設定
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

# 尋找 libcurl
find_package(CURL REQUIRED)

# 包含目錄
include_directories(${CURL_INCLUDE_DIRS})

# 下載 nlohmann/json (如果沒有安裝)
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# 可執行文件
add_executable(${PROJECT_NAME} main.cpp)

# 連結庫
target_link_libraries(${PROJECT_NAME} 
    ${CURL_LIBRARIES}
    nlohmann_json::nlohmann_json
)

# Windows 特定設定
if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32 winmm crypt32 ssl crypto z)
    # 嘗試靜態連結 libcurl
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "-static -static-libgcc -static-libstdc++"
    )
endif()
