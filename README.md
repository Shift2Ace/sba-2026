# 💊 Wi-Fi Medicine Dispenser System

This project is a smart medicine dispenser powered by an ESP32 microcontroller. It features a web-based dashboard for scheduling medicine dispensing events, an LCD for real-time display, and RTC (Real-Time Clock) for accurate timekeeping. The system supports Wi-Fi configuration via WiFiManager and allows users to manage events through a browser interface.

---

## 📁 Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Libraries Used](#libraries-used)
- [Program Guidelines](#program-guidelines)
- [User Guidelines](#user-guidelines)
- [Web Interface Overview](#web-interface-overview)
- [Future Improvements](#future-improvements)

---

## ✅ Features

- Real-time clock with DS1307
- LCD display (I2C 16x2)
- Web-based dashboard for event management
- Wi-Fi configuration via captive portal
- Supports daily, weekday-specific, and every-N-day schedules
- Event-based medicine dispensing
- Notification system (e.g., LED alert)
- Persistent event storage (in-memory for now)

---

## 🔧 Hardware Requirements

- ESP32 board
- DS1307 RTC module
- I2C 16x2 LCD display
- Push button (for Wi-Fi reset)
- LEDs or actuators for medicine dispensing
- Optional: Servo motors or solenoids for physical dispensing

---

## 📚 Libraries Used

Make sure to install these libraries via the Arduino Library Manager:

- `WiFi.h`
- `WiFiManager.h`
- `WebServer.h`
- `ArduinoJson.h`
- `RTClib.h`
- `Wire.h`
- `LiquidCrystal_I2C.h`

---

## 🧠 Program Guidelines

### 1. Wi-Fi Setup
- On first boot or when the reset button is pressed, the ESP32 enters AP mode (`MedicineDispenserAP`).
- Connect to this network and configure Wi-Fi credentials via the captive portal.

### 2. RTC Initialization
- The RTC is initialized and synced with the compile time on first boot.
- It keeps track of time even when the ESP32 is restarted.

### 3. Web Server
- Runs on port 80.
- Routes:
  - `/` – Dashboard
  - `/addEvent` – Add new event
  - `/submit` – Submit event data
  - `/events` – Get all events (JSON)
  - `/delete?id=ID` – Delete event by ID

### 4. Event Scheduling
- Events are stored in a `JsonArray` in memory.
- Supports:
  - Every Day: Repeats daily
  - Weekdays: Repeats on selected weekdays
  - Every (N) Day: Repeats every N days

### 5. Medicine Dispensing
- When the current time matches an event:
  - Dispenses the specified amount from the selected storage
  - Triggers a notification (e.g., LED)
  - Waits for user acknowledgment (simulated with delay)

---

## 👤 User Guidelines

### Initial Setup
1. Power on the device.
2. If no Wi-Fi is configured, connect to `MedicineDispenserAP`.
3. Follow the portal to connect the device to your Wi-Fi.

### Accessing the Dashboard
- Open a browser and go to the IP address shown on the LCD screen.
- You’ll see the dashboard with current time and event list.

### Adding an Event
1. Click "Add Event".
2. Choose a repeat method:
   - Every Day
   - Weekdays (select specific days)
   - Every (N) Day
3. Set the time, storage ID, and amount.
4. Click Submit.

### Deleting an Event
- On the dashboard, click Delete next to the event you want to remove.

### Syncing Events
- Click "Sync Events" to refresh the event list manually.

---

## 🖥️ Web Interface Overview

### Dashboard
- Displays current time
- Lists all scheduled events
- Allows deletion of events

### Add Event Page
- Form to create new events
- Dynamic controls based on repeat type
- Validation for input fields

---

## 🚀 Future Improvements

- Persistent storage using SPIFFS or EEPROM
- Authentication for web access
- Email or mobile notifications
- Integration with mobile apps
- Real-time feedback on medicine dispensing status

---

# 💊 Wi-Fi 藥品分配系統（繁體中文）

本專案是一個由 ESP32 微控制器驅動的智慧藥品分配器。它具備網頁儀表板以排程分配事件、LCD 顯示即時時間，以及 RTC（實時時鐘）以確保時間準確。系統支援透過 WiFiManager 進行 Wi-Fi 設定，並允許使用者透過瀏覽器介面管理事件。

---

## 📁 目錄

- [功能特色](#功能特色)
- [硬體需求](#硬體需求)
- [使用的函式庫](#使用的函式庫)
- [程式指南](#程式指南)
- [使用者指南](#使用者指南)
- [網頁介面概覽](#網頁介面概覽)
- [未來改進](#未來改進)

---

## ✅ 功能特色

- 使用 DS1307 的實時時鐘
- LCD 顯示器（I2C 16x2）
- 網頁儀表板管理事件
- 透過 WiFiManager 設定 Wi-Fi
- 支援每日、特定星期日、每 N 天排程
- 事件驅動的藥品分配
- 通知系統（例如 LED 提示）
- 事件儲存（目前為記憶體儲存）

---

## 🔧 硬體需求

- ESP32 開發板
- DS1307 RTC 模組
- I2C 16x2 LCD 顯示器
- 按鈕（用於重設 Wi-Fi）
- LED 或執行器以分配藥品
- 選用：伺服馬達或電磁閥以實體分配藥品

---

## 📚 使用的函式庫

請透過 Arduino Library Manager 安裝以下函式庫：

- `WiFi.h`
- `WiFiManager.h`
- `WebServer.h`
- `ArduinoJson.h`
- `RTClib.h`
- `Wire.h`
- `LiquidCrystal_I2C.h`

---

## 🧠 程式指南

### 1. **Wi-Fi 設定**
- 首次啟動或按下重設按鈕時，ESP32 會進入 AP 模式（SSID：MedicineDispenserAP）。
- 連接此網路並透過入口網站設定 Wi-Fi。

### 2. **RTC 初始化**
- RTC 會在首次啟動時與編譯時間同步。
- 即使 ESP32 重新啟動也能保持時間。

### 3. **網頁伺服器**
- 執行於 port 80。
- 路由：
  - `/` – 儀表板
  - `/addEvent` – 新增事件
  - `/submit` – 提交事件資料
  - `/events` – 取得所有事件（JSON）
  - `/delete?id=ID` – 依 ID 刪除事件

### 4. **事件排程**
- 事件儲存在記憶體中的 `JsonArray`。
- 支援：
  - **每日**：每天重複
  - **星期日**：選擇特定星期日重複
  - **每 N 天**：每 N 天重複

### 5. **藥品分配**
- 當目前時間與事件時間相符時：
  - 從指定儲存槽分配指定數量
  - 觸發通知（例如 LED）
  - 等待使用者確認（以 delay 模擬）

---

## 👤 使用者指南

### 🔌 初始設定
1. 啟動裝置。
2. 若尚未設定 Wi-Fi，請連接至 `MedicineDispenserAP`。
3. 依照入口網站設定 Wi-Fi。

### 🌐 存取儀表板
- 開啟瀏覽器並輸入 LCD 顯示的 IP 位址。
- 可看到儀表板、目前時間與事件列表。

### ➕ 新增事件
1. 點選 **"Add Event"**。
2. 選擇重複方式：
   - **每日**
   - **星期日**（選擇特定日）
   - **每 N 天**
3. 設定時間、儲存槽 ID 與數量。
4. 點選 **Submit**。

### 🗑️ 刪除事件
- 在儀表板中點選事件旁的 **Delete**。

### 🔁 同步事件
- 點選 **"Sync Events"** 手動更新事件列表。

---

## 🖥️ 網頁介面概覽

### 儀表板
- 顯示目前時間
- 列出所有排程事件
- 可刪除事件

### 新增事件頁面
- 表單以建立新事件
- 根據重複方式動態顯示控制項
- 欄位驗證

---

## 🚀 未來改進

- 使用 SPIFFS 或 EEPROM 儲存事件
- 網頁存取驗證
- 電子郵件或手機通知
- 與手機應用程式整合
- 即時回饋藥品分配狀態

---
