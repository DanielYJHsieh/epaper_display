#!/bin/bash
# WiFi SPI Display Server - Raspberry Pi 自動部署腳本
# 適用於 Raspberry Pi 1 Model B (ARMv6, Python 3.9)
# 使用方法: bash deploy_rpi.sh

set -e  # 遇到錯誤立即停止

echo "====================================="
echo " WiFi SPI Display Server"
echo " Raspberry Pi 自動部署"
echo "====================================="
echo ""

# 顏色定義
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 檢查是否為 root
if [ "$EUID" -eq 0 ]; then
  echo -e "${RED}❌ 請勿使用 root 執行此腳本${NC}"
  exit 1
fi

# 檢查 Python 版本
echo "🔍 檢查 Python 版本..."
PYTHON_VERSION=$(python3 --version | awk '{print $2}')
PYTHON_MAJOR=$(echo $PYTHON_VERSION | cut -d. -f1)
PYTHON_MINOR=$(echo $PYTHON_VERSION | cut -d. -f2)

if [ "$PYTHON_MAJOR" -ne 3 ] || [ "$PYTHON_MINOR" -ne 9 ]; then
  echo -e "${RED}❌ 錯誤: 需要 Python 3.9.x，但目前是 $PYTHON_VERSION${NC}"
  echo -e "${YELLOW}⚠️  請使用 Raspberry Pi OS Bullseye (Legacy) 版本${NC}"
  exit 1
fi
echo -e "${GREEN}✅ Python 版本正確: $PYTHON_VERSION${NC}"
echo ""

# 檢查 Debian 版本
echo "🔍 檢查作業系統版本..."
OS_CODENAME=$(grep VERSION_CODENAME /etc/os-release | cut -d= -f2)
if [ "$OS_CODENAME" != "bullseye" ]; then
  echo -e "${YELLOW}⚠️  警告: 目前系統是 $OS_CODENAME，建議使用 bullseye${NC}"
fi
echo -e "${GREEN}✅ 作業系統: Debian $OS_CODENAME${NC}"
echo ""

# 1. 更新系統套件
echo "📦 更新系統套件..."
sudo apt update
echo ""

# 2. 安裝系統依賴套件
echo "📚 安裝系統依賴套件..."
sudo apt install -y \
  python3-dev \
  python3-pip \
  python3-venv \
  python3-full \
  libjpeg-dev \
  zlib1g-dev \
  libfreetype6-dev \
  libopenjp2-7-dev \
  libtiff5-dev \
  git \
  vim \
  htop
echo -e "${GREEN}✅ 系統套件安裝完成${NC}"
echo ""

# 3. 建立虛擬環境
echo "🐍 建立 Python 虛擬環境..."
if [ -d "venv" ]; then
  echo -e "${YELLOW}⚠️  虛擬環境已存在，跳過建立${NC}"
else
  python3 -m venv venv
  echo -e "${GREEN}✅ 虛擬環境建立完成${NC}"
fi
echo ""

# 4. 啟用虛擬環境並安裝 Python 套件
echo "📦 安裝 Python 套件..."
source venv/bin/activate

# 升級 pip
pip install --upgrade pip

# 檢查 requirements_rpi.txt 是否存在
if [ ! -f "requirements_rpi.txt" ]; then
  echo -e "${YELLOW}⚠️  requirements_rpi.txt 不存在，建立預設版本...${NC}"
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
fi

# 使用 piwheels 安裝套件
pip install --index-url https://www.piwheels.org/simple \
  --extra-index-url https://pypi.org/simple \
  -r requirements_rpi.txt

echo -e "${GREEN}✅ Python 套件安裝完成${NC}"
echo ""

# 5. 建立必要目錄
echo "📁 建立目錄結構..."
mkdir -p logs
mkdir -p uploads
echo -e "${GREEN}✅ 目錄建立完成${NC}"
echo ""

# 6. 測試安裝
echo "🧪 測試套件安裝..."
python << 'PYEOF'
import sys
print(f"Python: {sys.version}")

try:
    import aiohttp
    print(f"✅ aiohttp: {aiohttp.__version__}")
    
    import websockets
    print(f"✅ websockets: {websockets.__version__}")
    
    from PIL import Image
    print(f"✅ Pillow: {Image.__version__}")
    
    import numpy as np
    print(f"✅ numpy: {np.__version__}")
    
    import psutil
    print(f"✅ psutil: {psutil.__version__}")
    
    print("\n✅ 所有套件載入成功！")
except ImportError as e:
    print(f"\n❌ 錯誤: {e}")
    sys.exit(1)
PYEOF

if [ $? -ne 0 ]; then
  echo -e "${RED}❌ 套件測試失敗${NC}"
  exit 1
fi
echo ""

# 7. 顯示完成訊息
echo "====================================="
echo -e "${GREEN} ✅ 部署完成！${NC}"
echo "====================================="
echo ""
echo "下一步:"
echo "1. 測試執行: source venv/bin/activate && python server_with_webui.py"
echo "2. 設定開機自動啟動: bash install_service.sh"
echo ""
echo "常用命令:"
echo "  啟動服務: sudo systemctl start wifi_display"
echo "  停止服務: sudo systemctl stop wifi_display"
echo "  查看狀態: sudo systemctl status wifi_display"
echo "  查看日誌: sudo journalctl -u wifi_display -f"
echo ""
