# 如何在本專案善用 GitHub Copilot 的 Agent Mode（中文）

本文件以 `wifi_spi_display` 專案為例，示範如何在 VS Code 中以 GitHub Copilot 的 Agent Mode（以下簡稱 Copilot Agent）進行「下指令、互動討論、邊改邊驗證」的開發方式，協助你加速從需求到可運作程式的整個流程。

---

## 你可以用 Copilot Agent 做什麼？
- 需求澄清與拆解：把自然語言需求轉成待辦清單（TODOs），逐步完成。
- 讀程式與找檔案：快速搜尋、定位本倉庫的相關檔案與符號。
- 寫程式與改檔：在正確檔案內新增/修改程式碼，避免手動複製貼上。
- 驗證與測試：幫你在終端機執行編譯/測試/腳本並總結結果。
- 文件與版本控制：同步撰寫 README/CHANGELOG，並協助產出 Git commit 訊息。

---

## 本專案常見工作流（以 wifi_spi_display 為例）

以下以「影像處理 Web UI 與 ESP8266 自動發現（UDP broadcast）功能」的開發旅程為例，展示如何指揮 Copilot Agent。

1) 啟動與對齊目標
- 直接描述要做的事與驗收標準，例如：
  -「請幫我把 Web UI 的重設選區改成保留原圖比例，畫面不足以白色填滿，且支援橫直向。」
- Agent 會：
  - 先整理需求成 TODO 清單
  - 指出會修改哪些檔案（如 `server/server_with_webui.py` 的內嵌 HTML/JS）

2) 探勘與定位
- 指令：
  -「搜尋 `resetCrop` 與 `sendImage` 定義，讀取附近 200 行。」
- Agent 會：
  - 快速打開對應區塊，摘要邏輯與潛在風險

3) 編輯與實作
- 指令：
  -「在 `sendImage()` 的橫向與直向分支加入維持比例的繪製流程，背景填白；避免拉伸。」
- Agent 會：
  - 在正確位置改動，並解釋關鍵數學與邊界情況（pillarbox/letterbox）

4) 驗證與執行
- 指令：
  -「幫我做語法檢查」或「啟動伺服器並提供關鍵日誌摘要」。
- 期望輸出：
  - Python 語法/執行結果、UDP 廣播訊息、WebSocket/HTTP 監聽埠資訊

5) 版本管理與文件
- 指令：
  -「以『fix(Web UI): Keep aspect ratio on reset』為主題建立 commit 並 push。」
  -「新建 `KEEP_ASPECT_RATIO.md`，寫上使用說明與示意。」

6) 硬體端（ESP8266）
- 指令：
  -「在 `client_esp8266.ino` 新增 UDP 自動發現，逾時 10 秒改 fallback 到 `config.h` 的 SERVER_HOST。」
  -「用 arduino-cli 編譯上傳到 COMx，並輸出序列埠日誌。」

---

## 快速開始：你可以直接複製的指令句型

- 尋找/閱讀
  -「幫我找出 `wifi_spi_display` 裡所有有 `server_with_webui.py` 字樣的檔案，讀取主要函式。」
  -「讀 `server/server_with_webui.py` 中包含 `resetCrop` 的區塊並總結。」
- 修改/新增
  -「在 `server_with_webui.py` 的 Web UI JS 內，為 `resetCrop()` 實作保持比例並置中。」
  -「新增 `docs/UDP_AUTO_DISCOVERY.md`，撰寫 ESP8266 自動發現規格與封包格式。」
- 執行/驗證
  -「在 Windows PowerShell 下，於 `server` 目錄執行 `python -m py_compile server_with_webui.py`，並回報錯誤。」
  -「啟動伺服器並顯示本機 IP、UDP 廣播內容與監聽埠。」
- Git 操作
  -「以『feat: UDP auto discovery』建 commit，描述動機、做法、測試，然後 push。」
- 生成文件
  -「建立 `KEEP_ASPECT_RATIO.md`，包括：功能說明、使用步驟、注意事項、常見問題。」

小訣竅：用「請直接幫我修改檔案並附上變更摘要」來節省往返；用「請先列 TODO 再開始」能把複雜任務拆解清楚。

