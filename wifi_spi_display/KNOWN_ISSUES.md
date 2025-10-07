# 已知問題 (Known Issues)

## v1.5.0 已知問題

### 1. Reset 後測試畫面未顯示 ⚠️

**問題描述**:
- Reset 後應該顯示 20×20 黑白棋盤格測試圖案
- 實際上測試圖案沒有出現

**重現步驟**:
1. 按下 ESP8266 板子上的 Reset 鍵
2. 觀察螢幕
3. 預期：清屏 → 顯示棋盤格測試圖案
4. 實際：清屏 → 螢幕保持白色（測試圖案未出現）

**Serial Monitor 輸出**:
```
*** [5/5] 初始化電子紙顯示器... ***
*** 清除整個螢幕（上電初始化）... ***
*** 螢幕清除完成，已切換到部分窗口模式 ***
*** 顯示測試畫面... ***
*** 測試畫面顯示完成 ***
初始化完成！
```

**可能原因分析**:

#### 原因 1: refresh(false) 無法顯示測試圖案
清屏後立即使用 `refresh(false)` 可能無法正確顯示新內容：
```cpp
display.clearScreen();         // 螢幕全白
display.refresh(true);         // 完整刷新
// ... 立即顯示測試圖案
display.writeImage(testBuffer, ...);
display.refresh(false);        // 快速部分更新
// 問題：E-Paper 可能需要一個基準畫面
```

#### 原因 2: 測試緩衝區初始化問題
測試緩衝區可能未正確初始化：
```cpp
uint8_t* testBuffer = (uint8_t*)malloc(DISPLAY_BUFFER_SIZE);
// 問題：malloc 分配的記憶體內容是隨機的
// 需要先清零：memset(testBuffer, 0xFF, DISPLAY_BUFFER_SIZE);
```

#### 原因 3: 棋盤格算法錯誤
位元操作可能有誤：
```cpp
if (isBlack) {
  testBuffer[byteIndex] &= ~(1 << bitIndex);  // 黑色
} else {
  testBuffer[byteIndex] |= (1 << bitIndex);   // 白色
}
// 問題：testBuffer 未初始化，&= 和 |= 操作結果不可預測
```

#### 原因 4: GxEPD2 需要完整刷新顯示新內容
在清屏後立即顯示測試圖案可能需要使用 `refresh(true)`：
```cpp
display.refresh(true);   // 清屏完成
// 時間過短，顯示器未進入穩定狀態
display.refresh(false);  // 測試圖案
// 可能需要 refresh(true) 才能正確顯示
```

**臨時解決方案**:

移除測試圖案功能，因為：
1. 測試圖案會增加啟動時間（額外 ~18 秒）
2. 第一張實際圖片可以作為測試驗證
3. 避免複雜的初始化問題

**未來修正方案**:

#### 方案 1: 修正測試緩衝區初始化
```cpp
void displayTestPattern() {
  uint8_t* testBuffer = (uint8_t*)malloc(DISPLAY_BUFFER_SIZE);
  if (!testBuffer) return;
  
  // 先清零緩衝區
  memset(testBuffer, 0xFF, DISPLAY_BUFFER_SIZE);  // 全白
  
  // 然後繪製黑色方塊
  for (int y = 0; y < DISPLAY_HEIGHT; y++) {
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
      if (((x / 20) + (y / 20)) % 2 == 0) {
        int byteIndex = y * (DISPLAY_WIDTH / 8) + (x / 8);
        int bitIndex = 7 - (x % 8);
        testBuffer[byteIndex] &= ~(1 << bitIndex);  // 設為黑色
      }
    }
  }
  
  // 使用完整刷新顯示
  display.writeImage(testBuffer, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 
                     false, false, true);
  display.refresh(true);  // 使用完整刷新
  
  free(testBuffer);
}
```

#### 方案 2: 簡化測試圖案
```cpp
void displayTestPattern() {
  uint8_t* testBuffer = (uint8_t*)malloc(DISPLAY_BUFFER_SIZE);
  if (!testBuffer) return;
  
  // 簡單的垂直條紋
  memset(testBuffer, 0xAA, DISPLAY_BUFFER_SIZE);  // 10101010 pattern
  
  display.writeImage(testBuffer, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 
                     false, false, true);
  display.refresh(true);
  
  free(testBuffer);
}
```

#### 方案 3: 延遲顯示測試圖案
```cpp
void setup() {
  // ... 初始化 ...
  
  display.clearScreen();
  display.refresh(true);
  
  // 等待 2 秒讓顯示器穩定
  delay(2000);
  
  // 然後顯示測試圖案
  displayTestPattern();
}
```

**影響範圍**:
- 🟡 輕微：不影響實際圖片顯示功能
- 🟡 輕微：僅影響啟動時的視覺反饋
- ✅ 實際圖片顯示正常工作

**優先級**: 低
- 測試圖案是錦上添花的功能
- 不影響核心功能（圖片顯示）
- 可以在未來版本中修正

**建議**:
暫時移除測試圖案功能，待深入研究 GxEPD2 行為後再實作。

---

## 其他注意事項

### E-Paper 部分更新限制
- 部分更新基於當前螢幕內容的差異
- 連續多次部分更新可能累積殘影
- 建議每 20-50 次更新後執行一次完整刷新

### 記憶體分配
- ESP8266 可用記憶體 ~50KB
- 顯示緩衝 12KB
- 記憶體碎片化可能影響分配成功率

### WebSocket 穩定性
- 更新時間過長會導致 WebSocket 斷線
- 建議單次更新時間 < 30 秒
- 目前更新時間 ~18 秒，安全範圍內

---

**最後更新**: 2025-10-08  
**版本**: v1.5.0
