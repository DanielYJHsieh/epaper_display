@echo off
echo ========================================
echo        ESP8266 Arduino IDE 啟動器
echo ========================================
echo 正在搜尋 Arduino IDE...
echo.

REM 首先檢查您系統上確認存在的路徑
if exist "C:\Program Files\Arduino IDE\Arduino IDE.exe" (
    echo ✓ 找到 Arduino IDE 2.x 於: C:\Program Files\Arduino IDE\
    echo 正在啟動 Arduino IDE...
    start "" "C:\Program Files\Arduino IDE\Arduino IDE.exe"
    echo.
    echo Arduino IDE 已啟動！
    echo 請在 Arduino IDE 中：
    echo 1. 安裝 ESP8266 開發板支援
    echo 2. 選擇正確的開發板（NodeMCU 1.0 或 Wemos D1 Mini）
    echo 3. 選擇正確的連接埠
    goto :success
)

REM 備用路徑檢查
if exist "C:\Program Files\Arduino\arduino.exe" (
    echo ✓ 找到 Arduino IDE 1.x 於: C:\Program Files\Arduino\
    start "" "C:\Program Files\Arduino\arduino.exe"
    goto :success
)

if exist "C:\Program Files (x86)\Arduino\arduino.exe" (
    echo ✓ 找到 Arduino IDE 1.x (x86) 於: C:\Program Files (x86)\Arduino\
    start "" "C:\Program Files (x86)\Arduino\arduino.exe"
    goto :success
)

if exist "%LOCALAPPDATA%\Programs\Arduino IDE\Arduino IDE.exe" (
    echo ✓ 找到 Arduino IDE 2.x 於用戶目錄
    start "" "%LOCALAPPDATA%\Programs\Arduino IDE\Arduino IDE.exe"
    goto :success
)

REM 如果都沒找到，嘗試通過 PATH 啟動
echo ⚠️  未在常見位置找到 Arduino IDE
echo 嘗試通過系統 PATH 啟動...
start arduino
if %errorlevel% equ 0 (
    goto :success
)

REM 如果都失敗了
echo.
echo ❌ 無法找到或啟動 Arduino IDE
echo.
echo 解決方案：
echo 1. 請從 https://www.arduino.cc/en/software 下載並安裝 Arduino IDE
echo 2. 確保安裝在預設位置
echo 3. 重新執行此批次檔案
goto :end

:success
echo.
echo ========================================
echo           設定提醒
echo ========================================
echo 📌 ESP8266 開發板設定：
echo.
echo 1. 檔案 → 偏好設定
echo 2. 額外的開發板管理員網址：
echo    http://arduino.esp8266.com/stable/package_esp8266com_index.json
echo.
echo 3. 工具 → 開發板 → 開發板管理員
echo 4. 搜尋並安裝 "ESP8266 by ESP8266 Community"
echo.
echo 5. 選擇開發板：
echo    - NodeMCU 1.0 (ESP-12E Module) [推薦]
echo    - LOLIN(WEMOS) D1 R2 & mini
echo.
echo 6. 設定參數：
echo    - CPU Frequency: 80 MHz
echo    - Flash Size: 4MB
echo    - Upload Speed: 115200
echo.
echo ✅ 設定完成後即可開始開發！

:end
echo.
echo 按任意鍵關閉視窗...
pause >nul
