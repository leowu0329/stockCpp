#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <limits>
#include <ctime>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// libcurl 回呼函數，用於接收HTTP回應資料
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// 股票資訊結構
struct StockInfo {
    std::string code;
    std::string name;
    std::string date;
    std::string openPrice;
    std::string closePrice;
    std::string highPrice;
    std::string lowPrice;
    std::string volume;
    std::string amount;
};

// 股票查詢類
class StockQuery {
private:
    CURL* curl;
    std::string baseUrl;

public:
    StockQuery() {
        curl = curl_easy_init();
        baseUrl = "https://www.twse.com.tw/exchangeReport/STOCK_DAY";
    }

    ~StockQuery() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
    }

    // 查詢指定日期的股票資料
    bool queryStock(const std::string& stockNo, const std::string& date) {
        if (!curl) {
            std::cerr << "錯誤: 無法初始化 libcurl" << std::endl;
            return false;
        }

        // 構建完整的URL
        std::string url = baseUrl + "?response=json&date=" + date + "&stockNo=" + stockNo;
        
        std::string readBuffer;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // 簡化SSL驗證

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "HTTP 請求失敗: " << curl_easy_strerror(res) << std::endl;
            return false;
        }

        // 解析JSON回應
        try {
            json jsonData = json::parse(readBuffer);
            
            // 檢查是否有錯誤
            if (jsonData.contains("stat") && jsonData["stat"] != "OK") {
                std::cerr << "API 錯誤: " << jsonData.value("stat", "未知錯誤") << std::endl;
                if (jsonData.contains("message")) {
                    std::cerr << "訊息: " << jsonData["message"] << std::endl;
                }
                return false;
            }

            // 顯示股票資訊
            displayStockInfo(jsonData, stockNo, date);
            return true;

        } catch (const json::parse_error& e) {
            std::cerr << "JSON 解析錯誤: " << e.what() << std::endl;
            std::cerr << "回應內容: " << readBuffer.substr(0, 500) << std::endl;
            return false;
        } catch (const std::exception& e) {
            std::cerr << "處理資料時發生錯誤: " << e.what() << std::endl;
            return false;
        }
    }

    // 查詢單個股票的當日資訊（返回 StockInfo）
    bool queryStockToday(const std::string& stockNo, const std::string& date, StockInfo& info) {
        if (!curl) {
            return false;
        }

        std::string url = baseUrl + "?response=json&date=" + date + "&stockNo=" + stockNo;
        std::string readBuffer;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            return false;
        }

        try {
            json jsonData = json::parse(readBuffer);
            
            if (jsonData.contains("stat") && jsonData["stat"] != "OK") {
                return false;
            }

            std::string rocDate = convertToRocDate(date);
            if (rocDate.empty()) {
                return false;
            }

            info.code = stockNo;
            info.date = date;

            if (jsonData.contains("data") && jsonData["data"].is_array()) {
                auto dataArray = jsonData["data"];
                
                for (const auto& row : dataArray) {
                    if (row.is_array() && row.size() > 0) {
                        std::string rowDate = row[0].is_string() ? row[0].get<std::string>() : "";
                        
                        if (rowDate == rocDate) {
                            // 找到匹配的日期
                            if (row.size() > 6) {
                                info.date = rowDate;
                                info.volume = row[1].is_string() ? row[1].get<std::string>() : "";
                                info.amount = row[2].is_string() ? row[2].get<std::string>() : "";
                                info.openPrice = row[3].is_string() ? row[3].get<std::string>() : "";
                                info.highPrice = row[4].is_string() ? row[4].get<std::string>() : "";
                                info.lowPrice = row[5].is_string() ? row[5].get<std::string>() : "";
                                info.closePrice = row[6].is_string() ? row[6].get<std::string>() : "";
                            }
                            return true;
                        }
                    }
                }
            }
            return false;

        } catch (...) {
            return false;
        }
    }

