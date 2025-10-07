# Arduino CLI vs Arduino IDE 比較與燒錄指南

## 📊 編譯差異詳細分析

### **編譯產生的檔案**
```
build/
├── client_esp8266.ino.bin    # 二進制韌體檔案（燒錄用）
├── client_esp8266.ino.elf    # 執行檔格式（除錯用）
└── client_esp8266.ino.map    # 記憶體映射檔案（分析用）
```

### **檔案說明**
- **`.bin`**：純二進制韌體，用於燒錄到 ESP8266
- **`.elf`**：包含除錯符號的執行檔
- **`.map`**：記憶體佈局分析檔案

## 🚀 arduino-cli 燒錄流程

### **1. 檢查開發板連接**
```bash
arduino-cli board list
```
輸出示例：
```
Port Protocol Type              Board Name FQBN Core
COM5 serial   Serial Port (USB) Unknown
```

### **2. 編譯（已完成）**
```bash
arduino-cli compile --fqbn esp8266:esp8266:d1_mini --output-dir ./build client_esp8266.ino
```

### **3. 燒錄到開發板**
```bash
# 自動檢測並燒錄
arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port COM5 client_esp8266.ino

# 或者直接指定 .bin 檔案
arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port COM5 --input-dir ./build
```

### **4. 監控序列埠輸出**
```bash
arduino-cli monitor --port COM5 --config baudrate=115200
```

## ⚡ CLI 燒錄的優勢

### **1. 自動化腳本支援**
```bash
# 一鍵編譯和燒錄腳本
#!/bin/bash
echo "開始編譯..."
arduino-cli compile --fqbn esp8266:esp8266:d1_mini client_esp8266.ino

if [ $? -eq 0 ]; then
    echo "編譯成功，開始燒錄..."
    arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port COM5 client_esp8266.ino
    echo "燒錄完成！"
else
    echo "編譯失敗！"
    exit 1
fi
```

### **2. CI/CD 整合**
```yaml
# GitHub Actions 示例
- name: Compile and Upload
  run: |
    arduino-cli compile --fqbn esp8266:esp8266:d1_mini client_esp8266.ino
    arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port $PORT client_esp8266.ino
```

### **3. 批量處理**
```bash
# 編譯多個專案
for project in project1 project2 project3; do
    arduino-cli compile --fqbn esp8266:esp8266:d1_mini $project/$project.ino
done
```

## 🔧 進階燒錄選項

### **1. 燒錄參數自訂**
```bash
arduino-cli upload \
  --fqbn esp8266:esp8266:d1_mini \
  --port COM5 \
  --verify \
  client_esp8266.ino
```

### **2. 使用自訂燒錄工具**
```bash
# 使用 esptool.py 直接燒錄 .bin 檔案
esptool.py --chip esp8266 --port COM5 --baud 460800 write_flash --flash_size=detect 0 ./build/client_esp8266.ino.bin
```

### **3. 燒錄驗證**
```bash
# 燒錄後驗證
arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port COM5 --verify client_esp8266.ino
```

## 📈 性能比較

| **項目** | **arduino-cli** | **Arduino IDE** |
|---------|----------------|-----------------|
| **編譯時間** | ~15-30 秒 | ~20-40 秒 |
| **燒錄時間** | ~10-15 秒 | ~15-25 秒 |
| **記憶體使用** | 低 | 高（GUI） |
| **批量處理** | 支援 | 手動 |
| **腳本化** | 完美 | 不支援 |

## 🛠️ 實用命令集合

### **開發板管理**
```bash
# 更新開發板索引
arduino-cli core update-index

# 安裝 ESP8266 核心
arduino-cli core install esp8266:esp8266

# 列出已安裝的核心
arduino-cli core list
```

### **庫管理**
```bash
# 搜尋庫
arduino-cli lib search GxEPD2

# 安裝庫
arduino-cli lib install "GxEPD2"

# 列出已安裝的庫
arduino-cli lib list
```

### **完整工作流程**
```bash
# 1. 編譯
arduino-cli compile --fqbn esp8266:esp8266:d1_mini client_esp8266.ino

# 2. 燒錄
arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port COM5 client_esp8266.ino

# 3. 監控
arduino-cli monitor --port COM5 --config baudrate=115200
```

## 💡 最佳實踐建議

### **1. 開發階段**
- 使用 Arduino IDE：圖形介面方便除錯
- 使用 arduino-cli：快速編譯測試

### **2. 生產部署**
- 全部使用 arduino-cli：自動化、可重複
- 整合到 CI/CD pipeline

### **3. 團隊協作**
- arduino-cli：確保編譯環境一致性
- 版本控制：包含 arduino-cli 配置

---

**結論**：arduino-cli 完全可以替代 Arduino IDE 進行編譯和燒錄，特別適合自動化和生產環境。兩者產生的韌體檔案完全相同。