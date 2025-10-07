# Reset 後第一張圖無法顯示 - 問題分析

## 問題描述
Reset 後發送第一張圖片，圖片無法顯示或顯示不正確。

## 問題分析

### 初始化流程
```cpp
void setup() {
  // 1. 序列埠初始化
  // 2. 記憶體整理
  // 3. WiFi 連接
  // 4. WebSocket 設定
  
  // 5. 顯示器初始化
  display.init(SERIAL_BAUD);
  display.setFullWindow();
  display.clearScreen();
  display.refresh(true);  // 完整刷新
}
```

### 第一次更新流程
```cpp
void displayFrame(const uint8_t* frame) {
  display.setPartialWindow(DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, 
                           DISPLAY_WIDTH, DISPLAY_HEIGHT);
  display.writeImage(frame, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 
                     false, false, true);
  display.refresh(false);  // 快速部分更新
}
```

## 根本原因分析

### 可能原因 1: 窗口模式衝突 ⚠️
**問題**:
- `setup()` 中使用 `setFullWindow()` + `refresh(true)`
- `displayFrame()` 中使用 `setPartialWindow()` + `refresh(false)`
- **窗口模式從 Full → Partial 可能需要中間步驟**

**GxEPD2 行為**:
```
setup() 執行:
  setFullWindow()      → display._window_x1 = 0, _window_y1 = 0
                       → display._window_x2 = 800, _window_y2 = 480
  clearScreen()        → 清除 800×480 區域
  refresh(true)        → 完整刷新整個螢幕

第一次 displayFrame():
  setPartialWindow()   → display._window_x1 = 200, _window_y1 = 120
                       → display._window_x2 = 600, _window_y2 = 360
  writeImage()         → 寫入 400×240 資料到部分窗口
  refresh(false)       → 快速部分更新
  
問題: GxEPD2 可能沒有正確從 FullWindow 切換到 PartialWindow
```

### 可能原因 2: PowerOn 狀態未同步 ⚠️
**問題**:
- `refresh(true)` 可能會執行 `powerOff()` 進入低功耗模式
- 第一次 `refresh(false)` 需要先 `powerOn()`
- **GxEPD2 內部狀態可能未正確管理**

**時序問題**:
```
setup():
  display.init()       → powerOn()
  refresh(true)        → _Update_Full + _Update_Part + powerOff() ?
  
第一次 displayFrame():
  refresh(false)       → 如果 display 已 powerOff，需要先 powerOn()
                       → 但 refresh(false) 可能跳過此步驟
```

### 可能原因 3: 部分更新需要基準畫面 ⚠️
**問題**:
- `clearScreen()` 清除成白色
- 部分更新 (`refresh(false)`) 是基於**當前螢幕內容**的差異更新
- **清除後的白色可能與第一張圖片的白色背景無法區分**

**更新機制**:
```
E-Paper 部分更新原理:
  - 比較新舊像素差異
  - 只更新有變化的像素
  - 如果都是白色 → 無變化 → 不更新

clearScreen() 後:
  - 螢幕: 全白 (800×480)
  - 第一張圖: 白底黑字 (400×240)
  - 問題: 白色區域可能不更新
```

### 可能原因 4: 初始化時序問題 🎯 **最可能**
**問題**:
- `display.init()` 後立即執行 `refresh(true)`
- **可能在 WebSocket 連接建立前就進入低功耗模式**
- 收到第一張圖時，顯示器狀態異常

**時序分析**:
```
目前流程:
  1. display.init()           ← 初始化硬體
  2. setFullWindow()          ← 設定全窗口
  3. clearScreen()            ← 清除螢幕
  4. refresh(true)            ← 完整刷新 (60秒)
     ↓
  5. 等待 WebSocket 圖片
     ↓
  6. displayFrame()
     ↓
  7. setPartialWindow()       ← 切換到部分窗口
  8. refresh(false)           ← 快速更新

問題: 第 4 步到第 7 步之間可能間隔很久
      顯示器可能進入深度低功耗模式
```

## 解決方案

### 方案 1: 移除 setup() 中的清屏 ❌ 不推薦
```cpp
void setup() {
  display.init(SERIAL_BAUD);
  // 移除 clearScreen() 和 refresh()
}
```
**缺點**: 啟動時可能有殘影

### 方案 2: 在 setup() 中使用 PartialWindow ⚠️ 可嘗試
```cpp
void setup() {
  display.init(SERIAL_BAUD);
  
  // 使用與 displayFrame 相同的窗口模式
  display.setPartialWindow(DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, 
                           DISPLAY_WIDTH, DISPLAY_HEIGHT);
  display.clearScreen();
  display.refresh(true);  // 完整刷新部分窗口
}
```
**優點**: 窗口模式一致

