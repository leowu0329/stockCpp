# 台灣股市查詢程式

這是一個使用 C++ 開發的台灣證券交易所（TWSE）股票查詢程式，可以查詢指定股票的每日交易資料。

## 功能特色

- 主選單介面，操作簡單直觀
- 個股資訊查詢：查詢單一股票的詳細交易資料
- 多筆個股當日資訊：一次查詢多檔股票的當日交易資訊
- 支援指定日期查詢
- 顯示完整的交易資訊（開盤價、收盤價、成交量等）
- 股票代號列表可自訂（stocks.json）
- **部分靜態連結：C++ 標準庫已打包到 .exe，libcurl 使用動態連結**

## 系統需求

- C++17 或更高版本
- CMake 3.10 或更高版本（可選）
- libcurl 開發庫（需要靜態連結版本）
- Git（用於下載 nlohmann/json）

### Windows 安裝依賴

#### 使用 MSYS2/MinGW-w64（推薦）

1. 安裝 MSYS2：https://www.msys2.org/
2. 在 MSYS2 終端中執行：

```bash
pacman -S mingw-w64-x86_64-curl mingw-w64-x86_64-gcc
```

#### 使用 vcpkg 安裝 libcurl

```powershell
vcpkg install curl:x64-windows-static
```

#### 使用 Chocolatey

```powershell
choco install curl
```

### Linux 安裝依賴

Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install libcurl4-openssl-dev cmake build-essential
```

Fedora/RHEL:

```bash
sudo dnf install libcurl-devel cmake gcc-c++
```

### macOS 安裝依賴

```bash
brew install curl cmake
```

## 編譯方式

### 使用 Makefile（推薦）

```bash
# 默認編譯（動態連結 libcurl，靜態連結 C++ 標準庫）
make

# 如果需要完全靜態連結（需要靜態版本的 libcurl）
make STATIC=1
```

默認配置會靜態連結 C++ 標準庫，動態連結 libcurl。生成的 .exe 需要 `libcurl-4.dll` 或 `libcurl.dll` 文件。

### 使用 PowerShell 編譯腳本（Windows）

```powershell
.\build.ps1
```

### 使用 CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

CMake 會自動下載 nlohmann/json，並在 Windows 上使用靜態連結。

### Windows (Visual Studio)

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## 連結方式說明

### 默認配置（推薦）

默認使用**部分靜態連結**：

- ✅ **靜態連結**：C++ 標準庫（libstdc++）、GCC 運行時庫（libgcc）
- ⚠️ **動態連結**：libcurl（需要 DLL 文件）

**優點**：

- .exe 文件較小（通常 1-2 MB）
- 編譯成功率高，無需特殊配置

**缺點**：

- 需要將 `libcurl-4.dll`（或 `libcurl.dll`）與 .exe 放在同一目錄
- DLL 文件通常位於 `C:\msys64\mingw64\bin\libcurl-4.dll`

### 完全靜態連結

如果需要完全靜態連結（所有庫都打包到 .exe），可以使用：

```bash
make STATIC=1
```

**要求**：

- 需要安裝靜態版本的 libcurl
- 可能需要額外的依賴庫（ssl, crypto, z 等）

**優點**：

- .exe 可以完全獨立運行，無需任何 DLL

**缺點**：

- .exe 文件較大（通常 5-10 MB）
- 需要正確配置靜態庫路徑

### 獲取 libcurl DLL

如果使用默認配置，需要將 libcurl DLL 複製到 .exe 所在目錄：

```powershell
# 從 MSYS2 複製 DLL
Copy-Item "C:\msys64\mingw64\bin\libcurl-4.dll" -Destination "."
```

或者將 `C:\msys64\mingw64\bin` 添加到系統 PATH 環境變數中。

詳細說明請參考 `STATIC_LINKING.md`。

## 使用方法

### 主選單模式（推薦）

```bash
./StockQuery
```

程式會顯示主選單：

```
========================================
        台灣股市查詢系統
========================================
1. 個股資訊
2. 多筆個股當日資訊
0. 離開
========================================
請選擇功能 (0-2):
```

#### 功能 1: 個股資訊

- 輸入股票代碼（例如：2330）
- 可選擇查詢日期，或使用當前日期
- 顯示該股票的詳細交易資訊

#### 功能 2: 多筆個股當日資訊

- 自動讀取 `stocks.json` 檔案中的股票列表
- 一次查詢多檔股票的當日資訊
- 以表格形式顯示所有股票的關鍵資訊

### 命令列模式（保留原有功能）

```bash
# 查詢單一股票
./StockQuery 2330

# 查詢指定日期
./StockQuery 2330 20240101
```

### 自訂股票列表

編輯 `stocks.json` 檔案，可以新增或修改要查詢的股票：

```json
{
  "stocks": [
    {
      "code": "2330",
      "name": "台積電"
    },
    {
      "code": "2317",
      "name": "鴻海"
    }
  ]
}
```

## 範例輸出

```
請輸入股票代碼 (例如: 2330 為台積電): 2330
使用日期: 20240101 (格式: YYYYMMDD)

正在查詢股票 2330 的資料...

========================================
股票代碼: 2330
查詢日期: 20240101 (113/01/01)
標題: 2330 台積電 2024年01月 每日收盤行情

交易資料:
------------------------------------------------------------------------------------------------------------------------
日期          成交股數      成交金額      開盤價        最高價        最低價        收盤價        漲跌價差      成交筆數
------------------------------------------------------------------------------------------------------------------------
113/01/02     12345678      1234567890    580.00        585.00        578.00        582.00        2.00          12345

========================================
```

## 常見股票代碼

- 2330: 台積電
- 2317: 鴻海
- 2454: 聯發科
- 2308: 台達電
- 2412: 中華電

## API 說明

本程式使用台灣證券交易所的公開 API：

```
https://www.twse.com.tw/exchangeReport/STOCK_DAY?response=json&date=YYYYMMDD&stockNo=股票代碼
```

## 注意事項

1. API 可能有請求頻率限制，請勿過度頻繁查詢
2. 查詢日期必須是交易日，非交易日可能無法取得資料
3. 某些歷史日期可能無法查詢
4. **DLL 依賴**：使用默認配置時，需要 `libcurl-4.dll` 文件。請確保 DLL 與 .exe 在同一目錄，或將 MSYS2 的 bin 目錄添加到 PATH
5. 如果遇到 DLL 缺失錯誤，請檢查 `C:\msys64\mingw64\bin\` 目錄中是否有 `libcurl-4.dll` 文件

## 授權

本專案僅供學習和個人使用。

## 問題回報

如有問題或建議，歡迎提出 Issue。
