# Makefile for StockQuery
# 需要先安裝 libcurl 和 nlohmann/json

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = StockQuery
SRC = main.cpp

# 檢查作業系統
UNAME_S := $(shell uname -s)

# 根據作業系統設定連結庫
ifeq ($(UNAME_S),Linux)
    # Linux: 動態連結
    LIBS = -lcurl
    STATIC_FLAGS =
endif
ifeq ($(UNAME_S),Darwin)
    # macOS: 動態連結
    LIBS = -lcurl
    STATIC_FLAGS =
endif
ifeq ($(OS),Windows_NT)
    # Windows: 使用動態連結 libcurl，靜態連結 C++ 標準庫
    # 這樣可以避免 DLL 依賴問題，同時保持 .exe 文件大小合理
    LIBS = -lcurl -lws2_32 -lwinmm
    # 只靜態連結 C++ 標準庫，libcurl 使用動態連結
    STATIC_FLAGS = -static-libgcc -static-libstdc++
    # 如果需要完全靜態連結，請使用: make STATIC=1
    ifeq ($(STATIC),1)
        # 完全靜態連結（需要靜態版本的 libcurl）
        LIBDIR = /mingw64/lib
        LIBS = -L$(LIBDIR) -lcurl -lws2_32 -lwinmm -lcrypt32 -lssl -lcrypto -lz
        STATIC_FLAGS = -static -static-libgcc -static-libstdc++
    endif
endif

# 如果 nlohmann/json 是系統安裝的
# 否則需要手動下載到 include 目錄
INCLUDES = -I./include

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(STATIC_FLAGS) $(INCLUDES) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: clean
