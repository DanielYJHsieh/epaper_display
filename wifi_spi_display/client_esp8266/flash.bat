@echo off
REM ESP8266 Client 快速編譯和燒錄腳本
REM 記憶體優化版 v1.1

echo ======================================
echo ESP8266 Client - 編譯和燒錄工具
echo 記憶體優化版 v1.1
echo ======================================
echo.

REM 檢查是否在正確的目錄
if not exist "client_esp8266.ino" (
    echo 錯誤：找不到 client_esp8266.ino
    echo 請在 client_esp8266 目錄中運行此腳本
    pause
    exit /b 1
)

REM 步驟 1：編譯
echo [1/3] 開始編譯...
arduino-cli compile --fqbn esp8266:esp8266:d1_mini client_esp8266.ino

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ✗ 編譯失敗！
    pause
    exit /b 1
)

echo.
echo ✓ 編譯成功！
echo.

REM 步驟 2：檢查開發板
echo [2/3] 檢查開發板連接...
arduino-cli board list

echo.
echo 請確認開發板已連接到 COM 埠
echo.

REM 步驟 3：燒錄
set /p PORT="請輸入 COM 埠號（例如：COM5）: "

echo.
echo [3/3] 開始燒錄到 %PORT%...
arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port %PORT% client_esp8266.ino

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ✗ 燒錄失敗！
    echo 請檢查：
    echo   1. COM 埠是否正確
    echo   2. 開發板是否已連接
    echo   3. 是否有其他程式佔用序列埠
    pause
    exit /b 1
)

echo.
echo ======================================
echo ✓ 燒錄成功！
echo ======================================
echo.
echo 您現在可以：
echo   1. 使用 arduino-cli monitor --port %PORT% --config baudrate=115200 監控輸出
echo   2. 啟動 server 來測試顯示功能
echo.

pause
