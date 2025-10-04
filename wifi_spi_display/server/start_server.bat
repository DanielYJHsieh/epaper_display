@echo off
REM WiFi SPI Display Server - 啟動腳本 (Windows)

echo =====================================
echo WiFi SPI Display Server 啟動器
echo =====================================
echo.

REM 檢查 Python
where python >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [錯誤] 找不到 Python！
    echo.
    echo 請先安裝 Python 3.8 或更新版本
    echo 下載: https://www.python.org/downloads/
    echo.
    pause
    exit /b 1
)

REM 顯示 Python 版本
echo [檢查] Python 版本:
python --version
echo.

REM 檢查依賴套件
echo [檢查] 驗證依賴套件...
python -c "import websockets, PIL, numpy" >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [警告] 依賴套件未安裝或不完整
    echo.
    echo 嘗試自動安裝依賴...
    python -m pip install -r requirements.txt
    if %ERRORLEVEL% NEQ 0 (
        echo [錯誤] 安裝失敗！請手動執行:
        echo   python -m pip install -r requirements.txt
        echo.
        pause
        exit /b 1
    )
    echo [完成] 依賴套件已安裝
    echo.
)

REM 啟動 Server
echo [啟動] WiFi SPI Display Server...
echo.
python server.py

REM 如果 Server 結束，等待使用者按鍵
pause
