# 🚨 GxEPD2 庫安裝指南

## 問題診斷
編譯錯誤：`GxEPD2_BW.h: No such file or directory`

這是因為 `GDEQ0426T82_WeMos_D1_Mini.ino` 程式使用 GxEPD2 庫，但您的 Arduino IDE 中沒有安裝此庫。

## 解決方案 1：安裝 GxEPD2 庫 (推薦)

### 步驟 1：開啟 Arduino IDE
1. 啟動 Arduino IDE
2. 點選 `工具` → `管理程式庫...` 或使用快捷鍵 `Ctrl+Shift+I`

### 步驟 2：搜尋並安裝 GxEPD2
1. 在搜尋框中輸入：`GxEPD2`
2. 找到作者為 **Jean-Marc Zingg** 的 `GxEPD2` 庫
3. 點選 `安裝` 按鈕
4. 等待安裝完成

### 步驟 3：安裝相依庫
GxEPD2 需要 Adafruit GFX Library：
1. 在搜尋框中輸入：`Adafruit GFX`
2. 找到作者為 **Adafruit** 的 `Adafruit GFX Library`
3. 點選 `安裝` 按鈕

### 步驟 4：重新編譯
1. 關閉並重新開啟 Arduino IDE
2. 開啟 `GDEQ0426T82_WeMos_D1_Mini.ino`
3. 點選 `編譯` (檢查標誌)

## 解決方案 2：使用現有的驅動程式

如果您希望使用之前測試過的驅動程式，我可以為您重新創建 `GDEQ0426T82_Arduino` 資料夾。

## 推薦使用方案 1

GxEPD2 庫的優勢：
- ✅ 專門的 GDEQ0426T82 驅動程式
- ✅ 更好的效能和穩定性
- ✅ 支援快速部分更新
- ✅ 豐富的圖形功能
- ✅ 持續更新和維護

## 驗證安裝成功

安裝完成後，您應該能夠：
1. 編譯 `GDEQ0426T82_WeMos_D1_Mini.ino` 無錯誤
2. 看到 GxEPD2 庫的範例程式
3. 使用完整的圖形功能

## 如果安裝失敗

如果無法安裝 GxEPD2 庫，請告知我，我會幫您：
1. 重新創建基於原始驅動程式的版本
2. 或提供手動安裝 GxEPD2 的方法
