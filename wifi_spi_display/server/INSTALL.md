# Server 端安裝指南

## ⚠️ 前置需求

您需要安裝 **Python 3.8 或更新版本**，並確保 `pip` 可用。

## 📋 檢查 Python 安裝

### 方法 1: 檢查是否已安裝 Python
在 PowerShell 或 CMD 執行：
```powershell
python --version
```

如果顯示版本號（如 `Python 3.12.x`），表示已安裝。

### 方法 2: 使用 Python Launcher（Windows）
```powershell
py --version
```

## 🔧 安裝 Python（如果尚未安裝）

### 選項 A: 從官方網站安裝（推薦）
1. 前往 https://www.python.org/downloads/
2. 下載 Windows 安裝程式
3. **重要**: 安裝時勾選 "Add Python to PATH"
4. 完成安裝後，**重新開啟 PowerShell**

### 選項 B: 使用 Chocolatey
```powershell
choco install python
```

### 選項 C: 使用 winget
```powershell
winget install Python.Python.3.12
```

## 📦 安裝依賴套件

安裝 Python 後，使用以下任一方法安裝依賴：

### 方法 1: 使用 pip（推薦）
```powershell
cd server
pip install -r requirements.txt
```

### 方法 2: 使用 Python 模組
如果 `pip` 命令不可用，試試：
```powershell
python -m pip install -r requirements.txt
```

### 方法 3: 使用 Python Launcher
```powershell
py -m pip install -r requirements.txt
```

### 方法 4: 手動安裝每個套件
```powershell
pip install websockets>=11.0
pip install Pillow>=10.0.0
pip install numpy>=1.24.0
```

## ✅ 驗證安裝

檢查所有套件是否已安裝：
```powershell
python -c "import websockets, PIL, numpy; print('✅ 所有套件已安裝')"
```

或使用 Python Launcher：
```powershell
py -c "import websockets, PIL, numpy; print('✅ 所有套件已安裝')"
```

## 🚀 執行 Server

安裝完成後：
```powershell
python server.py
```

或：
```powershell
py server.py
```

應該會看到：
```
=================================
WiFi SPI Display Server v1.0
=================================
啟動 WebSocket Server: 0.0.0.0:8266
等待客戶端連線...
```

## 🐛 常見問題

### Q: `pip: 無法辨識 'pip' 詞彙...`
**原因**: pip 不在系統 PATH 中

**解決方法**:
1. 使用 `python -m pip` 代替 `pip`
2. 或重新安裝 Python 並確保勾選 "Add to PATH"

### Q: `ModuleNotFoundError: No module named 'pip'`
**原因**: pip 沒有安裝

**解決方法**:
```powershell
# 下載 get-pip.py
Invoke-WebRequest -Uri https://bootstrap.pypa.io/get-pip.py -OutFile get-pip.py

# 安裝 pip
python get-pip.py
```

### Q: `No module named 'websockets'`
**原因**: 依賴套件未安裝

**解決方法**: 依照上述方法安裝 requirements.txt

### Q: 權限錯誤
**解決方法**: 使用管理員權限執行 PowerShell，或使用虛擬環境

## 🔄 使用虛擬環境（建議）

為了避免套件衝突，建議使用虛擬環境：

```powershell
# 建立虛擬環境
python -m venv venv

# 啟動虛擬環境 (PowerShell)
.\venv\Scripts\Activate.ps1

# 啟動虛擬環境 (CMD)
.\venv\Scripts\activate.bat

# 安裝依賴
pip install -r requirements.txt

# 執行 server
python server.py

# 完成後離開虛擬環境
deactivate
```

如果遇到 PowerShell 執行政策問題：
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

## 📝 快速啟動腳本

我們也提供了批次檔案快速啟動：

### Windows (PowerShell)
執行 `start_server.ps1`

### Windows (CMD)
執行 `start_server.bat`

---

如有其他問題，請參考主 README.md 或提交 Issue。
