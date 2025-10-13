# Raspberry Pi 1 Model B 部署指南

本指南提供在 **Raspberry Pi 1 Model B** 上部署 WiFi SPI Display Web UI Server 的完整流程。

---

## 📋 目錄

1. [硬體規格與限制](#硬體規格與限制)
2. [開發任務清單](#開發任務清單)
3. [作業系統準備](#作業系統準備)
4. [軟體安裝](#軟體安裝)
5. [程式碼優化](#程式碼優化)
6. [系統服務配置](#系統服務配置)
7. [網路與安全](#網路與安全)
8. [測試與驗證](#測試與驗證)
9. [維護與監控](#維護與監控)
10. [故障排除](#故障排除)

---

## 🔧 硬體規格與限制

### Raspberry Pi 1 Model B 規格

| 項目 | 規格 | 影響 |
|------|------|------|
| **CPU** | ARM1176JZF-S (ARMv6) 700MHz 單核心 | ⚠️ 運算能力有限，影響圖像處理速度 |
| **記憶體** | 512MB SDRAM | ⚠️ **關鍵限制**，需優化記憶體使用 |
| **SD 卡 I/O** | Class 4-10 (2-10 MB/s) | ⚠️ 影響日誌寫入與檔案讀取 |
| **網路** | 10/100 Mbps 乙太網路 | ✅ 內建，足夠 WebSocket 傳輸 |
| **WiFi** | ❌ **無內建 WiFi** | ℹ️ 本指南使用有線網路部署 |
| **USB** | 2x USB 2.0 | ✅ 可擴充外接裝置 |
| **功耗** | ~3.5W (700mA @ 5V) | ✅ 低功耗，適合長期運行 |

### 關鍵限制分析

#### 0. 網路連接方式

**本指南使用有線網路部署**:
- ✅ 內建 10/100 Mbps 乙太網路埠
- ✅ 最穩定可靠的連接方式
- ✅ 不佔用 USB 埠和額外記憶體
- ✅ 無需額外配置，直接連接網路線即可
- 📌 基於 512MB RAM 限制，有線網路是最佳選擇

**注意**: Raspberry Pi 1 Model B 無內建 WiFi。如需無線連接，請參考外部資源購買並設定相容的 USB WiFi 適配器（本指南不涵蓋無線網路設定）。

#### 1. 記憶體限制 (512MB)
- **系統佔用**: ~150-200MB
- **可用記憶體**: ~300-350MB
- **影響**:
  - 圖像處理時需控制緩衝區大小
  - 不能同時處理多個大圖
  - 需減少 numpy 陣列分配
  - Python GC 需主動觸發

**解決方案**:
- 使用串流處理而非一次載入整張圖
- 限制同時連線客戶端數量 (建議 ≤ 2)
- 新增低記憶體模式配置
- 優化 Pillow/numpy 使用

#### 2. CPU 限制 (ARMv6 單核心)
- **影響**:
  - 圖像轉換速度慢 (800x480 約需 2-3 秒)
  - Floyd-Steinberg 抖動演算法耗時
  - WebSocket 與 HTTP 併發處理能力有限

**解決方案**:
- 預設關閉抖動演算法
- 使用更簡單的壓縮演算法
- 限制上傳檔案大小 (≤ 5MB)
- 考慮使用 PyPy (若 ARMv6 支援)

#### 3. Python 版本相容性

**⚠️ 關鍵限制: 必須使用 Python 3.9 (Bullseye)**

| Python 版本 | Raspberry Pi OS | ARMv6 套件支援 | 狀態 |
|------------|----------------|---------------|------|
| **3.9.x** | **Bullseye (Legacy)** | ✅ **完整支援** | ✅ **使用此版本** |
| 3.11.x | Bookworm (Legacy) | ⚠️ 部分套件缺失 | ⚠️ 不推薦 |
| 3.13.x | Trixie (最新) | ❌ 編譯失敗 | ❌ **無法使用** |

**為什麼 Python 3.13 會失敗？**
- numpy 1.24.4: setup.py 不相容 Python 3.13
- Pillow 10.0.0: C 擴充編譯失敗
- aiohttp 3.8.5: 部分依賴套件無 ARMv6 wheel
- 大多數套件尚未完全支援 Python 3.13

**套件相容性 (Python 3.9)**:
- ✅ Pillow: 需編譯原生擴充 (libjpeg, zlib)
- ✅ numpy: ARMv6 使用 piwheels 預編譯版本
- ✅ aiohttp: 完整支援 Python 3.9+
- ✅ websockets: 完整支援 Python 3.8+

---

## ✅ 開發任務清單

### 階段 1: 環境準備 (預估 2-3 小時)
- [ ] **1.1** 下載並燒錄 Raspberry Pi OS Lite (32-bit Legacy) **Bullseye 版本**
- [ ] **1.2** 配置 SD 卡 (啟用 SSH)
- [ ] **1.3** 首次開機與系統更新
- [ ] **1.4** ⚠️ **驗證 Python 3.9 與 Debian Bullseye** (關鍵步驟)
- [ ] **1.5** 配置 swap 空間 (擴增至 1GB)
- [ ] **1.6** 安裝系統工具 (vim, htop, git)

### 階段 2: Python 環境設定 (預估 1-2 小時)
- [ ] **2.1** ⚠️ **再次確認 Python 3.9.x** (避免錯誤版本)
- [ ] **2.2** 安裝 Python 3.9 開發套件與 pip
- [ ] **2.3** 安裝圖像處理系統套件
  - libjpeg-dev, zlib1g-dev, libfreetype6-dev
  - libopenjp2-7, libtiff5-dev
- [ ] **2.4** 建立 Python 虛擬環境
- [ ] **2.5** 安裝 Python 套件 (使用 piwheels)

### 階段 3: 程式碼調整 (預估 3-4 小時)
- [ ] **3.1** 新增低記憶體模式配置
- [ ] **3.2** 優化 `server_with_webui.py`
  - 減少緩衝區大小
  - 新增記憶體監控
  - 限制併發連線
- [ ] **3.3** 優化 `image_processor.py`
  - 串流處理
  - 減少 numpy 陣列分配
  - 預設關閉抖動
- [ ] **3.4** 調整 HTTP 上傳限制 (5MB)

### 階段 4: 服務配置 (預估 1-2 小時)
- [ ] **4.1** 建立 systemd 服務單元
- [ ] **4.2** 配置日誌輪替 (logrotate)
- [ ] **4.3** 設定自動啟動與重啟策略
- [ ] **4.4** 建立服務管理腳本

### 階段 5: 網路與安全 (預估 1-2 小時)
- [ ] **5.1** 設定靜態 IP 或 DHCP 保留
- [ ] **5.2** 安裝並配置 Avahi (mDNS 服務探索)
- [ ] **5.3** 配置防火牆 (ufw)
- [ ] **5.4** SSH 金鑰認證與安全加固
- [ ] **5.5** (選用) Nginx 反向代理與 HTTPS

### 階段 6: 測試與驗證 (預估 2-3 小時)
- [ ] **6.1** 功能測試
  - 服務啟動/停止
  - Web UI 存取
  - 圖片上傳與顯示
- [ ] **6.2** 效能測試
  - 記憶體使用監控
  - CPU 負載測試
  - 多客戶端併發測試
- [ ] **6.3** 穩定性測試
  - 24 小時運行測試
  - 記憶體洩漏檢測
- [ ] **6.4** ESP8266 客戶端整合測試

### 階段 7: 文件與自動化 (預估 1-2 小時)
- [ ] **7.1** 撰寫部署文件
- [ ] **7.2** 建立部署腳本 (deploy.sh)
- [ ] **7.3** 建立備份與復原流程
- [ ] **7.4** 撰寫維護手冊

---

## 🖥️ 作業系統準備

### 1. 下載作業系統映像

使用 **Raspberry Pi OS Lite (32-bit, Legacy) - Bullseye**:

**⚠️ 重要: 官方映像下載連結 (直接下載 Bullseye)**

```bash
# 方法 1: 直接下載穩定的 Bullseye 版本 (推薦)
# 前往此目錄瀏覽所有 Bullseye 版本:
https://downloads.raspberrypi.com/raspios_oldstable_lite_armhf/images/

# 推薦下載 (經測試穩定):
https://downloads.raspberrypi.com/raspios_oldstable_lite_armhf/images/raspios_oldstable_lite_armhf-2024-03-15/2024-03-15-raspios-bullseye-armhf-lite.img.xz

# 方法 2: 從官方頁面選擇 (可能只有新版本)
https://www.raspberrypi.com/software/operating-systems/
# 向下捲動找 "Raspberry Pi OS (Legacy)" 分類
```

**如何驗證下載的映像檔？**
```bash
# 檔名格式: YYYY-MM-DD-raspios-CODENAME-armhf-lite.img.xz
# 
# ✅ 正確範例:
#    2024-03-15-raspios-bullseye-armhf-lite.img.xz
#    2023-10-10-raspios-bullseye-armhf-lite.img.xz
#
# ❌ 錯誤範例 (不要下載):
#    2024-10-22-raspios-bookworm-armhf-lite.img.xz  (Python 3.11)
#    2025-xx-xx-raspios-trixie-armhf-lite.img.xz    (Python 3.13)
```

**⚠️ 版本選擇警告**:

| 版本 | Debian | Python | ARMv6 支援 | 套件相容性 | 推薦 |
|------|--------|--------|-----------|----------|------|
| **Bullseye (Legacy)** | 11 | 3.9.x | ✅ 完整支援 | ✅ 所有套件可用 | ✅ **強烈推薦** |
| Bookworm (Legacy) | 12 | 3.11.x | ✅ 支援 | ⚠️ 部分套件問題 | ⚠️ 可能遇到問題 |
| Trixie (最新) | 13 | 3.13.x | ❌ 編譯失敗 | ❌ 多數套件不相容 | ❌ **請勿使用** |

**為何必須選擇 Bullseye Legacy 版本？**
- ✅ Raspberry Pi 1 使用 ARMv6 架構，需要 Legacy 版本
- ✅ Python 3.9 與所有套件完全相容
- ✅ Piwheels 提供完整的 ARMv6 預編譯套件
- ✅ 經過充分測試，穩定性最高
- ❌ **切勿使用 Trixie (Python 3.13)**: numpy, Pillow 等套件無法編譯
- ❌ 新版 Raspberry Pi OS 僅支援 ARMv7+ (Pi 2 以上)

### 2. 燒錄 SD 卡

**使用 Raspberry Pi Imager (推薦)**:

1. 下載並安裝 [Raspberry Pi Imager](https://www.raspberrypi.com/software/)
2. **⚠️ 重要**: 選擇作業系統步驟:
   
   **方法 1: 使用 Raspberry Pi Imager (如果可選擇舊版本)**
   - 點選 `Raspberry Pi OS (other)`
   - 選擇 `Raspberry Pi OS Lite (Legacy, 32-bit)`
   - ⚠️ **注意**: 某些版本的 Imager 只提供最新版本（Trixie）
   - 如果看不到版本說明或只有一個 Legacy 選項，請改用方法 2
   
   **方法 2: 手動下載映像檔 (推薦，更可控)**
   - 前往: https://downloads.raspberrypi.com/raspios_oldstable_lite_armhf/images/
   - 選擇日期資料夾: `raspios_oldstable_lite_armhf-2024-03-15/` (或類似)
   - 下載: `2024-03-15-raspios-bullseye-armhf-lite.img.xz`
   - 在 Raspberry Pi Imager 中選擇 `Use custom` (使用自訂映像)
   - 選擇剛下載的 `.img.xz` 檔案
   
   **如何確認是 Bullseye 版本？**
   - 檔名包含 `bullseye` (正確 ✅)
   - 檔名包含 `bookworm` (錯誤 ❌)
   - 檔名包含 `trixie` (錯誤 ❌)
   - 日期在 2023-2024 年的通常是 Bullseye

3. 選擇 SD 卡 (建議 ≥ 16GB, Class 10)
4. 進階設定 (齒輪圖示):
   ```
   啟用 SSH: ✅ 使用密碼認證
   設定使用者名稱: pi
   設定密碼: (自訂強密碼)
   設定語系: Asia/Taipei
   鍵盤配置: us
   ```

5. 燒錄 (約需 5-10 分鐘)

**使用 Raspberry Pi Imager 燒錄自訂映像**:

在 Imager 中選擇剛下載的映像檔:
1. 點選「選擇作業系統」→ 捲動到最下方
2. 選擇 `Use custom` (使用自訂映像)
3. 選擇剛下載的 `2024-03-15-raspios-bullseye-armhf-lite.img.xz`
4. 選擇 SD 卡
5. 進階設定（齒輪圖示）→ 設定 SSH、使用者名稱、密碼
6. 開始燒錄

**手動配置 (進階)**:

如果需要手動啟用 SSH，在 SD 卡燒錄後:

```bash
# 燒錄後，將 SD 卡重新插入電腦
# 在 boot 分割區建立檔案:

# 啟用 SSH
touch /Volumes/boot/ssh  # macOS/Linux
# 或
type nul > F:\ssh        # Windows
```

### 3. 首次開機與基本設定

```bash
# 1. 插入 SD 卡並啟動 Raspberry Pi
# 2. 連接網路線到路由器

# 3. 等待約 1-2 分鐘後，尋找 IP 位址
# 方法 A: 檢查路由器 DHCP 列表
# 方法 B: 使用網路掃描工具
nmap -sn 192.168.1.0/24 | grep -B 2 "Raspberry"

# 4. SSH 連線
ssh pi@<RPi_IP_Address>
# 預設密碼: raspberry (若未在 Imager 中自訂)

# 5. 驗證作業系統版本 (重要！)
cat /etc/os-release
# 應顯示:
# VERSION_CODENAME=bullseye
# VERSION="11 (bullseye)"

python3 --version
# 應顯示: Python 3.9.x

# ⚠️ 如果看到 Python 3.11 或 3.13，請停止並重新燒錄 SD 卡
# 使用正確的 Bullseye Legacy 版本

# 6. 更新系統 (首次執行約需 10-20 分鐘)
sudo apt update
sudo apt upgrade -y

# 7. 系統配置工具
sudo raspi-config
```

**raspi-config 重要設定**:

```
1 System Options
  → S4 Hostname: 改為 epaper-server
  → S6 Network at Boot: 啟用等待網路

3 Interface Options
  → (確認 SSH 已啟用)

4 Performance Options
  → P2 GPU Memory: 設為 16 (最小值，釋放記憶體給系統)

5 Localisation Options
  → L1 Locale: 選擇 en_US.UTF-8 和 zh_TW.UTF-8
  → L2 Timezone: Asia/Taipei

6 Advanced Options
  → A1 Expand Filesystem: 擴展至 SD 卡全部容量

完成後選擇 Finish 並重新啟動
```

**首次開機後檢查網路連線**:

```bash
# 檢查有線網路介面 (eth0)
ip addr show eth0
# 應顯示: inet 192.168.1.x/24

# 測試網路連通性
ping -c 4 8.8.8.8
ping -c 4 google.com
```

### 4. 配置 Swap 空間

Raspberry Pi 1 僅有 512MB RAM，需增加 swap。根據系統版本選擇對應方法：

**方法 A: 使用 dphys-swapfile (傳統方法)**

```bash
# 檢查是否已安裝
which dphys-swapfile

# 如果已安裝，執行以下步驟:
# 停用預設 swap
sudo dphys-swapfile swapoff
sudo systemctl stop dphys-swapfile

# 修改 swap 大小為 1GB
sudo nano /etc/dphys-swapfile
# 修改以下行:
CONF_SWAPSIZE=1024

# 重新建立 swap
sudo dphys-swapfile setup
sudo dphys-swapfile swapon
sudo systemctl start dphys-swapfile
```

**方法 B: 手動建立 Swap 檔案 (若無 dphys-swapfile)**

```bash
# 1. 檢查目前 swap 狀態
free -h
sudo swapon --show

# 2. 關閉現有 swap (如果有)
sudo swapoff -a

# 3. 建立 1GB swap 檔案
sudo fallocate -l 1G /swapfile
# 如果 fallocate 失敗，使用 dd:
# sudo dd if=/dev/zero of=/swapfile bs=1M count=1024 status=progress

# 4. 設定正確權限 (安全性考量)
sudo chmod 600 /swapfile

# 5. 格式化為 swap
sudo mkswap /swapfile

# 6. 啟用 swap
sudo swapon /swapfile

# 7. 驗證
free -h
# 應顯示 Swap: 1.0Gi

# 8. 設定開機自動掛載
echo '/swapfile none swap sw 0 0' | sudo tee -a /etc/fstab

# 9. 確認 fstab 內容
cat /etc/fstab | grep swap
```

**驗證 Swap 配置**:

```bash
# 檢查 swap 狀態
free -h
sudo swapon --show

# 應顯示類似:
# NAME      TYPE SIZE USED PRIO
# /swapfile file   1G   0B   -2
```

⚠️ **注意**: Swap 會增加 SD 卡寫入，影響壽命。建議:
- 使用高耐久度 SD 卡 (如 SanDisk High Endurance)
- 定期備份系統
- 調整 swappiness 降低 swap 使用頻率:

```bash
# 降低 swap 使用傾向 (預設 60)
sudo sysctl vm.swappiness=10

# 檢查目前設定
cat /proc/sys/vm/swappiness

# 永久生效
echo "vm.swappiness=10" | sudo tee -a /etc/sysctl.conf
```

### 5. 安裝基本工具

```bash
# 開發與監控工具
sudo apt install -y \
  vim \
  htop \
  iotop \
  git \
  curl \
  rsync \
  screen \
  tmux

# 系統資訊 (選用)
sudo apt install -y \
  lsb-release

# 顯示系統資訊 (若需要)
cat /etc/os-release
uname -a
```

---

## 📦 軟體安裝

### 1. Python 環境準備

```bash
# ⚠️ 再次確認 Python 版本 (關鍵步驟！)
python3 --version
# 必須顯示: Python 3.9.x

cat /etc/os-release | grep VERSION_CODENAME
# 必須顯示: VERSION_CODENAME=bullseye

# ❌ 如果顯示 Python 3.11.x 或 3.13.x，請停止部署！
# ❌ 如果顯示 bookworm 或 trixie，請重新燒錄 SD 卡！

# 安裝 Python 開發套件
sudo apt install -y \
  python3-dev \
  python3-pip \
  python3-venv \
  python3-full

# 注意: 新版 Raspberry Pi OS 使用 PEP 668 保護機制
# 不允許直接在系統 Python 安裝套件
# 我們將在虛擬環境中安裝所有套件
```

### 2. 圖像處理系統套件

Pillow 需要以下原生函式庫:

```bash
sudo apt install -y \
  libjpeg-dev \
  zlib1g-dev \
  libfreetype6-dev \
  liblcms2-dev \
  libopenjp2-7-dev \
  libtiff5-dev \
  libwebp-dev \
  tcl8.6-dev \
  tk8.6-dev \
  python3-tk

# 確認安裝
dpkg -l | grep -E 'libjpeg|zlib|libfreetype'
```

### 3. 建立專案目錄與虛擬環境

```bash
# 建立專案目錄
mkdir -p ~/wifi_spi_display
cd ~/wifi_spi_display

# 下載專案 (若已有 git repo)
git clone <你的 repo URL> .
# 或手動上傳檔案 (使用 scp/rsync)

# 建立虛擬環境
python3 -m venv venv

# 啟用虛擬環境
source venv/bin/activate

# 升級 pip (在虛擬環境中)
pip install --upgrade pip

# 驗證
which python
# 應顯示: /home/pi/wifi_spi_display/venv/bin/python

python --version
pip --version
```

### 4. 安裝 Python 套件

**建立 RPi 相容的 requirements.txt**:

```bash
# 建立 requirements_rpi.txt
cat > requirements_rpi.txt << 'EOF'
# RPi 1 Model B (ARMv6) 相容版本
# 使用 piwheels 預編譯套件加速安裝

# Web Server
aiohttp==3.8.5
websockets==11.0.3

# 圖像處理
Pillow==10.0.0
numpy==1.24.4

# 系統工具
psutil==5.9.5
EOF
```

**安裝套件**:

```bash
# 確保使用 piwheels (ARMv6 預編譯套件)
pip install --index-url https://www.piwheels.org/simple \
  --extra-index-url https://pypi.org/simple \
  -r requirements_rpi.txt

# 安裝時間可能較長 (Pillow 約 5-10 分鐘)
# 若遇到錯誤，檢查系統套件是否完整安裝
```

**驗證安裝**:

```bash
python << 'EOF'
import sys
print(f"Python: {sys.version}")

import aiohttp
print(f"aiohttp: {aiohttp.__version__}")

import websockets
print(f"websockets: {websockets.__version__}")

from PIL import Image
print(f"Pillow: {Image.__version__}")

import numpy as np
print(f"numpy: {np.__version__}")

import psutil
print(f"psutil: {psutil.__version__}")

print("\n✅ 所有套件安裝成功！")
EOF
```

### 5. 建立自動安裝腳本

```bash
cat > install_rpi.sh << 'SCRIPT'
#!/bin/bash
# WiFi SPI Display Server - Raspberry Pi 1 安裝腳本

set -e  # 遇到錯誤立即停止

echo "====================================="
echo " WiFi SPI Display Server"
echo " Raspberry Pi 1 Model B 安裝"
echo "====================================="
echo ""

# 檢查是否為 root
if [ "$EUID" -eq 0 ]; then
  echo "❌ 請勿使用 root 執行此腳本"
  exit 1
fi

# 1. 系統套件更新
echo "📦 更新系統套件..."
sudo apt update

# 2. 安裝 Python 與開發工具
echo "🐍 安裝 Python 環境..."
sudo apt install -y \
  python3-dev \
  python3-pip \
  python3-venv \
  git \
  vim

# 3. 安裝圖像處理系統套件
echo "🖼️  安裝圖像處理函式庫..."
sudo apt install -y \
  libjpeg-dev \
  zlib1g-dev \
  libfreetype6-dev \
  libopenjp2-7-dev \
  libtiff5-dev

# 4. 建立虛擬環境
echo "📁 建立 Python 虛擬環境..."
if [ ! -d "venv" ]; then
  python3 -m venv venv
fi

# 5. 安裝 Python 套件
echo "📚 安裝 Python 套件..."
source venv/bin/activate
pip install --upgrade pip
pip install --index-url https://www.piwheels.org/simple \
  --extra-index-url https://pypi.org/simple \
  -r requirements_rpi.txt

# 6. 建立必要目錄
echo "📂 建立目錄結構..."
mkdir -p logs
mkdir -p uploads

# 7. 測試安裝
echo "🧪 測試安裝..."
python << 'EOF'
import aiohttp, websockets, PIL, numpy
print("✅ 所有套件載入成功")
EOF

echo ""
echo "====================================="
echo " ✅ 安裝完成！"
echo "====================================="
echo ""
echo "下一步:"
echo "1. 啟動伺服器: source venv/bin/activate && python server/server_with_webui.py"
echo "2. 設定系統服務: sudo cp wifi_display.service /etc/systemd/system/"
echo "3. 啟用服務: sudo systemctl enable wifi_display"
echo ""
SCRIPT

chmod +x install_rpi.sh
```

---

## 🔧 程式碼優化

### 1. 新增低記憶體模式配置

建立 `server/config_rpi.py`:

```python
"""
Raspberry Pi 1 Model B 專用配置
針對 512MB RAM 最佳化
"""

# 記憶體限制 (bytes)
MAX_MEMORY_MB = 200  # 最大使用 200MB
LOW_MEMORY_THRESHOLD = 100  # 低於 100MB 可用時警告

# 圖像處理
IMAGE_MAX_SIZE = 5 * 1024 * 1024  # 上傳檔案限制 5MB
IMAGE_CACHE_SIZE = 2  # 僅快取最近 2 張圖片
USE_DITHER = False  # 關閉抖動演算法 (節省 CPU/記憶體)
TILE_HEIGHT = 30  # 條帶高度 (降低記憶體使用)

# WebSocket
MAX_CLIENTS = 2  # 最多同時 2 個客戶端連線
WS_MAX_SIZE = 2 * 1024 * 1024  # WebSocket 訊息限制 2MB
WS_PING_INTERVAL = 30  # 增加 ping 間隔

# HTTP
HTTP_MAX_UPLOAD = 5 * 1024 * 1024  # HTTP 上傳限制 5MB
HTTP_TIMEOUT = 60  # 增加超時時間

# 日誌
LOG_LEVEL = "INFO"  # 降低日誌輸出
LOG_MAX_SIZE = 10 * 1024 * 1024  # 日誌檔案最大 10MB
LOG_BACKUP_COUNT = 3  # 僅保留 3 個備份

# 效能監控
ENABLE_MEMORY_MONITOR = True  # 啟用記憶體監控
MEMORY_CHECK_INTERVAL = 10  # 每 10 秒檢查一次
GC_COLLECT_INTERVAL = 30  # 每 30 秒強制 GC
```

### 2. 修改 `server_with_webui.py`

新增記憶體監控與低記憶體模式:

```python
# 在檔案開頭新增
import gc
import psutil
from config_rpi import (
    MAX_MEMORY_MB, LOW_MEMORY_THRESHOLD, 
    IMAGE_MAX_SIZE, MAX_CLIENTS,
    ENABLE_MEMORY_MONITOR, GC_COLLECT_INTERVAL
)

class DisplayServer:
    def __init__(self, host: str = "0.0.0.0", port: int = 8266, http_port: int = 8080):
        # ... 現有程式碼 ...
        
        # 記憶體監控
        self.memory_monitor_enabled = ENABLE_MEMORY_MONITOR
        self.last_gc_time = time.time()
        
        # 修改上傳限制
        self.http_app = web.Application(client_max_size=IMAGE_MAX_SIZE)
        
        # 啟動記憶體監控
        if self.memory_monitor_enabled:
            asyncio.create_task(self._memory_monitor())
    
    async def _memory_monitor(self):
        """記憶體監控協程"""
        while True:
            try:
                # 取得記憶體資訊
                mem = psutil.virtual_memory()
                mem_used_mb = mem.used / 1024 / 1024
                mem_avail_mb = mem.available / 1024 / 1024
                
                # 記錄狀態
                if mem_avail_mb < LOW_MEMORY_THRESHOLD:
                    logger.warning(
                        f"⚠️  記憶體不足！可用: {mem_avail_mb:.1f}MB / "
                        f"已用: {mem_used_mb:.1f}MB ({mem.percent}%)"
                    )
                    # 強制垃圾回收
                    gc.collect()
                
                # 定期 GC
                if time.time() - self.last_gc_time > GC_COLLECT_INTERVAL:
                    collected = gc.collect()
                    self.last_gc_time = time.time()
                    logger.debug(f"🗑️  GC 回收: {collected} 個物件")
                
                # 每 10 秒檢查一次
                await asyncio.sleep(10)
                
            except Exception as e:
                logger.error(f"記憶體監控錯誤: {e}")
                await asyncio.sleep(60)
    
    async def handle_upload(self, request):
        """處理圖片上傳 (加入記憶體檢查)"""
        try:
            # 檢查可用記憶體
            mem = psutil.virtual_memory()
            if mem.available < 50 * 1024 * 1024:  # < 50MB
                return web.json_response({
                    'success': False,
                    'message': '記憶體不足，請稍後再試'
                }, status=503)
            
            # 原有上傳處理邏輯...
            # ...
            
        finally:
            # 上傳完成後立即 GC
            gc.collect()
```

### 3. 優化 `image_processor.py`

修改為串流處理模式:

```python
class ImageProcessor:
    def __init__(self, width: int = 400, height: int = 240, low_memory_mode: bool = True):
        """
        初始化圖像處理器
        
        Args:
            width: 顯示器寬度
            height: 顯示器高度
            low_memory_mode: 低記憶體模式 (Raspberry Pi 1)
        """
        self.width = width
        self.height = height
        self.low_memory_mode = low_memory_mode
        
    def convert_to_1bit(self, image: Image.Image, dither: bool = False) -> Image.Image:
        """
        將圖片轉換為 1-bit (低記憶體版本)
        
        Args:
            image: 輸入圖片
            dither: 是否使用抖動 (RPi1 預設關閉)
        """
        # 低記憶體模式: 使用更保守的記憶體策略
        if self.low_memory_mode:
            # 分段縮放 (避免大圖一次載入記憶體)
            max_intermediate_size = 2000
            if image.width > max_intermediate_size or image.height > max_intermediate_size:
                # 先縮小到中間尺寸
                scale = max_intermediate_size / max(image.width, image.height)
                intermediate_size = (
                    int(image.width * scale),
                    int(image.height * scale)
                )
                image = image.resize(intermediate_size, Image.Resampling.BILINEAR)
                del scale, intermediate_size
                gc.collect()
        
        # 縮放到目標尺寸
        image = image.resize((self.width, self.height), Image.Resampling.LANCZOS)
        
        # 轉換為灰階
        image = image.convert('L')
        
        # 轉換為 1-bit (預設不使用抖動)
        image = image.convert('1', dither=Image.Dither.FLOYDSTEINBERG if dither else Image.Dither.NONE)
        
        return image
    
    def image_to_bytes(self, image: Image.Image) -> bytes:
        """
        將 1-bit 圖片轉換為 bytes (最佳化版本)
        """
        if image.mode != '1':
            image = image.convert('1')
        
        # 直接使用 PIL 的 tobytes() 而非 numpy (節省記憶體)
        if self.low_memory_mode:
            return image.tobytes()
        else:
            # 原有 numpy 方式 (較快但耗記憶體)
            pixels = np.array(image, dtype=np.uint8)
            pixels = (pixels > 0).astype(np.uint8)
            packed = np.packbits(pixels, axis=1)
            return packed.tobytes()
```

---

## ⚙️ 系統服務配置

### 1. 建立 Systemd 服務單元

```bash
# 建立服務檔案
sudo nano /etc/systemd/system/wifi_display.service
```

貼上以下內容:

```ini
[Unit]
Description=WiFi SPI Display Server (E-Paper WebUI)
Documentation=https://github.com/<your-repo>
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=pi
Group=pi
WorkingDirectory=/home/pi/wifi_spi_display/server
Environment="PATH=/home/pi/wifi_spi_display/venv/bin"
ExecStart=/home/pi/wifi_spi_display/venv/bin/python server_with_webui.py

# 重啟策略
Restart=always
RestartSec=10
StartLimitInterval=200
StartLimitBurst=5

# 資源限制 (重要！)
MemoryMax=250M
MemoryHigh=200M
CPUQuota=90%

# 日誌
StandardOutput=journal
StandardError=journal
SyslogIdentifier=wifi_display

# 安全性
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=read-only
ReadWritePaths=/home/pi/wifi_spi_display/server/logs
ReadWritePaths=/home/pi/wifi_spi_display/server/uploads

[Install]
WantedBy=multi-user.target
```

### 2. 啟用並啟動服務

```bash
# 重新載入 systemd
sudo systemctl daemon-reload

# 啟用服務 (開機自動啟動)
sudo systemctl enable wifi_display.service

# 啟動服務
sudo systemctl start wifi_display.service

# 檢查狀態
sudo systemctl status wifi_display.service

# 查看日誌
sudo journalctl -u wifi_display.service -f
```

### 3. 配置日誌輪替

```bash
# 建立 logrotate 配置
sudo nano /etc/logrotate.d/wifi_display
```

貼上:

```
/home/pi/wifi_spi_display/server/logs/*.log {
    daily
    rotate 7
    compress
    delaycompress
    missingok
    notifempty
    create 0640 pi pi
    sharedscripts
    postrotate
        systemctl reload wifi_display >/dev/null 2>&1 || true
    endscript
}
```

### 4. 服務管理腳本

```bash
cat > ~/manage_server.sh << 'SCRIPT'
#!/bin/bash
# WiFi Display Server 管理腳本

SERVICE="wifi_display.service"

case "$1" in
  start)
    sudo systemctl start $SERVICE
    echo "✅ 服務已啟動"
    ;;
  stop)
    sudo systemctl stop $SERVICE
    echo "⏹️  服務已停止"
    ;;
  restart)
    sudo systemctl restart $SERVICE
    echo "🔄 服務已重啟"
    ;;
  status)
    sudo systemctl status $SERVICE
    ;;
  logs)
    sudo journalctl -u $SERVICE -f
    ;;
  enable)
    sudo systemctl enable $SERVICE
    echo "✅ 已設定開機自動啟動"
    ;;
  disable)
    sudo systemctl disable $SERVICE
    echo "⏹️  已取消開機自動啟動"
    ;;
  *)
    echo "用法: $0 {start|stop|restart|status|logs|enable|disable}"
    exit 1
    ;;
esac
SCRIPT

chmod +x ~/manage_server.sh
```

---

## 🌐 網路與安全

### 1. 設定靜態 IP

編輯 dhcpcd 配置:

```bash
sudo nano /etc/dhcpcd.conf
```

在檔案末尾加入:

```
# 靜態 IP 設定 (有線網路)
interface eth0
static ip_address=192.168.1.100/24
static routers=192.168.1.1
static domain_name_servers=192.168.1.1 8.8.8.8
```

套用設定:

```bash
sudo systemctl restart dhcpcd
```

### 2. 安裝並配置 Avahi (mDNS)

讓 ESP8266 透過 `epaper-server.local` 發現伺服器:

```bash
# 安裝 Avahi
sudo apt install -y avahi-daemon avahi-utils

# 啟用服務
sudo systemctl enable avahi-daemon
sudo systemctl start avahi-daemon

# 測試
avahi-browse -a
```

設定主機名稱:

```bash
sudo hostnamectl set-hostname epaper-server

# 驗證
hostname
# 應顯示: epaper-server

# 從其他裝置測試
ping epaper-server.local
```

### 3. 配置防火牆 (UFW)

```bash
# 安裝 ufw
sudo apt install -y ufw

# 預設規則
sudo ufw default deny incoming
sudo ufw default allow outgoing

# 允許 SSH
sudo ufw allow 22/tcp comment 'SSH'

# 允許伺服器埠號
sudo ufw allow 8266/tcp comment 'WebSocket'
sudo ufw allow 8080/tcp comment 'Web UI'

# 允許 mDNS
sudo ufw allow 5353/udp comment 'mDNS/Avahi'

# 啟用防火牆
sudo ufw enable

# 檢查狀態
sudo ufw status verbose
```

### 4. SSH 安全加固

```bash
# 修改 SSH 配置
sudo nano /etc/ssh/sshd_config
```

建議修改:

```
# 禁用 root 登入
PermitRootLogin no

# 僅允許金鑰認證 (設定好金鑰後再啟用)
# PasswordAuthentication no

# 限制登入嘗試
MaxAuthTries 3

# 閒置超時
ClientAliveInterval 300
ClientAliveCountMax 2
```

設定 SSH 金鑰認證:

```bash
# 在本機電腦產生金鑰 (若尚未有)
ssh-keygen -t ed25519 -C "your_email@example.com"

# 複製公鑰到 RPi
ssh-copy-id pi@epaper-server.local

# 測試金鑰登入
ssh pi@epaper-server.local

# 確認可正常登入後，禁用密碼認證
sudo nano /etc/ssh/sshd_config
# 取消註解並設為 no:
PasswordAuthentication no

# 重啟 SSH
sudo systemctl restart ssh
```

### 5. (選用) Nginx 反向代理

若需 HTTPS 或更好的效能:

```bash
# 安裝 Nginx
sudo apt install -y nginx

# 建立網站配置
sudo nano /etc/nginx/sites-available/wifi_display
```

貼上:

```nginx
server {
    listen 80;
    server_name epaper-server.local;
    
    # Web UI
    location / {
        proxy_pass http://127.0.0.1:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        
        # WebSocket 支援
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        
        # 超時設定
        proxy_read_timeout 3600s;
        proxy_send_timeout 3600s;
    }
    
    # WebSocket 埠號
    location /ws {
        proxy_pass http://127.0.0.1:8266;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
    }
}
```

啟用網站:

```bash
sudo ln -s /etc/nginx/sites-available/wifi_display /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl restart nginx

# 防火牆允許 HTTP
sudo ufw allow 80/tcp comment 'HTTP'
```

---

## 🧪 測試與驗證

### 1. 功能測試清單

```bash
# 建立測試腳本
cat > ~/test_server.sh << 'SCRIPT'
#!/bin/bash
# WiFi Display Server 功能測試

echo "====================================="
echo " WiFi Display Server 功能測試"
echo "====================================="
echo ""

# 1. 服務狀態測試
echo "1️⃣  測試服務狀態..."
sudo systemctl is-active --quiet wifi_display.service && echo "✅ 服務運行中" || echo "❌ 服務未運行"

# 2. 埠號監聽測試
echo ""
echo "2️⃣  測試埠號監聽..."
netstat -tuln | grep -E '8266|8080' && echo "✅ 埠號正常監聽" || echo "❌ 埠號未監聽"

# 3. Web UI 存取測試
echo ""
echo "3️⃣  測試 Web UI 存取..."
curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/ | grep 200 && echo "✅ Web UI 可存取" || echo "❌ Web UI 無法存取"

# 4. 記憶體使用測試
echo ""
echo "4️⃣  檢查記憶體使用..."
MEMORY_USED=$(ps aux | grep server_with_webui.py | grep -v grep | awk '{print $6}')
if [ -n "$MEMORY_USED" ]; then
  MEMORY_MB=$((MEMORY_USED / 1024))
  echo "📊 伺服器使用記憶體: ${MEMORY_MB}MB"
  if [ $MEMORY_MB -lt 200 ]; then
    echo "✅ 記憶體使用正常"
  else
    echo "⚠️  記憶體使用偏高"
  fi
else
  echo "❌ 無法取得記憶體資訊"
fi

# 5. mDNS 測試
echo ""
echo "5️⃣  測試 mDNS 發現..."
avahi-resolve -n epaper-server.local && echo "✅ mDNS 正常" || echo "❌ mDNS 失敗"

echo ""
echo "====================================="
echo " 測試完成"
echo "====================================="
SCRIPT

chmod +x ~/test_server.sh
```

執行測試:

```bash
~/test_server.sh
```

### 2. 效能測試

**記憶體負載測試**:

```bash
# 監控記憶體使用 (持續)
watch -n 1 'free -h && ps aux | grep server_with_webui | grep -v grep'

# 使用 htop 觀察
htop
# 按 F4 搜尋 "server_with_webui"
```

**CPU 負載測試**:

```bash
# 上傳多張大圖測試
for i in {1..5}; do
  curl -F "file=@test_large.jpg" http://epaper-server.local:8080/upload
  sleep 2
done

# 觀察 CPU 使用
mpstat 1 10
```

### 3. 穩定性測試

**24 小時運行測試**:

```bash
# 建立監控腳本
cat > ~/monitor_stability.sh << 'SCRIPT'
#!/bin/bash
# 24 小時穩定性監控

LOG_FILE="stability_test_$(date +%Y%m%d_%H%M%S).log"

echo "開始 24 小時穩定性測試: $(date)" | tee -a $LOG_FILE

for i in {1..1440}; do  # 24 小時 = 1440 分鐘
  TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")
  
  # 檢查服務狀態
  if sudo systemctl is-active --quiet wifi_display.service; then
    STATUS="✅ RUNNING"
  else
    STATUS="❌ STOPPED"
  fi
  
  # 記憶體使用
  MEMORY=$(ps aux | grep server_with_webui.py | grep -v grep | awk '{print $6}')
  MEMORY_MB=$((MEMORY / 1024))
  
  # 系統可用記憶體
  AVAIL_MEM=$(free -m | awk '/^Mem:/{print $7}')
  
  # 記錄
  echo "$TIMESTAMP | $STATUS | Server Memory: ${MEMORY_MB}MB | Available: ${AVAIL_MEM}MB" | tee -a $LOG_FILE
  
  # 每分鐘檢查
  sleep 60
done

echo "測試結束: $(date)" | tee -a $LOG_FILE
SCRIPT

chmod +x ~/monitor_stability.sh

# 在 screen 中執行 (可中離)
screen -S stability_test
~/monitor_stability.sh
# 按 Ctrl+A 再按 D 離開 (測試繼續執行)

# 重新連接查看
screen -r stability_test
```

### 4. ESP8266 客戶端整合測試

在 ESP8266 客戶端程式中設定伺服器位址:

```cpp
// 方法 1: 使用 IP 位址
const char* SERVER_IP = "192.168.1.100";

// 方法 2: 使用 mDNS
const char* SERVER_HOSTNAME = "epaper-server.local";
```

測試流程:

1. ESP8266 上電並連接 WiFi
2. 嘗試連接伺服器
3. 上傳測試圖片
4. 觀察電子紙顯示
5. 檢查伺服器日誌:

```bash
sudo journalctl -u wifi_display.service -f
```

---

## 📊 維護與監控

### 1. 日常監控命令

```bash
# 系統資源
free -h           # 記憶體
df -h             # 磁碟空間
top               # 即時資源使用
htop              # 進階監控 (需安裝)

# 服務狀態
sudo systemctl status wifi_display.service  # 服務狀態
sudo journalctl -u wifi_display.service -n 50  # 最近 50 行日誌
sudo journalctl -u wifi_display.service --since "1 hour ago"  # 過去 1 小時日誌

# 網路
netstat -tuln | grep -E '8266|8080'  # 檢查埠號
ss -tulpn | grep python              # 查看 Python 監聽埠號

# 溫度 (RPi 1 較不會過熱，但仍可檢查)
vcgencmd measure_temp
```

### 2. 效能監控腳本

```bash
cat > ~/monitor_performance.sh << 'SCRIPT'
#!/bin/bash
# WiFi Display Server 效能監控

while true; do
  clear
  echo "====================================="
  echo " WiFi Display Server 效能監控"
  echo " $(date)"
  echo "====================================="
  echo ""
  
  # CPU 溫度
  TEMP=$(vcgencmd measure_temp | awk -F= '{print $2}')
  echo "🌡️  CPU 溫度: $TEMP"
  echo ""
  
  # 記憶體
  echo "💾 記憶體使用:"
  free -h
  echo ""
  
  # 伺服器程序
  echo "📊 伺服器程序:"
  ps aux | grep server_with_webui.py | grep -v grep | awk '{printf "  PID: %s | CPU: %s%% | MEM: %s%% | RSS: %d MB\n", $2, $3, $4, $6/1024}'
  echo ""
  
  # 網路連線
  echo "🌐 網路連線:"
  netstat -tn 2>/dev/null | grep -E '8266|8080' | wc -l | awk '{print "  活躍連線數: " $1}'
  echo ""
  
  # 磁碟使用
  echo "💿 磁碟使用:"
  df -h / | tail -1 | awk '{print "  已用: " $3 " / 總共: " $2 " (" $5 ")"}'
  echo ""
  
  echo "====================================="
  echo "按 Ctrl+C 結束監控"
  
  sleep 5
done
SCRIPT

chmod +x ~/monitor_performance.sh

# 執行
~/monitor_performance.sh
```

### 3. 自動備份腳本

```bash
cat > ~/backup_server.sh << 'SCRIPT'
#!/bin/bash
# WiFi Display Server 備份腳本

BACKUP_DIR="/home/pi/backups"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
BACKUP_FILE="wifi_display_backup_$TIMESTAMP.tar.gz"

echo "開始備份: $(date)"

# 建立備份目錄
mkdir -p $BACKUP_DIR

# 停止服務 (可選)
# sudo systemctl stop wifi_display.service

# 打包
tar -czf $BACKUP_DIR/$BACKUP_FILE \
  --exclude='venv' \
  --exclude='__pycache__' \
  --exclude='*.pyc' \
  --exclude='logs/*.log' \
  /home/pi/wifi_spi_display

# 重啟服務 (若有停止)
# sudo systemctl start wifi_display.service

# 刪除 7 天前的備份
find $BACKUP_DIR -name "wifi_display_backup_*.tar.gz" -mtime +7 -delete

echo "備份完成: $BACKUP_DIR/$BACKUP_FILE"
ls -lh $BACKUP_DIR/$BACKUP_FILE
SCRIPT

chmod +x ~/backup_server.sh

# 設定 cron 定期備份 (每週日凌晨 2 點)
crontab -e
# 加入:
# 0 2 * * 0 /home/pi/backup_server.sh >> /home/pi/backup.log 2>&1
```

### 4. 系統更新流程

```bash
# 定期更新系統 (每月)
sudo apt update
sudo apt upgrade -y
sudo apt autoremove -y
sudo apt autoclean

# 更新 Python 套件
source ~/wifi_spi_display/venv/bin/activate
pip list --outdated
pip install --upgrade pip
pip install --upgrade -r requirements_rpi.txt

# 重啟服務
sudo systemctl restart wifi_display.service
```

---

## 🔧 故障排除

### 常見問題與解決方案

#### 0. Python 版本錯誤 (最常見問題)

**症狀**: 安裝套件時出現編譯錯誤，如:
```
error: subprocess-exited-with-error
KeyError: '__version__'
ModuleNotFoundError: No module named 'numpy.core._multiarray_umath'
Building wheel for numpy (pyproject.toml): finished with status 'error'
```

**原因**: 使用了 Python 3.11+ 或 Python 3.13 (Bookworm/Trixie)

**解決方案**:

```bash
# 1. 確認目前版本
python3 --version
cat /etc/os-release | grep VERSION_CODENAME

# 如果顯示:
# VERSION_CODENAME=trixie   → Python 3.13 ❌ 錯誤
# VERSION_CODENAME=bookworm → Python 3.11 ⚠️ 不推薦
# VERSION_CODENAME=bullseye → Python 3.9  ✅ 正確

# 2. 如果不是 bullseye，必須重新燒錄 SD 卡
# 無法降級 Python，唯一解決方案: 重新燒錄

# 3. 下載正確的 Bullseye 映像檔
# 直接連結:
wget https://downloads.raspberrypi.com/raspios_oldstable_lite_armhf/images/raspios_oldstable_lite_armhf-2024-03-15/2024-03-15-raspios-bullseye-armhf-lite.img.xz

# 或瀏覽此目錄選擇版本:
# https://downloads.raspberrypi.com/raspios_oldstable_lite_armhf/images/

# 4. 使用 Raspberry Pi Imager 的 "Use custom" 功能燒錄
# 確保檔名包含 "bullseye"
```

**快速檢查清單**:
```bash
# 在 Raspberry Pi 上執行，確認版本:
echo "=== 系統版本檢查 ==="
echo -n "Debian 版本: "; cat /etc/os-release | grep VERSION_CODENAME | cut -d= -f2
echo -n "Python 版本: "; python3 --version
echo ""
echo "✅ 正確配置: bullseye + Python 3.9.x"
echo "❌ 錯誤配置: trixie/bookworm + Python 3.11+/3.13+"
```

**預防措施**:
- ✅ 直接從官方檔案庫下載指定的 Bullseye 映像
- ✅ 檢查檔名確認包含 "bullseye"
- ✅ 首次開機後立即執行版本驗證腳本
- ✅ 發現版本錯誤時，立即停止並重新燒錄
- ❌ 不要使用 Raspberry Pi Imager 的預設「最新版本」

#### 1. 服務無法啟動

**症狀**: `sudo systemctl status wifi_display` 顯示 `failed`

**檢查步驟**:

```bash
# 查看詳細錯誤
sudo journalctl -u wifi_display.service -n 100

# 手動執行測試
cd ~/wifi_spi_display/server
source ../venv/bin/activate
python server_with_webui.py
```

**常見原因**:
- Python 套件缺失 → 重新安裝 `pip install -r requirements_rpi.txt`
- 埠號被佔用 → 檢查 `netstat -tuln | grep -E '8266|8080'`
- 權限問題 → 確認 `/home/pi/wifi_spi_display` 所有者為 `pi:pi`

#### 2. 記憶體不足 (OOM Killer)

**症狀**: 伺服器突然停止，日誌顯示 `Out of memory: Killed process`

**解決方案**:

```bash
# 1. 增加 swap (已在前面設定)
free -h

# 2. 降低記憶體使用
# 編輯 config_rpi.py，調整:
IMAGE_MAX_SIZE = 3 * 1024 * 1024  # 降至 3MB
MAX_CLIENTS = 1  # 僅允許 1 個客戶端

# 3. 限制服務記憶體 (systemd)
sudo systemctl edit wifi_display.service
# 加入:
[Service]
MemoryMax=180M

# 重新載入並重啟
sudo systemctl daemon-reload
sudo systemctl restart wifi_display.service
```

#### 3. Web UI 無法存取

**症狀**: 瀏覽器無法開啟 `http://epaper-server.local:8080`

**檢查步驟**:

```bash
# 1. 檢查服務是否運行
sudo systemctl status wifi_display.service

# 2. 檢查埠號監聽
netstat -tuln | grep 8080

# 3. 檢查防火牆
sudo ufw status
sudo ufw allow 8080/tcp

# 4. 測試本地存取
curl http://localhost:8080/

# 5. 檢查 mDNS
ping epaper-server.local
avahi-browse -a | grep epaper
```

#### 4. ESP8266 無法連接

**症狀**: ESP8266 客戶端無法連接到伺服器

**檢查步驟**:

```bash
# 1. 確認伺服器監聽 0.0.0.0 (所有介面)
netstat -tuln | grep 8266
# 應顯示: 0.0.0.0:8266

# 2. 檢查防火牆
sudo ufw status | grep 8266

# 3. 檢查網路連通性
# 在 ESP8266 序列埠輸出中查看 IP 解析結果

# 4. 手動測試 WebSocket 連接
# 使用 websocat 或其他工具
```

#### 5. 圖像處理速度過慢

**症狀**: 上傳圖片後等待很久才顯示

**優化方案**:

```python
# 1. 關閉抖動演算法 (config_rpi.py)
USE_DITHER = False

# 2. 降低圖像尺寸 (暫時使用 400x240)
processor = ImageProcessor(400, 240)

# 3. 使用更快的縮放演算法
image.resize(size, Image.Resampling.BILINEAR)  # 取代 LANCZOS
```

#### 6. SD 卡寫入過多

**症狀**: SD 卡壽命縮短，系統變慢

**解決方案**:

```bash
# 1. 降低日誌等級
# config_rpi.py:
LOG_LEVEL = "WARNING"  # 改為 WARNING

# 2. 將日誌移至 tmpfs (RAM)
sudo nano /etc/fstab
# 加入:
tmpfs /home/pi/wifi_spi_display/server/logs tmpfs defaults,noatime,size=50M 0 0

# 套用
sudo mount -a

# 3. 降低 swap 使用
sudo sysctl vm.swappiness=10
```

---

## 📚 參考資源

### 官方文件

- [Raspberry Pi OS Documentation](https://www.raspberrypi.com/documentation/computers/os.html)
- [Python asyncio](https://docs.python.org/3/library/asyncio.html)
- [aiohttp Documentation](https://docs.aiohttp.org/)
- [Pillow Documentation](https://pillow.readthedocs.io/)

### 套件版本查詢

- [PyPI](https://pypi.org/)
- [Piwheels](https://www.piwheels.org/) - ARMv6 預編譯套件

### 效能調校

- [Raspberry Pi Performance Tuning](https://www.raspberrypi.com/documentation/computers/config_txt.html)
- [Python Memory Optimization](https://docs.python.org/3/library/gc.html)

---

## 📝 版本歷史

- **v1.3** (2025-10-13): **重要更新 - 明確指定 Bullseye 版本**
  - ⚠️ 強制要求使用 Debian Bullseye (Python 3.9)
  - 新增版本對照表與相容性說明
  - 新增 Python 3.13 (Trixie) 錯誤警告
  - 在多個關鍵步驟新增版本驗證
  - 新增故障排除章節: Python 版本錯誤
  - 強調錯誤版本無法降級，必須重新燒錄
- **v1.2** (2025-10-13): 簡化為有線網路部署
  - 移除 USB WiFi 適配器相關設定說明
  - 聚焦於有線網路部署流程
  - 簡化配置步驟，提升可讀性
- **v1.1** (2025-10-13): 新增 WiFi 支援說明
  - 明確標註 Raspberry Pi 1 無內建 WiFi
  - 新增 USB WiFi 適配器設定指南
  - 強調有線網路為推薦方案
  - 更新所有 WiFi 相關設定說明
- **v1.0** (2025-10-12): 初版建立，完整部署流程
- 針對 Raspberry Pi 1 Model B (512MB RAM, ARMv6) 最佳化

---

**⚠️ 部署前必讀**:
- **必須使用 Raspberry Pi OS Lite (Legacy) - Debian Bullseye**
- **必須確認 Python 3.9.x** (不是 3.11 或 3.13)
- **錯誤版本無法修復，必須重新燒錄 SD 卡**

**維護者**: WiFi SPI Display 專案團隊  
**最後更新**: 2025-10-13
