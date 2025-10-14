# Raspberry Pi 快速部署指南

> **快速部署 WiFi SPI Display Server 到 Raspberry Pi**
> 
> 適用於: Raspberry Pi 1/2/3/4 (建議使用 Raspberry Pi OS Bullseye)

---

## 🚀 快速開始 (5 分鐘)

### 前置需求

- ✅ Raspberry Pi 已安裝 Raspberry Pi OS (推薦 Bullseye Legacy)
- ✅ 已連接網路 (有線或無線)
- ✅ 已啟用 SSH 存取

### 一鍵部署

```bash
# 1. SSH 連接到 Raspberry Pi
ssh pi@<raspberry-pi-ip>

# 2. 下載專案 (使用 git)
cd ~
git clone https://github.com/<your-username>/epaper_display.git
cd epaper_display/wifi_spi_display/server

# 或使用 rsync 從本機複製 (如果你已經有專案)
# 在本機執行:
# rsync -avz --exclude 'venv' --exclude '__pycache__' \
#   ./epaper_display/wifi_spi_display/server/ \
#   pi@<raspberry-pi-ip>:~/wifi_spi_display/server/

# 3. 執行自動部署腳本
bash deploy_rpi.sh

# 4. 設定開機自動啟動
bash install_service.sh
```

完成！服務現在已經運行並設定為開機自動啟動。

---

## 📋 詳細步驟

### 步驟 1: 準備 Raspberry Pi

#### 1.1 檢查 Python 版本

```bash
python3 --version
# 應顯示: Python 3.9.x (Bullseye) 或 3.11.x (Bookworm)
```

⚠️ **重要**: 
- Raspberry Pi 1 必須使用 Python 3.9 (Bullseye)
- Python 3.13 (Trixie) 不支援，請避免使用

#### 1.2 更新系統 (選用但建議)

```bash
sudo apt update
sudo apt upgrade -y
```

### 步驟 2: 下載專案檔案

**方法 A: 使用 Git (推薦)**

```bash
cd ~
git clone https://github.com/<your-username>/epaper_display.git
cd epaper_display/wifi_spi_display/server
```

**方法 B: 使用 rsync 從本機傳輸**

在本機 Windows 電腦執行:

```powershell
# 使用 PowerShell
cd D:\1_myproject\epaper_display\epaper_display\wifi_spi_display

# 方法 1: 使用 scp
scp -r server pi@192.168.0.41:~/wifi_spi_display/

# 方法 2: 使用 rsync (需要 WSL 或 Git Bash)
rsync -avz --exclude 'venv' --exclude '__pycache__' --exclude '*.pyc' `
  server/ pi@192.168.0.41:~/wifi_spi_display/server/
```

### 步驟 3: 執行自動部署

```bash
cd ~/wifi_spi_display/server  # 或你的專案路徑
bash deploy_rpi.sh
```

**此腳本會自動:**
- ✅ 檢查 Python 版本相容性
- ✅ 安裝系統依賴套件 (libjpeg, zlib, etc.)
- ✅ 建立 Python 虛擬環境
- ✅ 安裝 Python 套件 (aiohttp, Pillow, numpy, etc.)
- ✅ 建立必要目錄 (logs, uploads)
- ✅ 驗證安裝結果

預計時間: 5-15 分鐘 (視網路速度和 Pi 型號)

### 步驟 4: 設定開機自動啟動

```bash
bash install_service.sh
```

**此腳本會自動:**
- ✅ 建立 systemd 服務單元
- ✅ 設定為開機自動啟動
- ✅ 立即啟動服務
- ✅ 顯示服務狀態

### 步驟 5: 驗證安裝

```bash
# 檢查服務狀態
sudo systemctl status wifi_display

# 檢查埠號監聽
netstat -tuln | grep -E '8266|8080'

# 檢查日誌
sudo journalctl -u wifi_display -f
```

**測試 Web UI:**

在瀏覽器開啟: `http://<raspberry-pi-ip>:8080`

---

## 🔧 常用管理命令

### 服務管理

```bash
# 啟動服務
sudo systemctl start wifi_display

# 停止服務
sudo systemctl stop wifi_display

# 重啟服務
sudo systemctl restart wifi_display

# 查看狀態
sudo systemctl status wifi_display

# 查看即時日誌
sudo journalctl -u wifi_display -f

# 查看最近 100 行日誌
sudo journalctl -u wifi_display -n 100
```

