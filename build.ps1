# PowerShell 编译脚本
# 用于在 Windows 上编译 StockQuery

$ErrorActionPreference = "Stop"

Write-Host "正在编译 StockQuery..." -ForegroundColor Green

# 检查 nlohmann/json.hpp 是否存在
if (-not (Test-Path "include/nlohmann/json.hpp")) {
    Write-Host "错误: 找不到 include/nlohmann/json.hpp" -ForegroundColor Red
    Write-Host "正在下载 nlohmann/json.hpp..." -ForegroundColor Yellow
    try {
        Invoke-WebRequest -Uri "https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp" -OutFile "include/nlohmann/json.hpp"
        Write-Host "下载成功!" -ForegroundColor Green
    } catch {
        Write-Host "下载失败: $_" -ForegroundColor Red
        exit 1
    }
}

# 编译命令（静态链接）
$compiler = "g++"
$cxxflags = "-std=c++17 -Wall -Wextra -O2"
$staticFlags = "-static -static-libgcc -static-libstdc++"
$includes = "-I./include"
$source = "main.cpp"
$output = "StockQuery.exe"
# Windows 静态链接所需的库
$libs = "-lcurl -lws2_32 -lwinmm -lcrypt32 -lssl -lcrypto -lz"

$command = "$compiler $cxxflags $staticFlags $includes -o $output $source $libs"

Write-Host "执行: $command" -ForegroundColor Cyan

try {
    Invoke-Expression $command
    if ($LASTEXITCODE -eq 0) {
        Write-Host "`n编译成功! 可执行文件: $output" -ForegroundColor Green
    } else {
        Write-Host "`n编译失败 (退出代码: $LASTEXITCODE)" -ForegroundColor Red
        exit $LASTEXITCODE
    }
} catch {
    Write-Host "`n编译错误: $_" -ForegroundColor Red
    exit 1
}