private:
    // 將西元年日期 (YYYYMMDD) 轉換為民國年格式 (YYY/MM/DD)
    std::string convertToRocDate(const std::string& dateStr) {
        if (dateStr.length() != 8) {
            return "";
        }
        
        int year = std::stoi(dateStr.substr(0, 4));
        int rocYear = year - 1911; // 民國年 = 西元年 - 1911
        std::string month = dateStr.substr(4, 2);
        std::string day = dateStr.substr(6, 2);
        
        std::ostringstream oss;
        oss << rocYear << "/" << month << "/" << day;
        return oss.str();
    }

    // 顯示股票資訊
    void displayStockInfo(const json& jsonData, const std::string& stockNo, const std::string& queryDate) {
        std::cout << "\n========================================\n";
        std::cout << "股票代碼: " << stockNo << std::endl;
        
        // 轉換查詢日期為民國年格式
        std::string rocDate = convertToRocDate(queryDate);
        if (!rocDate.empty()) {
            std::cout << "查詢日期: " << queryDate << " (" << rocDate << ")" << std::endl;
        }
        
        if (jsonData.contains("title")) {
            std::cout << "標題: " << jsonData["title"] << std::endl;
        }

        if (jsonData.contains("data") && jsonData["data"].is_array()) {
            std::cout << "\n交易資料:\n";
            std::cout << std::string(120, '-') << std::endl;
            
            // 顯示表頭
            if (jsonData.contains("fields")) {
                std::vector<std::string> fields = jsonData["fields"];
                for (const auto& field : fields) {
                    std::cout << std::setw(12) << std::left << field;
                }
                std::cout << std::endl;
                std::cout << std::string(120, '-') << std::endl;
            }

            // 只顯示指定日期的資料
            auto dataArray = jsonData["data"];
            bool found = false;
            
            for (const auto& row : dataArray) {
                if (row.is_array() && row.size() > 0) {
                    std::string rowDate = row[0].is_string() ? row[0].get<std::string>() : "";
                    
                    // 檢查日期是否匹配（API返回的日期格式為 YYY/MM/DD）
                    if (!rocDate.empty() && rowDate == rocDate) {
                        found = true;
                        // 顯示匹配的資料
                        for (const auto& cell : row) {
                            std::string cellStr = cell.is_string() ? cell.get<std::string>() : cell.dump();
                            std::cout << std::setw(12) << std::left << cellStr;
                        }
                        std::cout << std::endl;
                        break; // 找到匹配的日期後就停止
                    }
                }
            }
            
            if (!rocDate.empty() && !found) {
                std::cout << "未找到日期 " << queryDate << " (" << rocDate << ") 的交易資料" << std::endl;
                std::cout << "提示: 該日期可能為非交易日或資料尚未更新" << std::endl;
            } else if (rocDate.empty()) {
                // 如果日期格式不正確，顯示最近10筆資料
                size_t startIdx = dataArray.size() > 10 ? dataArray.size() - 10 : 0;
                for (size_t i = startIdx; i < dataArray.size(); ++i) {
                    auto row = dataArray[i];
                    if (row.is_array()) {
                        for (const auto& cell : row) {
                            std::string cellStr = cell.is_string() ? cell.get<std::string>() : cell.dump();
                            std::cout << std::setw(12) << std::left << cellStr;
                        }
                        std::cout << std::endl;
                    }
                }
            }
        }

        std::cout << "\n========================================\n";
    }
};

// 讀取股票代號列表
bool loadStockList(const std::string& filename, std::vector<StockInfo>& stocks) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "錯誤: 無法開啟檔案 " << filename << std::endl;
        return false;
    }

    try {
        json jsonData;
        file >> jsonData;
        
        if (jsonData.contains("stocks") && jsonData["stocks"].is_array()) {
            for (const auto& stock : jsonData["stocks"]) {
                StockInfo info;
                info.code = stock.value("code", "");
                info.name = stock.value("name", "");
                stocks.push_back(info);
            }
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "讀取股票列表錯誤: " << e.what() << std::endl;
    }
    
    return false;
}

// 獲取當前日期（格式：YYYYMMDD）
std::string getCurrentDate() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d");
    return oss.str();
}

// 清屏函數（跨平台）
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// 等待用戶按Enter
void waitForEnter() {
    std::cout << "\n按一次enter就回主選單";
    // 直接等待用戶按Enter（之前的輸入已經被 getline 處理完）
    // 使用 getline 而不是 get，因為 getline 會自動處理換行符
    std::string dummy;
    std::getline(std::cin, dummy);
}

// 顯示主選單
void showMenu() {
    clearScreen();
    std::cout << "\n========================================\n";
    std::cout << "        台灣股市查詢系統\n";
    std::cout << "========================================\n";
    std::cout << "1. 個股資訊\n";
    std::cout << "2. 多筆個股當日資訊\n";
    std::cout << "0. 離開\n";
    std::cout << "========================================\n";
    std::cout << "請選擇功能 (0-2): ";
}

