# 🐛 Coordinate Fix v1.6.4 - Tile Display Position Correction

**Date**: 2024-10-10  
**Version**: v1.6.4  
**Issue**: All 4 tiles displayed in top-left corner  
**Status**: ✅ FIXED

---

## 📋 Problem Summary

After fixing the critical reset() bug in v1.6.3, the zero-copy architecture worked correctly without crashes. However, all 4 tiles were being displayed in the top-left corner (0,0) instead of their correct positions.

---

## 🔍 Root Cause Analysis

### Initial Symptoms
```
📍 分區更新: 左上 (索引=0, 座標=(0,0), 尺寸=400×240)
📍 分區更新: 右上 (索引=1, 座標=(400,0), 尺寸=400×240)
📍 分區更新: 左下 (索引=2, 座標=(0,240), 尺寸=400×240)
📍 分區更新: 右上 (索引=3, 座標=(400,240), 尺寸=400×240)

Result: All tiles appeared at (0,0) ❌
```

### Investigation Steps

**Attempt 1**: Removed `setFullWindow()` call
- **Theory**: `setFullWindow()` before `setPartialWindow()` might reset coordinate system
- **Result**: Still all tiles at top-left ❌

**Attempt 2**: Used `firstPage()`/`nextPage()` loop with relative coordinates
```cpp
display.setPartialWindow(tile_x, tile_y, TILE_WIDTH, TILE_HEIGHT);
display.firstPage();
do {
  display.writeImage(tileBuffer, 0, 0, TILE_WIDTH, TILE_HEIGHT, ...);
} while (display.nextPage());
```
- **Theory**: Maybe writeImage coordinates are relative to PartialWindow
- **Result**: System hung/froze in nextPage() loop ❌

**Attempt 3**: Used absolute coordinates for writeImage ✅
```cpp
display.setPartialWindow(tile_x, tile_y, TILE_WIDTH, TILE_HEIGHT);
display.writeImage(tileBuffer, tile_x, tile_y, TILE_WIDTH, TILE_HEIGHT, ...);
display.refresh(false);
```
- **Theory**: Both setPartialWindow and writeImage use absolute screen coordinates
- **Result**: SUCCESS! All tiles display in correct positions ✅

---

## 🎯 The Solution

### GxEPD2 Coordinate System Understanding

**Key Discovery**: GxEPD2's `writeImage()` uses **absolute screen coordinates**, not relative to the PartialWindow.

```cpp
// WRONG (all tiles appear at 0,0)
display.setPartialWindow(tile_x, tile_y, TILE_WIDTH, TILE_HEIGHT);
display.writeImage(tileBuffer, 0, 0, TILE_WIDTH, TILE_HEIGHT, ...);

// CORRECT (tiles appear at correct positions)
display.setPartialWindow(tile_x, tile_y, TILE_WIDTH, TILE_HEIGHT);
display.writeImage(tileBuffer, tile_x, tile_y, TILE_WIDTH, TILE_HEIGHT, ...);
```

### Fixed Code

**Location**: `client_esp8266/client_esp8266.ino`, `handleTileUpdate()` function

```cpp
// 設置部分窗口到指定的分區座標
// 注意：setPartialWindow 的座標是絕對螢幕座標
display.setPartialWindow(tile_x, tile_y, TILE_WIDTH, TILE_HEIGHT);

Serial.println(F("✓ setPartialWindow 完成"));
yield();

// writeImage 的座標也是絕對螢幕座標（不是相對於 PartialWindow）
// 所以這裡要使用 tile_x, tile_y 而不是 0, 0
display.writeImage(tileBuffer, tile_x, tile_y, TILE_WIDTH, TILE_HEIGHT, false, false, true);

Serial.println(F("✓ writeImage 完成（緩衝區保留供下次使用）"));
yield();

// 執行顯示刷新（這個操作需要約 18 秒）
Serial.println(F("🔄 開始 refresh..."));
display.refresh(false);  // 快速部分更新
Serial.println(F("✓ refresh 完成"));
```

---

## 📊 Tile Coordinate Mapping

| Tile Index | Name | Coordinates | writeImage Parameters |
|------------|------|-------------|-----------------------|
| 0 | 左上 (Top-Left) | (0, 0) | `writeImage(buffer, 0, 0, 400, 240, ...)` |
| 1 | 右上 (Top-Right) | (400, 0) | `writeImage(buffer, 400, 0, 400, 240, ...)` |
| 2 | 左下 (Bottom-Left) | (0, 240) | `writeImage(buffer, 0, 240, 400, 240, ...)` |
| 3 | 右下 (Bottom-Right) | (400, 240) | `writeImage(buffer, 400, 240, 400, 240, ...)` |

