# 靜態連結說明

## 問題

在 Windows 上使用 MinGW 進行靜態連結時，可能會遇到找不到 libcurl 靜態庫的問題。

## 解決方案

### 方案 1: 使用動態連結（推薦，簡單）

如果不需要完全靜態連結，可以使用動態連結：

```bash
make
```

這會生成一個需要 libcurl DLL 的 .exe 文件。您需要將以下 DLL 文件與 .exe 放在同一目錄：
- `libcurl-4.dll` 或 `libcurl.dll`
- 可能還需要其他依賴 DLL（如 `libssl-3-x64.dll`, `libcrypto-3-x64.dll` 等）

### 方案 2: 安裝靜態版本的 libcurl

#### 使用 vcpkg（推薦）

```powershell
vcpkg install curl:x64-windows-static
```

然後修改 Makefile，使用 vcpkg 的庫路徑。

#### 使用 MSYS2

MSYS2 默認安裝的 libcurl 可能是動態連結的。要獲得靜態庫，您可能需要：

1. 從源碼編譯 libcurl 靜態版本
2. 或使用預編譯的靜態庫

### 方案 3: 使用 Makefile.static（實驗性）

如果您的系統有靜態版本的 libcurl，可以嘗試：

```bash
make -f Makefile.static
```

### 方案 4: 手動指定庫路徑

如果您的 libcurl 靜態庫在其他位置，可以手動修改 Makefile 中的 `LIBDIR` 變數。

## 檢查是否有靜態庫

在 MSYS2/MinGW 終端中執行：

```bash
ls /mingw64/lib/libcurl.a
```

如果文件存在，則有靜態庫；如果不存在，則只有動態庫。

## 當前狀態

- ✅ C++ 標準庫已靜態連結（`-static-libgcc -static-libstdc++`）
- ⚠️ libcurl 可能需要動態連結（取決於安裝的版本）

## 建議

對於大多數用途，動態連結 libcurl 就足夠了。只需要確保目標機器上有必要的 DLL 文件即可。
