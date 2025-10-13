# Raspberry Pi 1 Model B 開發總結

## 📝 任務概述

已完成將 WiFi SPI Display Server 移植到 **Raspberry Pi 1 Model B** 的規劃與基礎設定。

---

## ✅ 已完成的項目

### 1. 📚 完整部署文件

**檔案**: `docs/RASPBERRY_PI_DEPLOYMENT.md` (超過 1000 行)

**內容**:
- ✅ 硬體規格與限制分析 (512MB RAM, ARMv6 CPU, SD I/O)
- ✅ 作業系統準備 (Raspberry Pi OS Lite Legacy 32-bit)
  - SD 卡燒錄步驟
  - SSH 啟用
  - 系統配置 (raspi-config)
  - Swap 擴增至 1GB
- ✅ 軟體安裝流程
  - Python 3.9+ 環境
  - 圖像處理函式庫 (libjpeg, zlib, freetype)
  - 虛擬環境建立
  - Python 套件安裝 (使用 piwheels)
- ✅ 程式碼優化指南
  - 記憶體監控機制
  - 低記憶體模式實作
  - 圖像處理最佳化
- ✅ 系統服務配置
  - Systemd 服務單元
  - 日誌輪替
  - 自動啟動設定
- ✅ 網路與安全
  - 靜態 IP 設定
  - mDNS/Avahi 配置
  - 防火牆 (ufw)
  - SSH 安全加固
  - Nginx 反向代理 (選用)
- ✅ 測試與驗證流程
  - 功能測試腳本
  - 效能測試方法
  - 24 小時穩定性測試
- ✅ 維護與監控
  - 日常監控命令
  - 效能監控腳本
  - 備份腳本
  - 系統更新流程
- ✅ 故障排除指南
  - 常見問題與解決方案
  - 記憶體不足處理
  - 服務啟動失敗排查

### 2. ⚙️ 配置檔案

**檔案**: `server/config_rpi.py`

**內容**:
```python
# 記憶體限制
MAX_MEMORY_MB = 200
LOW_MEMORY_THRESHOLD_MB = 100
CRITICAL_MEMORY_THRESHOLD_MB = 50

# 圖像處理
IMAGE_MAX_SIZE = 5 * 1024 * 1024  # 5MB
USE_DITHER = False  # 關閉抖動 (節省 CPU)
LOW_MEMORY_MODE = True
RESIZE_ALGORITHM = "BILINEAR"  # 較快的縮放

# WebSocket
MAX_CLIENTS = 2  # 最多 2 個客戶端

# 效能監控
ENABLE_MEMORY_MONITOR = True
GC_COLLECT_INTERVAL = 30  # 每 30 秒 GC
```

### 3. 📦 套件管理

**檔案**: `server/requirements_rpi.txt`

**內容**:
```
aiohttp==3.8.5          # Web 伺服器
websockets==11.0.3      # WebSocket 協定
Pillow==10.0.0          # 圖像處理
numpy==1.24.4           # 數值運算 (ARMv6 最佳化)
psutil==5.9.5           # 系統監控
```

**特點**:
- ✅ 使用 piwheels (ARMv6 預編譯套件)
- ✅ 避免長時間編譯
- ✅ 版本固定，確保相容性

### 4. 🔧 自動化安裝腳本

**檔案**: `server/install_rpi.sh`

**功能**:
- ✅ 系統套件更新
- ✅ Python 開發工具安裝
- ✅ 圖像處理函式庫安裝
- ✅ 虛擬環境建立
- ✅ Python 套件安裝 (使用 piwheels)
- ✅ 目錄結構建立
- ✅ 安裝測試與驗證
- ✅ 彩色輸出與錯誤處理

**使用方式**:
```bash
chmod +x install_rpi.sh
./install_rpi.sh
```

### 5. 🎛️ Systemd 服務

**檔案**: `server/wifi_display.service`

**配置**:
```ini
[Service]
# 記憶體限制
MemoryMax=250M
MemoryHigh=200M

# CPU 限制
CPUQuota=90%

# 重啟策略
Restart=always
RestartSec=10

# 安全性
NoNewPrivileges=true
ProtectSystem=strict
```

**管理指令**:
```bash
# 安裝服務
sudo cp wifi_display.service /etc/systemd/system/
sudo systemctl daemon-reload

# 啟用與啟動
sudo systemctl enable wifi_display.service
sudo systemctl start wifi_display.service

# 查看狀態
sudo systemctl status wifi_display.service

# 查看日誌
sudo journalctl -u wifi_display.service -f
```

### 6. 📋 開發任務清單

**檔案**: `docs/RPI_DEVELOPMENT_TASKS.md`

**內容**:
- ✅ 已完成項目總覽 (70%)
- ✅ 待實作項目清單 (30%)
- ✅ 階段性開發計劃
- ✅ 實作建議流程 (方案 A 與 B)
- ✅ 測試與驗證項目