### Calculation Formula
```cpp
// Correct formula (produces absolute screen coordinates)
uint16_t tile_x = (tileIndex % 2) * TILE_WIDTH;   // 0 or 400
uint16_t tile_y = (tileIndex / 2) * TILE_HEIGHT;  // 0 or 240
```

---

## 🧪 Validation

### Test Results
- ✅ Tile 0 (Top-Left): Displays at (0, 0)
- ✅ Tile 1 (Top-Right): Displays at (400, 0)
- ✅ Tile 2 (Bottom-Left): Displays at (0, 240)
- ✅ Tile 3 (Bottom-Right): Displays at (400, 240)
- ✅ No system hangs or freezes
- ✅ Memory remains stable at ~23KB free
- ✅ Zero-copy architecture still working correctly

### Compilation Stats
```
RAM Usage: 30,484 / 80,192 bytes (38%)
IRAM Usage: 60,737 / 65,536 bytes (92%)
Flash Usage: 379,752 / 1,048,576 bytes (36%)
```

---

## 📝 Lessons Learned

### GxEPD2 API Behavior

1. **setPartialWindow(x, y, w, h)**
   - Uses absolute screen coordinates
   - Defines the update region
   - Does NOT change the coordinate origin

2. **writeImage(buffer, x, y, w, h, ...)**
   - Uses absolute screen coordinates (NOT relative to PartialWindow)
   - Must match the region defined by setPartialWindow
   - Buffer content should match the region size

3. **firstPage()/nextPage()**
   - Can cause hangs in partial update mode
   - Not recommended for simple partial window updates
   - Better to use direct writeImage() + refresh()

### Debugging Strategy

1. ✅ Start with simplest approach (direct API calls)
2. ✅ Add extensive Serial logging for visibility
3. ✅ Test coordinate calculations independently
4. ✅ Avoid complex patterns (like page loops) unless necessary
5. ✅ Understand library behavior through experimentation

---

## 🔄 Version History

### v1.6.4 (2024-10-10) - **Current**
- 🐛 **FIX**: Corrected writeImage() to use absolute coordinates
- 🐛 **FIX**: Removed problematic firstPage/nextPage loop
- ✅ All 4 tiles now display in correct positions
- ✅ Added detailed Serial logging for debugging

### v1.6.3 (2024-10-09)
- 🐛 **CRITICAL FIX**: Fixed reset() memory corruption
- ✅ Zero-copy architecture functional
- ⚠️ Coordinate issue: all tiles at (0,0)

### v1.6.2 (2024-10-08)
- 🔧 Disabled compression to avoid fragmentation
- ⚠️ 24KB peak memory usage

### v1.6.1 (2024-10-07)
- 💾 Pre-allocated tileBuffer
- ⚠️ Memory fragmentation with compression

### v1.6.0 (2024-10-06)
- 🎨 Initial tiled display implementation
- ⚠️ Multiple bugs discovered

---

## 📊 Performance Summary

| Metric | Status |
|--------|--------|
| Display Accuracy | ✅ All 4 tiles correct positions |
| Memory Stability | ✅ ~23KB free throughout |
| System Stability | ✅ No hangs or crashes |
| Update Time | ✅ ~18s per tile, ~80s total |
| Zero-Copy | ✅ Working correctly |

---

## 🎯 Impact

This fix completes the tiled display feature:
- **v1.6.0**: Feature implementation (buggy)
- **v1.6.1-v1.6.2**: Memory optimization journey
- **v1.6.3**: Zero-copy architecture (memory optimal, but coordinate bug)
- **v1.6.4**: Coordinate fix (fully functional!) ✅

Now the system can display full 800×480 images using 4 sequential tile updates with optimal memory usage (12KB) and correct positioning!

---

## ✅ Status

- [x] Coordinate bug identified
- [x] GxEPD2 API behavior understood
- [x] Fix implemented (absolute coordinates)
- [x] Code compiled successfully
- [x] Firmware uploaded to ESP8266
- [x] Hardware tested and verified
- [x] All 4 tiles display correctly

---

**Impact**: This fix makes the tiled display feature fully operational. Users can now display full-screen 800×480 images on GDEQ0426T82 e-paper displays using ESP8266 with optimal memory usage.
