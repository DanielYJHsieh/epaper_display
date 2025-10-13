# Raspberry Pi 1 Model B 開發任務清單

本文件列出將 WiFi SPI Display Server 部署到 **Raspberry Pi 1 Model B** 所需的開發與實作任務。

---

## 📋 硬體環境

- **裝置**: Raspberry Pi 1 Model B
- **CPU**: ARM1176JZF-S (ARMv6) 700MHz 單核心
- **記憶體**: 512MB SDRAM
- **儲存**: SD 卡 (建議 ≥ 16GB Class 10)
- **網路**: 10/100 Mbps 乙太網路
- **功耗**: ~3.5W (700mA @ 5V)

---

## ✅ 已完成的開發項目

### 1. 文件建立 ✓

- [x] **完整部署指南** (`docs/RASPBERRY_PI_DEPLOYMENT.md`)
  - 硬體規格與限制分析
  - 作業系統準備 (Raspberry Pi OS Legacy 32-bit)
  - 軟體安裝步驟
  - 程式碼優化指南
  - 系統服務配置
  - 網路與安全設定
  - 測試與驗證流程
  - 維護與監控方法
  - 故障排除指南

### 2. 配置檔案 ✓

- [x] **RPi 最佳化配置** (`server/config_rpi.py`)
  - 記憶體限制與閾值 (200MB max, 100MB warning)
  - 圖像處理設定 (5MB 上傳限制, 關閉抖動)
  - WebSocket 設定 (最多 2 個客戶端)
  - HTTP 設定 (60 秒超時)
  - 日誌配置 (10MB 上限)
  - 效能監控參數 (記憶體監控, GC 間隔)

### 3. 套件管理 ✓

- [x] **RPi 相容 requirements** (`server/requirements_rpi.txt`)
  - aiohttp 3.8.5
  - websockets 11.0.3
  - Pillow 10.0.0
  - numpy 1.24.4
  - psutil 5.9.5
  - 使用 piwheels 預編譯套件 (ARMv6 最佳化)

### 4. 自動化安裝 ✓

- [x] **安裝腳本** (`server/install_rpi.sh`)
  - 系統套件更新
  - Python 開發工具安裝
  - 圖像處理函式庫安裝
  - Python 虛擬環境建立
  - Python 套件安裝 (使用 piwheels)
  - 目錄結構建立
  - 安裝測試與驗證

### 5. 系統服務 ✓

- [x] **Systemd 服務單元** (`server/wifi_display.service`)
  - 服務自動啟動配置
  - 記憶體限制 (250MB max, 200MB high)
  - CPU 配額限制 (90%)
  - 重啟策略 (自動重啟, 限流保護)
  - 日誌配置 (journal)
  - 安全性設定 (NoNewPrivileges, ProtectSystem)

---

## 🚧 待實作的開發項目

### 階段 1: 程式碼最佳化 (預估 3-4 小時)

#### 1.1 修改 `server_with_webui.py`

**目標**: 新增記憶體監控與低記憶體模式支援

**需要修改**:

```python
# 在檔案開頭新增
import gc
import psutil
from config_rpi import (
    MAX_MEMORY_MB, LOW_MEMORY_THRESHOLD_MB, 
    IMAGE_MAX_SIZE, MAX_CLIENTS,
    ENABLE_MEMORY_MONITOR, GC_COLLECT_INTERVAL
)

class DisplayServer:
    def __init__(self, ...):
        # 修改上傳限制
        self.http_app = web.Application(client_max_size=IMAGE_MAX_SIZE)
        
        # 新增記憶體監控
        self.memory_monitor_enabled = ENABLE_MEMORY_MONITOR
        if self.memory_monitor_enabled:
            asyncio.create_task(self._memory_monitor())
    
    async def _memory_monitor(self):
        """記憶體監控協程"""
        # 定期檢查記憶體使用
        # 低於閾值時觸發 GC
        # 記錄警告訊息
    
    async def handle_upload(self, request):
        """處理上傳 (加入記憶體檢查)"""
        # 上傳前檢查可用記憶體
        # 記憶體不足時返回 503
        # 上傳完成後強制 GC
```

**檔案位置**: `server/server_with_webui.py`  
**修改行數**: 約 50-100 行  
**測試**: 上傳多張圖片，監控記憶體使用

#### 1.2 修改 `image_processor.py`

**目標**: 新增低記憶體模式與串流處理

**需要修改**:

```python
class ImageProcessor:
    def __init__(self, width, height, low_memory_mode=True):
        self.low_memory_mode = low_memory_mode
    
    def convert_to_1bit(self, image, dither=False):
        """低記憶體模式: 分段處理大圖"""
        if self.low_memory_mode:
            # 大圖先縮小到中間尺寸
            # 避免一次載入太多記憶體
        # 使用 BILINEAR 取代 LANCZOS (較快)
    
    def image_to_bytes(self, image):
        """低記憶體模式: 直接使用 tobytes()"""
        if self.low_memory_mode:
            return image.tobytes()  # 不使用 numpy
```

**檔案位置**: `server/image_processor.py`  
**修改行數**: 約 30-50 行  
**測試**: 處理 5MB 大圖，監控記憶體峰值

---

### 階段 2: 測試與驗證 (預估 2-3 小時)

#### 2.1 功能測試