### 7. 📖 專案 README 更新

**檔案**: `README.md`

**更新**:
- ✅ 新增 Raspberry Pi 部署指南連結
- ✅ 快速導覽區塊更新

---

## 🔄 待實作的項目 (30%)

### 程式碼修改 (需要實機測試)

#### 1. `server/server_with_webui.py`

**需新增**:
```python
# 記憶體監控協程
async def _memory_monitor(self):
    while True:
        mem = psutil.virtual_memory()
        if mem.available < LOW_MEMORY_THRESHOLD_MB * 1024 * 1024:
            logger.warning(f"記憶體不足: {mem.available/1024/1024:.1f}MB")
            gc.collect()
        await asyncio.sleep(10)

# 上傳時記憶體檢查
async def handle_upload(self, request):
    mem = psutil.virtual_memory()
    if mem.available < CRITICAL_MEMORY_THRESHOLD_MB * 1024 * 1024:
        return web.json_response({
            'success': False,
            'message': '記憶體不足'
        }, status=503)
    # ... 原有邏輯
    gc.collect()  # 上傳完成後 GC
```

#### 2. `server/image_processor.py`

**需修改**:
```python
class ImageProcessor:
    def __init__(self, width, height, low_memory_mode=True):
        self.low_memory_mode = low_memory_mode
    
    def convert_to_1bit(self, image, dither=False):
        if self.low_memory_mode:
            # 大圖先縮小到中間尺寸
            if max(image.size) > 2000:
                scale = 2000 / max(image.size)
                intermediate = (int(image.width * scale), int(image.height * scale))
                image = image.resize(intermediate, Image.Resampling.BILINEAR)
                gc.collect()
        # ... 繼續處理
```

### 測試項目

- [ ] 功能測試 (服務啟動、Web UI、圖片上傳)
- [ ] 效能測試 (記憶體峰值、CPU 負載、處理速度)
- [ ] 穩定性測試 (24 小時運行、記憶體洩漏)
- [ ] ESP8266 整合測試

### 系統整合

- [ ] 網路設定 (靜態 IP、mDNS)
- [ ] 防火牆配置 (ufw)
- [ ] SSH 安全加固

---

## 📊 檔案清單

### 新建檔案

```
wifi_spi_display/
├── docs/
│   ├── RASPBERRY_PI_DEPLOYMENT.md    ✅ 完整部署指南 (1000+ 行)
│   └── RPI_DEVELOPMENT_TASKS.md      ✅ 開發任務清單
│
└── server/
    ├── config_rpi.py                 ✅ RPi 最佳化配置
    ├── requirements_rpi.txt          ✅ RPi 相容套件清單
    ├── install_rpi.sh                ✅ 自動化安裝腳本
    └── wifi_display.service          ✅ Systemd 服務單元
```

### 修改檔案

```
wifi_spi_display/
└── README.md                         ✅ 新增 RPi 部署連結
```

---

## 🚀 快速開始流程

### 在 Raspberry Pi 1 Model B 上部署

#### 1. 準備 SD 卡

```bash
1. 下載 Raspberry Pi OS Lite (Legacy, 32-bit)
2. 使用 Raspberry Pi Imager 燒錄
3. 進階設定:
   - 啟用 SSH
   - 設定使用者名稱與密碼
   - 配置 WiFi (若使用)
```

#### 2. 首次開機設定

```bash
# SSH 連線
ssh pi@<RPi_IP>

# 系統更新
sudo apt update && sudo apt upgrade -y

# 擴展檔案系統與配置
sudo raspi-config
# - 擴展檔案系統
# - 設定時區 (Asia/Taipei)
# - GPU 記憶體改為 16MB

# 增加 swap
sudo nano /etc/dphys-swapfile
# 改為: CONF_SWAPSIZE=1024
sudo dphys-swapfile setup
sudo dphys-swapfile swapon
```

#### 3. 下載專案並安裝

```bash
# 建立目錄
mkdir -p ~/wifi_spi_display
cd ~/wifi_spi_display

# 下載專案 (使用 git 或 scp 上傳)
git clone <你的 repo URL> .
# 或
scp -r /path/to/wifi_spi_display/* pi@<RPi_IP>:~/wifi_spi_display/

# 執行安裝腳本
cd server
chmod +x install_rpi.sh
./install_rpi.sh
```

#### 4. 測試執行

```bash
# 啟用虛擬環境
source ../venv/bin/activate

# 手動執行測試
python server_with_webui.py

# 開啟瀏覽器測試
# http://<RPi_IP>:8080
```

#### 5. 設定系統服務

```bash
# 安裝 systemd 服務
sudo cp wifi_display.service /etc/systemd/system/
sudo systemctl daemon-reload

# 啟用並啟動
sudo systemctl enable wifi_display.service
sudo systemctl start wifi_display.service

# 檢查狀態
sudo systemctl status wifi_display.service
```

#### 6. 配置網路