---

## 實戰演練 A：保持比例重設選區 + 送圖

需求：
- Web UI 的「重設選區」需保留原圖比例；送圖時不得拉伸，空白處填白，橫直向皆適用。

你可以這樣和 Agent 合作：
1)「列出會改動的函式與檔案」→ 期望回應 `resetCrop()`、`sendImage()` 皆在 `server_with_webui.py` 的內嵌 JS。
2)「請在橫向/直向分支加入白底置中，依比例縮放計算 drawWidth/drawHeight/drawX/drawY。」
3)「跑語法檢查與最小化啟動測試，回報關鍵日誌。」
4)「撰寫 `KEEP_ASPECT_RATIO.md` 說明差異（Auto-Fit vs Reset）並提交。」

驗收要點：
- 任何寬高比都應正確 letterbox/pillarbox；
- Portrait 分支須先旋轉 90°，再按比例置中繪製；
- 程式碼通過 py_compile；
- 實測畫面居中且無拉伸。

---

## 實戰演練 B：UDP 廣播與 ESP8266 自動發現

需求：
- Server 以固定間隔在 UDP 8888 廣播：`EPAPER_SERVER:<IP>:<PORT>`；
- ESP8266 啟動後收聽 10 秒；成功解析即連線，否則 fallback 到 `config.h` 的預設主機。

互動腳本：
1)「在 `server_with_webui.py` 建立廣播線程並啟動/停止函式。」
2)「在 `client_esp8266.ino` 新增 `discoverServer()` 並更新 `setup()` 流程。」
3)「編譯與上傳；序列埠監控顯示找到的 IP 與連線結果。」
4)「更新 `UDP_AUTO_DISCOVERY.md` 與 `README.md`，包含故障排除。」

驗收要點：
- 伺服器日誌可見廣播內容；
- ESP8266 能在 10 秒內連到正確 IP；
- fallback 情境可重現；
- 文件涵蓋封包格式、埠與網段注意事項。

---

## 高效互動技巧

- 以結果為導向描述：「幫我完成 X，並附最小測試。」
- 要 Agent 在每 3–5 個動作後彙報進度與下一步。
- 要求建立 TODO 清單並維持『唯一進行中』的項目，利於追蹤。
- 將修改集中成『小批次、低風險』，便於回滾與審查。
- 讓 Agent 執行編譯/測試，並在失敗時嘗試 1–2 次自動修正。
- 文件與程式碼一起演進：每次功能變更同步更新 README/CHANGELOG。

---

## 常見問題（FAQ）

- Q: Agent 誤判路徑或檔名？
  - A: 讓它用「搜尋」再「讀檔案」取得上下文，並在修改前重複檢查路徑。
- Q: Windows PowerShell 指令格式？
  - A: 在同一行串接命令請用 `;`，避免使用 `&&`。
- Q: 無法連線或序列埠號不確定？
  - A: 指派 Agent 先列出可用連線，或讓它顯示 `arduino-cli board list`；需要時請你手動確認 COM 連接。
- Q: 大量輸出導致訊息過長？
  - A: 要求 Agent 使用過濾器（如 `| select -First 100`）或摘要重點。

---

## 參考：本專案曾與 Agent 合作完成的里程碑
- 開機優化（移除上電清屏/測試圖）
- Web UI（上傳/貼上、Canvas 裁切、800×480 等比框）
- 橫直向切換與 90° 旋轉
- UDP 廣播與 ESP8266 自動發現
- Reset 選區保持比例並白底置中
- 文件化：README、RELEASE_NOTES、KEEP_ASPECT_RATIO、UDP_AUTO_DISCOVERY

---

## 後續建議
- 增加 mDNS 作為備援自動發現機制；
- UI 增加「填滿（裁切）」與「保留比例（留白）」模式切換；
- 加入自動化小型測試（影像比例集）與壓力測試腳本；
- 以 Agent 管理長期 TODO 看板，週期性檢視技術債。

---

撰寫/維護：以本倉庫的 Copilot Agent 工作流程為基礎，持續更新。