**測試項目**:
- [ ] 服務啟動/停止
- [ ] Web UI 存取 (http://epaper-server.local:8080)
- [ ] 圖片上傳與顯示
- [ ] WebSocket 連線穩定性
- [ ] ESP8266 客戶端整合

**測試腳本**: `test_server.sh` (已在部署指南中提供)

#### 2.2 效能測試

**測試項目**:
- [ ] 記憶體使用監控 (預期 < 200MB)
- [ ] CPU 負載測試 (多張圖片上傳)
- [ ] 圖像處理速度 (800x480 約 2-3 秒)
- [ ] 併發連線測試 (2 個客戶端)

**監控工具**: `htop`, `free -h`, `vmstat`

#### 2.3 穩定性測試

**測試項目**:
- [ ] 24 小時運行測試
- [ ] 記憶體洩漏檢測
- [ ] 重複上傳測試 (100 次)
- [ ] 異常情況處理 (網路斷線, 大檔上傳)

**測試腳本**: `monitor_stability.sh` (已在部署指南中提供)

---

### 階段 3: 系統整合 (預估 1-2 小時)

#### 3.1 網路設定

**任務**:
- [ ] 設定靜態 IP (編輯 `/etc/dhcpcd.conf`)
- [ ] 安裝並配置 Avahi (mDNS 服務探索)
- [ ] 測試 `epaper-server.local` 解析

**參考**: 部署指南「網路與安全」章節

#### 3.2 防火牆配置

**任務**:
- [ ] 安裝 ufw
- [ ] 允許 SSH (22), WebSocket (8266), HTTP (8080)
- [ ] 允許 mDNS (5353/udp)
- [ ] 啟用防火牆

**指令**:
```bash
sudo apt install -y ufw
sudo ufw allow 22/tcp
sudo ufw allow 8266/tcp
sudo ufw allow 8080/tcp
sudo ufw allow 5353/udp
sudo ufw enable
```

#### 3.3 SSH 安全加固

**任務**:
- [ ] 設定 SSH 金鑰認證
- [ ] 禁用密碼登入
- [ ] 限制登入嘗試次數

---

### 階段 4: 文件補充 (預估 1 小時)

#### 4.1 使用手冊

**內容**:
- [ ] 快速啟動指南
- [ ] 常見操作流程
- [ ] 故障排除 FAQ

**檔案**: `docs/USER_MANUAL.md` (待建立)

#### 4.2 維護指南

**內容**:
- [ ] 日常監控命令
- [ ] 備份與復原流程
- [ ] 系統更新步驟
- [ ] 效能調校技巧

**檔案**: `docs/MAINTENANCE_GUIDE.md` (待建立)

---

## 📊 開發進度總覽

### 已完成 (70%)

- ✅ 硬體分析與相容性評估
- ✅ 完整部署指南文件
- ✅ RPi 最佳化配置檔
- ✅ 套件相依性管理 (requirements_rpi.txt)
- ✅ 自動化安裝腳本
- ✅ Systemd 服務配置

### 待完成 (30%)

- ⏳ 程式碼記憶體優化 (server_with_webui.py, image_processor.py)
- ⏳ 功能測試與效能驗證
- ⏳ 系統整合 (網路、防火牆、SSH)
- ⏳ 補充使用與維護文件

---

## 🛠️ 實作建議流程

### 方案 A: 實機測試優先 (推薦)

適合已有 Raspberry Pi 1 硬體的情況:

1. **燒錄 SD 卡**
   - 下載 Raspberry Pi OS Lite (Legacy, 32-bit)
   - 使用 Raspberry Pi Imager 燒錄
   - 啟用 SSH 與設定 WiFi

2. **執行安裝腳本**
   ```bash
   cd ~/wifi_spi_display/server
   chmod +x install_rpi.sh
   ./install_rpi.sh
   ```

3. **手動測試伺服器**
   ```bash
   source venv/bin/activate
   python server_with_webui.py
   ```

4. **監控記憶體使用**
   ```bash
   # 另開終端
   watch -n 1 'free -h && ps aux | grep server_with_webui'
   ```

5. **根據測試結果調整**
   - 修改 `config_rpi.py` 參數
   - 優化 `image_processor.py` 演算法
   - 調整 systemd 資源限制

6. **設定系統服務**
   ```bash
   sudo cp wifi_display.service /etc/systemd/system/
   sudo systemctl daemon-reload
   sudo systemctl enable wifi_display.service
   sudo systemctl start wifi_display.service
   ```

7. **長期穩定性測試**
   ```bash
   screen -S stability
   ~/monitor_stability.sh
   # Ctrl+A, D 離開
   ```

### 方案 B: 程式碼優先

適合尚未取得硬體，先完成程式碼修改:

1. **修改程式碼**
   - 實作 `server_with_webui.py` 記憶體監控
   - 實作 `image_processor.py` 低記憶體模式
   - 增加單元測試

2. **本機模擬測試**
   ```bash
   # 限制 Python 記憶體使用
   ulimit -v 250000  # 250MB
   python server_with_webui.py
   ```

3. **準備硬體後執行方案 A 的步驟 1-7**

---

## 🔗 相關文件

- **部署指南**: [docs/RASPBERRY_PI_DEPLOYMENT.md](docs/RASPBERRY_PI_DEPLOYMENT.md)
- **專案規劃**: [PROJECT_PLAN.md](PROJECT_PLAN.md)
- **伺服器說明**: [server/README.md](server/README.md)
- **配置參數**: [server/config_rpi.py](server/config_rpi.py)

---

## 📞 支援

若在開發過程中遇到問題，請參考:

1. **故障排除**: 部署指南「故障排除」章節
2. **系統日誌**: `sudo journalctl -u wifi_display.service -f`
3. **Raspberry Pi 官方文件**: https://www.raspberrypi.com/documentation/

---

**版本**: 1.0  
**建立日期**: 2025-10-12  
**維護**: WiFi SPI Display 專案團隊
