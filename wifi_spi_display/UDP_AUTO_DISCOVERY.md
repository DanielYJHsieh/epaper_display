# UDP 自動發現功能

## 🎯 功能說明

ESP8266 現在可以**自動發現** Server IP，無需手動修改 `config.h` 中的 `SERVER_HOST`！

---

## 🔧 工作原理

### Server 端（Python）
- 啟動後自動開啟 UDP 廣播
- 每 3 秒廣播一次訊息到 `port 8888`
- 廣播格式：`EPAPER_SERVER:192.168.0.43:8266`

### ESP8266 端
- WiFi 連線後監聽 UDP 廣播（port 8888）
- 最多等待 10 秒接收廣播訊息
- 解析 Server IP 並自動連線
- 如果超時則使用 `config.h` 的預設 IP

---

## 📋 使用方式

### 方法 1：自動發現（推薦 ⭐⭐⭐⭐⭐）

**無需修改 config.h！**

1. 啟動 Server：
```bash
python server_with_webui.py
```

2. Server 會顯示：
```
🔊 UDP 廣播啟動: port 8888, 間隔 3s
   廣播訊息: EPAPER_SERVER:192.168.0.43:8266

💡 ESP8266 設定:
   自動發現: UDP 廣播 (無需手動設定 IP)
   或手動設定: #define SERVER_HOST "192.168.0.43"
```

3. 上傳 ESP8266 韌體（無需修改 IP）

4. ESP8266 序列埠會顯示：
```
*** [3.5/5] 自動發現 Server... ***
*** 正在搜尋 Server (UDP 廣播)... ***
✓ 發現 Server!
  IP: 192.168.0.43
  Port: 8266
*** ✓ 使用自動發現的 Server IP ***
```

---

### 方法 2：手動設定（備用）

如果自動發現失敗，ESP8266 會自動使用 `config.h` 中的預設 IP：

```cpp
#define SERVER_HOST "192.168.0.43"  // 備用 IP
#define SERVER_PORT 8266
```

序列埠會顯示：
```
*** [3.5/5] 自動發現 Server... ***
*** 正在搜尋 Server (UDP 廣播)... ***
✗ 未找到 Server (超時)
*** ⚠ 使用預設 Server IP (config.h) ***
```

---

## 🎓 技術細節

### UDP 廣播協定

**訊息格式**：
```
EPAPER_SERVER:<IP>:<PORT>
```

**範例**：
```
EPAPER_SERVER:192.168.0.43:8266
```

### 程式碼實作

#### Server 端（Python）
```python
def _udp_broadcast_thread(self):
    """UDP 廣播執行緒"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    
    local_ip = get_local_ip()
    message = f"EPAPER_SERVER:{local_ip}:{self.port}".encode('utf-8')
    
    while self.broadcast_running:
        sock.sendto(message, ('<broadcast>', 8888))
        time.sleep(3)
```

#### ESP8266 端（Arduino）
```cpp
String discoverServer() {
  udp.begin(8888);  // 監聽廣播埠
  
  unsigned long startTime = millis();
  while (millis() - startTime < 10000) {  // 10 秒超時
    int packetSize = udp.parsePacket();
    if (packetSize) {
      char buffer[128];
      int len = udp.read(buffer, sizeof(buffer) - 1);
      buffer[len] = '\0';
      
      String message = String(buffer);
      if (message.startsWith("EPAPER_SERVER:")) {
        // 解析 IP
        int firstColon = message.indexOf(':');
        int secondColon = message.indexOf(':', firstColon + 1);
        String ip = message.substring(firstColon + 1, secondColon);
        return ip;
      }
    }
    delay(100);
  }
  return "";  // 超時
}
```

---

## 🔍 故障排除

### 問題 1：ESP8266 找不到 Server

**可能原因**：
- Server 未啟動
- 防火牆阻擋 UDP port 8888
- ESP8266 和 Server 不在同一網段

**解決方法**：
```bash
# Windows：允許 UDP 8888
netsh advfirewall firewall add rule name="ESP8266 UDP Discovery" dir=in action=allow protocol=UDP localport=8888

# Linux/Mac：檢查防火牆
sudo ufw allow 8888/udp
```

### 問題 2：Server 顯示廣播訊息但 ESP8266 收不到

**檢查網路**：
```bash
# 在 PC 上測試廣播
# Windows PowerShell
$udp = New-Object System.Net.Sockets.UdpClient
$udp.Client.SetSocketOption([System.Net.Sockets.SocketOptionLevel]::Socket, [System.Net.Sockets.SocketOptionName]::ReuseAddress, $true)
$udp.Client.Bind((New-Object System.Net.IPEndPoint([System.Net.IPAddress]::Any, 8888)))
$udp.Receive([ref]$null)

# Linux/Mac
nc -u -l 8888
```

### 問題 3：需要完全關閉自動發現

**修改 ESP8266 程式碼**：
```cpp
// 註解掉自動發現
// discoveredServerIP = discoverServer();

// 直接使用 config.h
String serverHost = String(SERVER_HOST);
```

---

## 📊 效能影響

### Server 端
- **記憶體**: +約 1KB（執行緒）
- **CPU**: 可忽略（每 3 秒一次廣播）
- **網路**: 約 30 bytes/3s ≈ 10 bytes/s

### ESP8266 端
- **記憶體**: +約 0.5KB（UDP 緩衝）
- **啟動時間**: +最多 10 秒（首次發現）
- **後續連線**: 無影響（只在啟動時執行）

---

## 🎉 優點

✅ **無需手動設定 IP**：Server IP 變更時 ESP8266 自動適應  
✅ **向後相容**：發現失敗自動使用 config.h 預設值  
✅ **跨平台**：PC、筆電、手機（Termux）都支援  
✅ **低開銷**：廣播訊息極小，不影響效能  
✅ **易於除錯**：序列埠清楚顯示發現過程  

---

## 📝 版本資訊

- **新增日期**: 2025-10-10
- **版本**: v1.8
- **相容性**: ESP8266 + Python 3.7+
- **測試環境**: Windows 11, WeMos D1 Mini, Python 3.13
