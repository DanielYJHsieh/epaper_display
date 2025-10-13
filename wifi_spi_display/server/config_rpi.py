"""
Raspberry Pi 1 Model B 專用配置
針對 512MB RAM 最佳化

適用環境:
- Raspberry Pi 1 Model B (ARMv6, 512MB RAM)
- Raspberry Pi OS Lite (Legacy, 32-bit)
- Python 3.9+
"""

# =============================================================================
# 記憶體限制
# =============================================================================

# 最大記憶體使用 (MB)
MAX_MEMORY_MB = 200

# 記憶體警告閾值 (MB)
# 當系統可用記憶體低於此值時觸發警告並強制 GC
LOW_MEMORY_THRESHOLD_MB = 100

# 記憶體危險閾值 (MB)
# 當系統可用記憶體低於此值時拒絕新請求
CRITICAL_MEMORY_THRESHOLD_MB = 50

# =============================================================================
# 圖像處理
# =============================================================================

# 上傳檔案大小限制 (bytes)
IMAGE_MAX_SIZE = 5 * 1024 * 1024  # 5MB

# 圖像快取大小 (張數)
# 降低快取以節省記憶體
IMAGE_CACHE_SIZE = 2

# 預設關閉抖動演算法
# Floyd-Steinberg 抖動會增加 CPU 與記憶體負擔
USE_DITHER = False

# 條帶顯示高度 (pixels)
# 較小的條帶高度可減少單次記憶體分配
TILE_HEIGHT = 30

# 低記憶體模式
# 啟用後會使用更保守的記憶體策略
LOW_MEMORY_MODE = True

# 中間圖像尺寸限制 (pixels)
# 處理大圖時先縮小到此尺寸，避免記憶體爆炸
MAX_INTERMEDIATE_SIZE = 2000

# 縮放演算法
# LANCZOS: 高品質但慢 (預設)
# BILINEAR: 中等品質但快 (推薦給 RPi1)
# NEAREST: 低品質但極快
RESIZE_ALGORITHM = "BILINEAR"  # "LANCZOS" | "BILINEAR" | "NEAREST"

# =============================================================================
# WebSocket 設定
# =============================================================================

# 最大同時客戶端連線數
# RPi1 記憶體有限，建議 ≤ 2
MAX_CLIENTS = 2

# WebSocket 訊息大小限制 (bytes)
WS_MAX_SIZE = 2 * 1024 * 1024  # 2MB

# WebSocket ping 間隔 (秒)
# 增加間隔以降低 CPU 負擔
WS_PING_INTERVAL = 30

# WebSocket ping 超時 (秒)
WS_PING_TIMEOUT = 10

# =============================================================================
# HTTP 設定
# =============================================================================

# HTTP 上傳大小限制 (bytes)
HTTP_MAX_UPLOAD = 5 * 1024 * 1024  # 5MB

# HTTP 請求超時 (秒)
# RPi1 處理較慢，增加超時時間
HTTP_TIMEOUT = 60

# HTTP 連線超時 (秒)
HTTP_CONNECT_TIMEOUT = 30

# =============================================================================
# 日誌設定
# =============================================================================

# 日誌等級
# DEBUG: 詳細除錯資訊 (會增加 I/O)
# INFO: 一般資訊 (預設)
# WARNING: 僅警告與錯誤 (推薦給 RPi1)
# ERROR: 僅錯誤
LOG_LEVEL = "INFO"

# 日誌檔案最大大小 (bytes)
LOG_MAX_SIZE = 10 * 1024 * 1024  # 10MB

# 日誌備份數量
LOG_BACKUP_COUNT = 3

# 日誌輸出位置
LOG_DIR = "logs"
LOG_FILE = "server.log"

# =============================================================================
# 效能監控
# =============================================================================

# 啟用記憶體監控
# 會定期檢查系統記憶體使用情況
ENABLE_MEMORY_MONITOR = True

# 記憶體檢查間隔 (秒)
MEMORY_CHECK_INTERVAL = 10