// 功能1: 個股資訊
void function1_StockInfo(StockQuery& stockQuery) {
    std::string stockNo;
    std::string date;

    clearScreen();
    std::cout << "\n【個股資訊查詢】\n";
    std::cout << "請輸入股票代碼 (例如: 2330): ";
    std::cin >> stockNo;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    date = getCurrentDate();
    std::cout << "使用日期: " << date << " (格式: YYYYMMDD)" << std::endl;
    std::cout << "如需查詢其他日期，請輸入 (直接按Enter使用當前日期): ";
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty()) {
        date = input;
    }

    clearScreen();
    std::cout << "\n正在查詢股票 " << stockNo << " 的資料..." << std::endl;
    stockQuery.queryStock(stockNo, date);
    
    waitForEnter();
}

// 功能2: 多筆個股當日資訊
void function2_MultipleStocksToday(StockQuery& stockQuery) {
    std::vector<StockInfo> stockList;
    
    clearScreen();
    std::cout << "\n【多筆個股當日資訊】\n";
    
    // 讀取股票列表
    if (!loadStockList("stocks.json", stockList)) {
        std::cerr << "無法讀取股票列表檔案 stocks.json" << std::endl;
        waitForEnter();
        return;
    }

    std::cout << "已載入 " << stockList.size() << " 檔股票\n" << std::endl;

    // 獲取查詢日期
    std::string date = getCurrentDate();
    std::cout << "使用日期: " << date << " (格式: YYYYMMDD)" << std::endl;
    std::cout << "如需查詢其他日期，請輸入 (直接按Enter使用當前日期): ";
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty()) {
        date = input;
    }

    clearScreen();
    std::cout << "\n正在查詢多筆股票當日資訊..." << std::endl;
    std::cout << std::string(120, '=') << std::endl;
    std::cout << std::setw(6) << std::left << "代碼"
              << std::setw(10) << std::left << "名稱"
              << std::setw(12) << std::left << "日期"
              << std::setw(12) << std::right << "開盤價"
              << std::setw(12) << std::right << "最高價"
              << std::setw(12) << std::right << "最低價"
              << std::setw(12) << std::right << "收盤價"
              << std::setw(15) << std::right << "成交量"
              << std::setw(15) << std::right << "成交金額"
              << std::endl;
    std::cout << std::string(120, '=') << std::endl;

    int successCount = 0;
    for (auto& stock : stockList) {
        StockInfo info;
        if (stockQuery.queryStockToday(stock.code, date, info)) {
            info.name = stock.name;
            std::cout << std::setw(6) << std::left << info.code
                      << std::setw(10) << std::left << info.name
                      << std::setw(12) << std::left << info.date
                      << std::setw(12) << std::right << info.openPrice
                      << std::setw(12) << std::right << info.highPrice
                      << std::setw(12) << std::right << info.lowPrice
                      << std::setw(12) << std::right << info.closePrice
                      << std::setw(15) << std::right << info.volume
                      << std::setw(15) << std::right << info.amount
                      << std::endl;
            successCount++;
        } else {
            std::cout << std::setw(6) << std::left << stock.code
                      << std::setw(10) << std::left << stock.name
                      << std::setw(12) << std::left << "查詢失敗"
                      << std::endl;
        }
        
        // 避免請求過於頻繁，稍作延遲
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << std::string(120, '=') << std::endl;
    std::cout << "\n查詢完成: 成功 " << successCount << " / " << stockList.size() << " 筆\n" << std::endl;
    
    waitForEnter();
}

int main(int argc, char* argv[]) {
    // 初始化 libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    StockQuery stockQuery;
    int choice;

    // 如果從命令列參數執行，保持原有功能
    if (argc >= 2) {
        std::string stockNo = argv[1];
        std::string date = (argc >= 3) ? argv[2] : getCurrentDate();
        
        std::cout << "\n正在查詢股票 " << stockNo << " 的資料..." << std::endl;
        bool success = stockQuery.queryStock(stockNo, date);
        
        curl_global_cleanup();
        return success ? 0 : 1;
    }

    // 主選單循環
    while (true) {
        showMenu();
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1:
                function1_StockInfo(stockQuery);
                break;
            case 2:
                function2_MultipleStocksToday(stockQuery);
                break;
            case 0:
                clearScreen();
                std::cout << "\n感謝使用，再見！\n" << std::endl;
                curl_global_cleanup();
                return 0;
            default:
                clearScreen();
                std::cout << "\n無效的選擇，請重新輸入！\n" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                break;
        }
    }

    curl_global_cleanup();
    return 0;
}
