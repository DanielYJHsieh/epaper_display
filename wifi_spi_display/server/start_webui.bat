@echo off
REM ===================================================
REM ESP8266 WiFi Display Server - Web UI 版本啟動腳本
REM ===================================================
REM 
REM 功能：
REM - 啟動 WebSocket Server (port 8266)
REM - 啟動 HTTP Web UI (port 8080)
REM - 支援瀏覽器控制介面
REM 
REM 使用方式：
REM   雙擊此檔案即可啟動
REM 
REM 網頁介面：
REM   本機: http://localhost:8080
REM   網路: http://[你的IP]:8080
REM ===================================================

echo.
echo ====================================
echo   ESP8266 Display Server - Web UI
echo ====================================
echo.
echo 啟動 Web UI 版本...
echo.

REM 切換到 server 目錄
cd /d "%~dp0"

REM 檢查 Python 是否安裝
python --version >nul 2>&1
if errorlevel 1 (
    echo [錯誤] 找不到 Python！
    echo 請先安裝 Python 3.7 或更新版本
    echo.
    pause
    exit /b 1
)

REM 檢查依賴是否安裝
echo 檢查依賴套件...
python -c "import websockets, PIL, numpy, aiohttp" >nul 2>&1
if errorlevel 1 (
    echo.
    echo [警告] 缺少必要的依賴套件！
    echo.
    echo 是否要自動安裝依賴套件？
    echo   1. 是（推薦）
    echo   2. 否（手動安裝）
    echo.
    choice /c 12 /n /m "請選擇 [1/2]: "
    
    if errorlevel 2 (
        echo.
        echo 請手動執行: install_webui_deps.bat
        echo 或執行: pip install websockets pillow numpy aiohttp
        echo.
        pause
        exit /b 1
    )
    
    echo.
    echo 正在安裝依賴套件...
    pip install websockets pillow numpy aiohttp
    
    if errorlevel 1 (
        echo.
        echo [錯誤] 依賴套件安裝失敗！
        echo.
        pause
        exit /b 1
    )
    
    echo.
    echo [成功] 依賴套件安裝完成！
    echo.
)

REM 啟動 Server
echo.
echo ====================================
echo   正在啟動 Server...
echo ====================================
echo.
echo 提示：
echo   - WebSocket Server: ws://0.0.0.0:8266
echo   - Web UI: http://localhost:8080
echo   - 按 Ctrl+C 停止 Server
echo.
echo ====================================
echo.

python server_with_webui.py

REM Server 結束後
echo.
echo Server 已停止。
echo.
pause
