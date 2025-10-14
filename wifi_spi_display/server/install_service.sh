#!/bin/bash
# WiFi SPI Display Server - systemd 服務安裝腳本
# 使用方法: bash install_service.sh

set -e

echo "====================================="
echo " WiFi Display Server 服務安裝"
echo "====================================="
echo ""

# 顏色定義
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# 檢查是否在正確目錄
if [ ! -f "server_with_webui.py" ]; then
  echo -e "${RED}❌ 錯誤: 請在 server 目錄中執行此腳本${NC}"
  exit 1
fi

# 取得當前使用者和工作目錄
CURRENT_USER=$(whoami)
WORK_DIR=$(pwd)
VENV_PATH="$WORK_DIR/venv/bin/python"

echo "🔍 設定資訊:"
echo "  使用者: $CURRENT_USER"
echo "  工作目錄: $WORK_DIR"
echo "  Python: $VENV_PATH"
echo ""

# 檢查虛擬環境是否存在
if [ ! -f "$VENV_PATH" ]; then
  echo -e "${RED}❌ 錯誤: 找不到虛擬環境${NC}"
  echo "請先執行: bash deploy_rpi.sh"
  exit 1
fi

# 建立 systemd 服務檔案
echo "📝 建立 systemd 服務檔案..."
cat > /tmp/wifi_display.service << EOF
[Unit]
Description=WiFi SPI Display Server (E-Paper WebUI)
Documentation=https://github.com/DanielYJHsieh/epaper_display
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=$CURRENT_USER
Group=$CURRENT_USER
WorkingDirectory=$WORK_DIR
Environment="PATH=$WORK_DIR/venv/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
ExecStart=$VENV_PATH server_with_webui.py

# 重啟策略
Restart=always
RestartSec=10
StartLimitInterval=200
StartLimitBurst=5

# 資源限制 (Raspberry Pi 1 專用)
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
ReadWritePaths=$WORK_DIR/logs
ReadWritePaths=$WORK_DIR/uploads

[Install]
WantedBy=multi-user.target
EOF

# 複製服務檔案到系統目錄
echo "📋 安裝服務檔案..."
sudo cp /tmp/wifi_display.service /etc/systemd/system/
sudo chmod 644 /etc/systemd/system/wifi_display.service

# 重新載入 systemd
echo "🔄 重新載入 systemd..."
sudo systemctl daemon-reload

# 啟用服務
echo "✅ 啟用開機自動啟動..."
sudo systemctl enable wifi_display.service

# 啟動服務
echo "▶️  啟動服務..."
sudo systemctl start wifi_display.service

# 等待服務啟動
sleep 2

# 檢查服務狀態
echo ""
echo "📊 服務狀態:"
sudo systemctl status wifi_display.service --no-pager || true

echo ""
echo "====================================="
echo -e "${GREEN} ✅ 服務安裝完成！${NC}"
echo "====================================="
echo ""
echo "服務已設定為開機自動啟動"
echo ""
echo "常用管理命令:"
echo "  查看狀態: sudo systemctl status wifi_display"
echo "  啟動服務: sudo systemctl start wifi_display"
echo "  停止服務: sudo systemctl stop wifi_display"
echo "  重啟服務: sudo systemctl restart wifi_display"
echo "  查看日誌: sudo journalctl -u wifi_display -f"
echo "  停用自動啟動: sudo systemctl disable wifi_display"
echo ""
echo "Web UI: http://$(hostname -I | awk '{print $1}'):8080"
echo "WebSocket: ws://$(hostname -I | awk '{print $1}'):8266"
echo ""
