#!/bin/bash
################################################################################
# WiFi SPI Display Server - Raspberry Pi 1 Model B 安裝腳本
#
# 用途: 自動化安裝所有必要的系統套件、Python 環境與相依套件
# 適用: Raspberry Pi 1 Model B (ARMv6, 512MB RAM)
# 作業系統: Raspberry Pi OS Lite (Legacy, 32-bit)
#
# 使用方式:
#   chmod +x install_rpi.sh
#   ./install_rpi.sh
#
# 作者: WiFi SPI Display 專案團隊
# 版本: 1.0
# 日期: 2025-10-12
################################################################################

set -e  # 遇到錯誤立即停止
set -u  # 使用未定義變數時報錯

# 顏色輸出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 函數: 列印訊息
print_header() {
    echo -e "${BLUE}=====================================${NC}"
    echo -e "${BLUE} $1${NC}"
    echo -e "${BLUE}=====================================${NC}"
}

print_step() {
    echo -e "${GREEN}➤ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

# 檢查是否為 root
if [ "$EUID" -eq 0 ]; then
    print_error "請勿使用 root 執行此腳本"
    echo "正確用法: ./install_rpi.sh"
    exit 1
fi

# 檢查是否為 Raspberry Pi
if [ ! -f /proc/device-tree/model ]; then
    print_warning "偵測不到 Raspberry Pi 裝置資訊"
    read -p "確定要繼續安裝嗎? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
else
    RPI_MODEL=$(cat /proc/device-tree/model)
    echo "偵測到裝置: $RPI_MODEL"
fi

# 檢查 Python 版本
PYTHON_VERSION=$(python3 --version 2>&1 | awk '{print $2}')
PYTHON_MAJOR=$(echo $PYTHON_VERSION | cut -d. -f1)
PYTHON_MINOR=$(echo $PYTHON_VERSION | cut -d. -f2)

if [ "$PYTHON_MAJOR" -lt 3 ] || { [ "$PYTHON_MAJOR" -eq 3 ] && [ "$PYTHON_MINOR" -lt 9 ]; }; then
    print_error "需要 Python 3.9 或更新版本 (目前: $PYTHON_VERSION)"
    exit 1
fi

print_header "WiFi SPI Display Server 安裝"
echo "Python 版本: $PYTHON_VERSION"
echo "安裝路徑: $(pwd)"
echo ""

read -p "按 Enter 繼續安裝，或 Ctrl+C 取消..."

# ============================================================================
# 步驟 1: 更新系統套件
# ============================================================================

print_header "步驟 1/8: 更新系統套件"
print_step "執行 apt update..."

sudo apt update || {
    print_error "apt update 失敗，請檢查網路連線"
    exit 1
}

print_step "✓ 系統套件清單已更新"
echo ""

# ============================================================================
# 步驟 2: 安裝 Python 開發工具
# ============================================================================

print_header "步驟 2/8: 安裝 Python 開發工具"

PYTHON_PACKAGES=(
    python3-dev
    python3-pip
    python3-venv
    python3-setuptools
    python3-wheel
)

print_step "安裝套件: ${PYTHON_PACKAGES[*]}"
sudo apt install -y "${PYTHON_PACKAGES[@]}" || {
    print_error "Python 開發工具安裝失敗"
    exit 1
}

print_step "✓ Python 開發工具安裝完成"
echo ""

# ============================================================================
# 步驟 3: 安裝圖像處理系統函式庫
# ============================================================================

print_header "步驟 3/8: 安裝圖像處理函式庫"

IMAGE_PACKAGES=(
    libjpeg-dev
    zlib1g-dev
    libfreetype6-dev
    liblcms2-dev
    libopenjp2-7-dev
    libtiff5-dev
    libwebp-dev
    tcl8.6-dev
    tk8.6-dev
)

print_step "安裝套件: ${IMAGE_PACKAGES[*]}"
sudo apt install -y "${IMAGE_PACKAGES[@]}" || {
    print_error "圖像處理函式庫安裝失敗"
    exit 1
}

print_step "✓ 圖像處理函式庫安裝完成"
echo ""

# ============================================================================
# 步驟 4: 安裝系統工具
# ============================================================================

print_header "步驟 4/8: 安裝系統工具"

SYSTEM_TOOLS=(
    git
    vim
    htop
    curl
    rsync
    screen
)

print_step "安裝套件: ${SYSTEM_TOOLS[*]}"
sudo apt install -y "${SYSTEM_TOOLS[@]}" || {
    print_warning "部分系統工具安裝失敗 (非關鍵錯誤)"
}

print_step "✓ 系統工具安裝完成"
echo ""

# ============================================================================
# 步驟 5: 建立 Python 虛擬環境
# ============================================================================

print_header "步驟 5/8: 建立 Python 虛擬環境"

if [ -d "venv" ]; then
    print_warning "虛擬環境已存在，跳過建立"
else
    print_step "建立虛擬環境..."
    python3 -m venv venv || {
        print_error "虛擬環境建立失敗"
        exit 1
    }
    print_step "✓ 虛擬環境建立完成"
fi

echo ""

# ============================================================================
# 步驟 6: 安裝 Python 套件
# ============================================================================

print_header "步驟 6/8: 安裝 Python 套件"

print_step "啟用虛擬環境..."
source venv/bin/activate

print_step "升級 pip..."
pip install --upgrade pip

if [ ! -f "server/requirements_rpi.txt" ]; then
    print_error "找不到 requirements_rpi.txt"
    print_error "請確認檔案位於: server/requirements_rpi.txt"
    exit 1
fi

print_step "安裝 Python 套件 (使用 piwheels)..."
print_warning "這可能需要 5-15 分鐘，請耐心等候..."

pip install --index-url https://www.piwheels.org/simple \
            --extra-index-url https://pypi.org/simple \
            -r server/requirements_rpi.txt || {
    print_error "Python 套件安裝失敗"
    print_error "請檢查網路連線與系統套件是否完整安裝"
    exit 1
}

print_step "✓ Python 套件安裝完成"
echo ""

# ============================================================================
# 步驟 7: 建立必要目錄
# ============================================================================

print_header "步驟 7/8: 建立目錄結構"

DIRS=(
    "server/logs"
    "server/uploads"
)

for dir in "${DIRS[@]}"; do
    if [ ! -d "$dir" ]; then
        print_step "建立目錄: $dir"
        mkdir -p "$dir"
    else
        print_step "目錄已存在: $dir"
    fi
done

print_step "✓ 目錄結構建立完成"
echo ""

# ============================================================================
# 步驟 8: 測試安裝
# ============================================================================

print_header "步驟 8/8: 測試安裝"

print_step "測試 Python 套件載入..."

python << 'EOF'
import sys

packages = {
    'aiohttp': 'Web 伺服器',
    'websockets': 'WebSocket 協定',
    'PIL': '圖像處理 (Pillow)',
    'numpy': '數值運算',
    'psutil': '系統監控'
}

all_ok = True

for pkg_name, description in packages.items():
    try:
        if pkg_name == 'PIL':
            import PIL
            from PIL import Image
        else:
            __import__(pkg_name)
        print(f"  ✓ {pkg_name:12} - {description}")
    except ImportError as e:
        print(f"  ✗ {pkg_name:12} - 載入失敗: {e}")
        all_ok = False

if all_ok:
    print("\n✅ 所有套件測試通過！")
    sys.exit(0)
else:
    print("\n❌ 部分套件測試失敗")
    sys.exit(1)
EOF

TEST_RESULT=$?

if [ $TEST_RESULT -eq 0 ]; then
    echo ""
    print_header "安裝完成！"
    
    echo -e "${GREEN}✅ WiFi SPI Display Server 安裝成功${NC}"
    echo ""
    echo "下一步:"
    echo "  1. 測試執行伺服器:"
    echo -e "     ${YELLOW}source venv/bin/activate${NC}"
    echo -e "     ${YELLOW}cd server${NC}"
    echo -e "     ${YELLOW}python server_with_webui.py${NC}"
    echo ""
    echo "  2. 設定系統服務 (開機自動啟動):"
    echo -e "     ${YELLOW}sudo cp wifi_display.service /etc/systemd/system/${NC}"
    echo -e "     ${YELLOW}sudo systemctl daemon-reload${NC}"
    echo -e "     ${YELLOW}sudo systemctl enable wifi_display.service${NC}"
    echo -e "     ${YELLOW}sudo systemctl start wifi_display.service${NC}"
    echo ""
    echo "  3. 查看服務狀態:"
    echo -e "     ${YELLOW}sudo systemctl status wifi_display.service${NC}"
    echo ""
    echo "  4. Web UI 存取位址:"
    LOCAL_IP=$(hostname -I | awk '{print $1}')
    echo -e "     ${BLUE}http://${LOCAL_IP}:8080${NC}"
    echo -e "     ${BLUE}http://$(hostname).local:8080${NC}"
    echo ""
    echo "完整文件請參考: docs/RASPBERRY_PI_DEPLOYMENT.md"
    
else
    echo ""
    print_error "安裝測試失敗"
    print_error "請檢查錯誤訊息並重新執行安裝"
    exit 1
fi