### 方案 3: 第一次更新使用完整刷新 ✅ 推薦
```cpp
void displayFrame(const uint8_t* frame) {
  static bool firstUpdate = true;
  
  display.setPartialWindow(DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, 
                           DISPLAY_WIDTH, DISPLAY_HEIGHT);
  display.writeImage(frame, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 
                     false, false, true);
  
  if (firstUpdate) {
    Serial.println(F("*** 第一次更新：使用完整刷新 ***"));
    display.refresh(true);   // 完整刷新
    firstUpdate = false;
  } else {
    display.refresh(false);  // 快速部分更新
  }
}
```
**優點**: 
- 確保第一次正確顯示
- 後續保持快速
- 窗口模式一致

### 方案 4: 延遲清屏到第一次更新前 ⚠️ 複雜
```cpp
void setup() {
  display.init(SERIAL_BAUD);
  // 不清屏
}

void displayFrame(const uint8_t* frame) {
  static bool displayCleared = false;
  
  if (!displayCleared) {
    Serial.println(F("*** 第一次更新前清屏 ***"));
    display.setPartialWindow(DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, 
                             DISPLAY_WIDTH, DISPLAY_HEIGHT);
    display.clearScreen();
    display.refresh(true);
    displayCleared = true;
  }
  
  // 正常更新
  display.setPartialWindow(DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, 
                           DISPLAY_WIDTH, DISPLAY_HEIGHT);
  display.writeImage(frame, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 
                     false, false, true);
  display.refresh(false);
}
```
**缺點**: 複雜，第一次更新時間加倍

### 方案 5: 清屏後立即設置回 PartialWindow ✅ 最佳
```cpp
void setup() {
  display.init(SERIAL_BAUD);
  
  // 清除整個螢幕
  display.setFullWindow();
  display.clearScreen();
  display.refresh(true);
  
  // 清屏後立即切回 PartialWindow 模式
  display.setPartialWindow(DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, 
                           DISPLAY_WIDTH, DISPLAY_HEIGHT);
  Serial.println(F("*** 螢幕清除完成，已切換到部分窗口模式 ***"));
}
```
**優點**:
- 啟動時清除殘影
- 立即切換到正確窗口模式
- 第一次更新直接使用快速模式
- 簡單有效

## 測試驗證

### 測試步驟
1. 按下 Reset 鍵
2. 觀察 Serial Monitor 輸出
3. 發送第一張圖片
4. 檢查是否正確顯示
5. 發送第二張圖片
6. 確認仍然正常

### 預期結果
```
[Reset]
*** 初始化電子紙... ***
*** 清除整個螢幕... ***
*** 螢幕清除完成，已切換到部分窗口模式 ***
[等待圖片]
更新顯示中...
_Update_Part : xxxxx
顯示更新耗時: 18000 ms
[第一張圖正確顯示] ✅
```

## 建議採用的方案

**方案 5 - 清屏後立即切回 PartialWindow**

理由：
1. ✅ 啟動時清除殘影（用戶需求）
2. ✅ 窗口模式一致（避免 Full→Partial 衝突）
3. ✅ 第一次更新保持快速（~18 秒）
4. ✅ 實作簡單，不增加複雜度
5. ✅ 所有更新時間一致（用戶體驗好）

## GxEPD2 窗口模式說明

### setFullWindow()
```cpp
void setFullWindow() {
  _window_x1 = 0;
  _window_y1 = 0;
  _window_x2 = PHYSICAL_WIDTH;   // 800
  _window_y2 = PHYSICAL_HEIGHT;  // 480
}
```

### setPartialWindow(x, y, w, h)
```cpp
void setPartialWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  _window_x1 = x;        // 200
  _window_y1 = y;        // 120
  _window_x2 = x + w;    // 600
  _window_y2 = y + h;    // 360
}
```

### refresh(bool partial_update_mode)
- `refresh(true)`: 完整刷新（清除殘影，~60 秒）
- `refresh(false)`: 快速部分更新（基於當前內容差異，~18 秒）

## 相關問題

### 為何第二張圖正常？
因為第一張圖已經建立了 PartialWindow 狀態：
```
第一張圖: setPartialWindow() → 狀態建立
第二張圖: setPartialWindow() → 狀態已存在 → 正常
```

### 為何需要 refresh(true) 清屏？
- E-Paper 是**反射式**顯示器
- 需要完整刷新才能完全清除像素
- `clearScreen()` 只是填充白色到緩衝區
- `refresh(true)` 才會真正驅動像素變化

## 實作修正

修改 setup() 函數，在清屏後立即切換到 PartialWindow 模式。