# 垃圾回收間隔 (秒)
# 主動觸發 Python GC 以釋放記憶體
GC_COLLECT_INTERVAL = 30

# 啟用效能統計
# 記錄處理時間與資源使用
ENABLE_PERFORMANCE_STATS = True

# =============================================================================
# 伺服器設定
# =============================================================================

# 預設監聽位址
DEFAULT_HOST = "0.0.0.0"

# WebSocket 埠號
DEFAULT_WS_PORT = 8266

# HTTP/Web UI 埠號
DEFAULT_HTTP_PORT = 8080

# UDP 廣播埠號 (服務探索)
UDP_BROADCAST_PORT = 8267

# UDP 廣播間隔 (秒)
UDP_BROADCAST_INTERVAL = 5

# =============================================================================
# 顯示器設定
# =============================================================================

# 預設顯示器尺寸
DEFAULT_WIDTH = 400
DEFAULT_HEIGHT = 240

# 全螢幕顯示器尺寸
FULLSCREEN_WIDTH = 800
FULLSCREEN_HEIGHT = 480

# =============================================================================
# 安全性設定
# =============================================================================

# 允許的圖像格式
ALLOWED_IMAGE_FORMATS = ['JPEG', 'PNG', 'BMP', 'GIF', 'WEBP']

# 上傳目錄
UPLOAD_DIR = "uploads"

# 暫存檔案保留時間 (秒)
TEMP_FILE_RETENTION = 3600  # 1 小時

# =============================================================================
# 實驗性功能
# =============================================================================

# 使用 PyPy (若可用)
# PyPy 在某些情況下可提升效能，但需額外安裝
USE_PYPY = False

# 啟用圖像預處理快取
# 會快取轉換後的 1-bit 圖像，加速重複發送
ENABLE_IMAGE_CACHE = True

# 壓縮演算法
# "RLE": Run-Length Encoding (簡單快速)
# "HYBRID": 混合壓縮 (較佳壓縮率但較慢)
COMPRESSION_ALGORITHM = "RLE"

# =============================================================================
# 除錯設定
# =============================================================================

# 開發模式
# 啟用後會顯示更多除錯資訊
DEBUG_MODE = False

# 詳細記錄記憶體使用
VERBOSE_MEMORY_LOGGING = False

# 記錄所有 HTTP 請求
LOG_HTTP_REQUESTS = True

# 記錄所有 WebSocket 訊息
LOG_WS_MESSAGES = False  # 會產生大量日誌

# =============================================================================
# 輔助函數
# =============================================================================

def get_config_summary():
    """取得配置摘要"""
    return {
        "platform": "Raspberry Pi 1 Model B",
        "memory": {
            "max_mb": MAX_MEMORY_MB,
            "low_threshold_mb": LOW_MEMORY_THRESHOLD_MB,
            "critical_threshold_mb": CRITICAL_MEMORY_THRESHOLD_MB,
        },
        "image": {
            "max_size_mb": IMAGE_MAX_SIZE / 1024 / 1024,
            "cache_size": IMAGE_CACHE_SIZE,
            "dither": USE_DITHER,
            "low_memory_mode": LOW_MEMORY_MODE,
        },
        "network": {
            "max_clients": MAX_CLIENTS,
            "ws_port": DEFAULT_WS_PORT,
            "http_port": DEFAULT_HTTP_PORT,
        },
        "monitoring": {
            "memory_monitor": ENABLE_MEMORY_MONITOR,
            "performance_stats": ENABLE_PERFORMANCE_STATS,
        }
    }


def print_config():
    """列印配置資訊"""
    import json
    config = get_config_summary()
    print("=" * 60)
    print("Raspberry Pi 1 Model B - Server Configuration")
    print("=" * 60)
    print(json.dumps(config, indent=2, ensure_ascii=False))
    print("=" * 60)


if __name__ == "__main__":
    # 直接執行此檔案時顯示配置
    print_config()