### 開機自動啟動

```bash
# 啟用開機自動啟動
sudo systemctl enable wifi_display

# 停用開機自動啟動
sudo systemctl disable wifi_display

# 檢查是否已啟用
sudo systemctl is-enabled wifi_display
```

### 更新程式碼

```bash
# 停止服務
sudo systemctl stop wifi_display

# 更新程式碼 (使用 git)
cd ~/epaper_display
git pull

# 或使用 rsync 從本機更新
# rsync -avz server/ pi@<ip>:~/wifi_spi_display/server/

# 更新 Python 套件 (如果 requirements.txt 有更新)
cd ~/wifi_spi_display/server
source venv/bin/activate
pip install -r requirements_rpi.txt --upgrade

# 重啟服務
sudo systemctl restart wifi_display
```

---

## 🐛 常見問題

### 1. 服務無法啟動

```bash
# 查看詳細錯誤
sudo journalctl -u wifi_display -n 50

# 手動測試執行
cd ~/wifi_spi_display/server
source venv/bin/activate
python server_with_webui.py
```

常見原因:
- Python 套件未安裝完整 → 重新執行 `bash deploy_rpi.sh`
- 埠號被佔用 → 檢查 `netstat -tuln | grep 8266`
- 權限問題 → 確認檔案所有者正確

### 2. Web UI 無法存取

```bash
# 檢查服務是否運行
sudo systemctl status wifi_display

# 檢查埠號
netstat -tuln | grep 8080

# 檢查防火牆 (如果已安裝 ufw)
sudo ufw status
sudo ufw allow 8080/tcp
sudo ufw allow 8266/tcp
```

### 3. Python 版本錯誤

如果看到 "需要 Python 3.9.x" 的錯誤:

- Raspberry Pi 1: 必須使用 Bullseye (Python 3.9)
- Raspberry Pi 2+: 可使用 Bullseye 或 Bookworm

**解決方案**: 重新燒錄正確版本的 Raspberry Pi OS

### 4. 記憶體不足 (Raspberry Pi 1)

如果經常因記憶體不足而重啟:

```bash
# 檢查記憶體使用
free -h

# 增加 swap
sudo dphys-swapfile swapoff
sudo nano /etc/dphys-swapfile
# 修改: CONF_SWAPSIZE=1024
sudo dphys-swapfile setup
sudo dphys-swapfile swapon

# 調整服務記憶體限制
sudo systemctl edit wifi_display.service
# 加入:
# [Service]
# MemoryMax=180M

sudo systemctl daemon-reload
sudo systemctl restart wifi_display
```

---

## 📊 系統資源監控

### 即時監控

```bash
# 使用 htop (需先安裝: sudo apt install htop)
htop
# 按 F4 搜尋 "server_with_webui"

# 監控記憶體
watch -n 1 'free -h'

# 監控 CPU 溫度
watch -n 1 'vcgencmd measure_temp'
```

### 效能統計

```bash
# 檢查服務記憶體使用
ps aux | grep server_with_webui.py | grep -v grep

# 檢查磁碟空間
df -h

# 檢查網路連線
netstat -tn | grep -E '8266|8080'
```

---

## 🔄 完整重新部署

如果需要完全重新開始:

```bash
# 1. 停止並移除服務
sudo systemctl stop wifi_display
sudo systemctl disable wifi_display
sudo rm /etc/systemd/system/wifi_display.service
sudo systemctl daemon-reload

# 2. 刪除舊的虛擬環境和暫存檔
cd ~/wifi_spi_display/server
rm -rf venv __pycache__ logs/* uploads/*

# 3. 重新執行部署
bash deploy_rpi.sh
bash install_service.sh
```

---

## 📚 相關文件

- [完整部署指南](./docs/RASPBERRY_PI_DEPLOYMENT.md) - 詳細的部署說明和最佳化建議
- [專案 README](../README.md) - 專案功能說明
- [ESP8266 客戶端](../client_esp8266/) - ESP8266 端的程式碼

---

## 🆘 取得協助

如果遇到問題:

1. 查看日誌: `sudo journalctl -u wifi_display -n 100`
2. 檢查系統資源: `htop` 或 `free -h`
3. 參考完整部署指南的故障排除章節
4. 提交 Issue 到 GitHub

---

**最後更新**: 2025-10-15
**維護者**: WiFi SPI Display 專案團隊