```bash
# 設定靜態 IP
sudo nano /etc/dhcpcd.conf
# 加入:
# interface eth0
# static ip_address=192.168.1.100/24
# static routers=192.168.1.1

# 安裝 Avahi (mDNS)
sudo apt install -y avahi-daemon

# 測試
ping epaper-server.local
```

---

## 📈 效能預期

### 記憶體使用

| 狀態 | RAM 使用 | 說明 |
|------|---------|------|
| 系統基礎 | ~150MB | Raspberry Pi OS Lite |
| 伺服器閒置 | ~50MB | Python + aiohttp + websockets |
| 圖像處理中 | ~150-200MB | Pillow + numpy 處理 800x480 |
| **總計** | **~350MB** | 尚餘 ~160MB 緩衝 |

### 處理速度

| 操作 | 時間 | 說明 |
|------|------|------|
| 800×480 圖像轉換 | 2-3 秒 | 無抖動，BILINEAR 縮放 |
| 400×240 圖像轉換 | 0.5-1 秒 | 較小尺寸 |
| WebSocket 傳輸 | <1 秒 | 取決於網路 |

### CPU 使用

- **閒置**: 5-10%
- **圖像處理**: 80-100% (短時間)
- **平均負載**: < 0.5

---

## ⚠️ 重要注意事項

### 1. 記憶體限制

- Raspberry Pi 1 僅有 512MB RAM
- 系統佔用約 150-200MB
- 可用記憶體約 300-350MB
- **必須** 啟用 swap (建議 1GB)
- **必須** 使用低記憶體模式配置

### 2. CPU 效能

- 單核心 700MHz ARM11
- 圖像處理較慢 (800×480 約需 2-3 秒)
- 建議關閉抖動演算法
- 限制同時連線數 (≤ 2 個客戶端)

### 3. SD 卡壽命

- 頻繁寫入會縮短壽命
- 使用高耐久度 SD 卡 (如 SanDisk High Endurance)
- 降低日誌等級 (WARNING)
- 考慮將日誌寫入 tmpfs (RAM)
- 定期備份系統

### 4. 網路穩定性

- 建議使用有線網路 (更穩定)
- WiFi 適配器會增加 USB 匯流排負擔
- 使用靜態 IP 或 DHCP 保留

---

## 📚 參考文件

### 主要文件

1. **部署指南** (必讀): `docs/RASPBERRY_PI_DEPLOYMENT.md`
   - 完整安裝步驟
   - 程式碼修改指南
   - 測試與驗證
   - 故障排除

2. **開發任務**: `docs/RPI_DEVELOPMENT_TASKS.md`
   - 已完成項目
   - 待實作項目
   - 實作建議流程

3. **配置參數**: `server/config_rpi.py`
   - 記憶體限制
   - 效能參數
   - 功能開關

### 官方資源

- [Raspberry Pi OS 文件](https://www.raspberrypi.com/documentation/)
- [Python asyncio](https://docs.python.org/3/library/asyncio.html)
- [Pillow 文件](https://pillow.readthedocs.io/)
- [Piwheels](https://www.piwheels.org/) - ARMv6 預編譯套件

---

## 🎯 下一步

### 方案 A: 有硬體 (推薦)

1. 燒錄 SD 卡
2. 執行 `install_rpi.sh`
3. 手動測試伺服器
4. 監控記憶體使用
5. 根據測試結果調整配置
6. 設定系統服務
7. 長期穩定性測試

### 方案 B: 無硬體

1. 修改程式碼 (記憶體監控、低記憶體模式)
2. 本機模擬測試 (限制記憶體使用)
3. 取得硬體後執行方案 A

---

## ✅ 總結

### 已交付成果

✅ **完整部署文件** (1000+ 行，涵蓋所有細節)  
✅ **自動化安裝腳本** (一鍵安裝所有相依套件)  
✅ **系統服務配置** (開機自動啟動，資源限制)  
✅ **最佳化配置檔** (針對 512MB RAM 最佳化)  
✅ **開發任務清單** (明確列出已完成與待實作項目)  

### 關鍵優勢

🎯 **記憶體最佳化**: 限制使用 200MB，為系統保留 300MB  
🎯 **ARMv6 相容**: 使用 piwheels 預編譯套件，避免長時間編譯  
🎯 **自動化安裝**: 一鍵完成系統配置與套件安裝  
🎯 **穩定可靠**: Systemd 服務管理，自動重啟與資源限制  
🎯 **易於維護**: 詳細文件、監控腳本、故障排除指南  

### 待完成工作

⏳ 程式碼記憶體監控實作 (約 2 小時)  
⏳ 實機功能與效能測試 (約 3 小時)  
⏳ 系統整合與網路配置 (約 1 小時)  

**預估總工時**: 6-8 小時 (已完成基礎設定與文件，實際實作時間)

---

**建立日期**: 2025-10-12  
**版本**: 1.0  
**維護**: WiFi SPI Display 專案團隊
