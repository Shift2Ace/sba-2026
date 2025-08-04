#include <WiFi.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


// Pin
RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);


// *** Update the SSID and password
const char* ssid = "LC_RM311";
const char* password = "a2b3c4d5";

int nextID = 1;

char timeStr[9]; // HH:MM:SS
unsigned long currentTimestamp = 0; // 

int status = 0;
// 0: Normal
// 1: Outputing
// 2: Wait to take

unsigned long lastDisplayUpdate = 0;
const unsigned long displayInterval = 1000; // 1 second

// Set web server port to 80
WebServer server(80);

// setup data storage
DynamicJsonDocument doc(2048);
JsonArray eventDataArray = doc.to<JsonArray>();

// HTML home page
const char* homePageHtml = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <title>Event Dashboard</title>
    <style>
      body {
        font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
        background-color: #f4f7f9;
        color: #333;
        margin: 0;
        padding: 0;
        display: flex;
        justify-content: center;
      }

      .container {
        max-width: 800px;
        width: 95%;
        background-color: #fff;
        padding: 2em;
        margin: 2em;
        border-radius: 12px;
        box-shadow: 0 0 15px rgba(0, 0, 0, 0.1);
      }

      h1 {
        text-align: center;
        margin-bottom: 1em;
      }

      #currentTime {
        text-align: center;
        font-weight: bold;
        font-size: 1.1em;
        margin-bottom: 1em;
      }

      .btn {
        display: block;
        margin: 1em auto;
        padding: 0.7em 1.5em;
        background-color: #4a90e2;
        color: white;
        border: none;
        border-radius: 6px;
        font-size: 1em;
        cursor: pointer;
        transition: background-color 0.3s ease;
        text-decoration: none;
        text-align: center;
      }

      .btn:hover {
        background-color: #357abd;
      }

      table {
        width: 100%;
        border-collapse: collapse;
        margin-top: 1em;
      }

      th,
      td {
        padding: 0.8em;
        border: 1px solid #ddd;
        text-align: center;
      }

      th {
        background-color: #f0f3f7;
      }
      .clock-btn {
        padding: 0.5em 1em;
        background-color: #0e8620;
        color: white;
        border: none;
        border-radius: 6px;
        font-size: 0.95em;
        cursor: pointer;
      }

      .clock-btn:hover {
        background-color: #196237;
      }

      .clock-bar {
        display: flex;
        justify-content: center;
        align-items: center;
        gap: 1em;
        margin-bottom: 1em;
      }

      #currentTime {
        font-weight: bold;
        font-size: 1.1em;
        margin: 0;
        padding: 0.5em 1em;
        border-radius: 6px;
        display: flex;
        align-items: center;
        height: 40px;
      }
    </style>
  </head>
  <body>
    <div class="container">
      <h1>Dashboard</h1>
      <div
        style="
          display: flex;
          justify-content: center;
          align-items: center;
          gap: 1em;
          margin-bottom: 1em;
        "
      >
        <div class="clock-bar">
          <div id="currentTime">Current Time: --</div>
          <button class="clock-btn" onclick="onClockButtonClick()">
            Sync Events
          </button>
        </div>
      </div>
      <a href="/addEvent" class="btn">Add Event</a>

      <table>
        <thead>
          <tr>
            <th>Event ID</th>
            <th>Repeat</th>
            <th>Time</th>
            <th>Storage ID</th>
            <th>Amount</th>
          </tr>
        </thead>
        <tbody id="eventTableBody"></tbody>
      </table>
    </div>

    <script>
      function syncTime() {
        const now = new Date();
        const formatted = now.toLocaleString();
        document.getElementById("currentTime").innerText =
          "Current Time: " + formatted;
      }

      // Update time every second
      setInterval(syncTime, 1000);
      syncTime();

      async function fetchEvents() {
        const response = await fetch('/events');
        const events = await response.json();
        const tableBody = document.getElementById('eventTableBody');
        tableBody.innerHTML = ''; // Clear previous rows
    
        events.forEach(event => {
          const row = document.createElement('tr');
          row.innerHTML = `
            <td>${event.id}</td>
            <td>${event.repeat || 'N/A'}</td>
            <td>${event.time}</td>
            <td>Storage ${event.storageId}</td>
            <td>${event.amount}</td>
            <td><button class="delete-btn" onclick="deleteRow(this)">Delete</button></td>
          `;
          tableBody.appendChild(row);
        });
      }

      function deleteRow(button) {
        const row = button.closest('tr');
        const eventId = row.children[0].innerText;
      
        fetch(`/delete?id=${eventId}`, { method: 'GET' })
          .then(res => res.text())
          .then(data => {
            alert("Deleted: " + data);
            fetchEvents(); // Refresh table
          })
          .catch(err => {
            console.error("Delete failed:", err);
            alert("Failed to delete event");
          });
      }

    
      // Call fetchEvents once page loads
      window.onload = () => {
        fetchEvents();
        setInterval(fetchEvents, 5000); // Fetch every 5 seconds
      };

    </script>
  </body>
</html>
)rawliteral";

// add event page
const char* addEventPageHtml = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <title>Add Event</title>
    <style>
      body {
        font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
        background-color: #f4f7f9;
        color: #333;
        margin: 0;
        padding: 0;
        display: flex;
        justify-content: center;
      }

      .container {
        max-width: 500px;
        width: 90%;
        background-color: #fff;
        padding: 2em;
        margin: 2em;
        border-radius: 12px;
        box-shadow: 0 0 15px rgba(0, 0, 0, 0.1);
      }

      h1 {
        text-align: center;
        margin-bottom: 1em;
      }

      fieldset {
        border: 1px solid #ccc;
        border-radius: 8px;
        padding: 1em;
        margin-bottom: 1.5em;
      }

      legend {
        font-weight: bold;
      }

      label {
        display: block;
        margin: 0.5em 0;
      }

      input[type="checkbox"] {
        margin-right: 0.5em;
      }

      input[type="time"],
      input[type="number"],
      select {
        width: 100%;
        padding: 0.5em;
        border: 1px solid #ccc;
        border-radius: 6px;
        margin-top: 0.5em;
        box-sizing: border-box;
      }

      button {
        margin-top: 1.5em;
        width: 100%;
        padding: 0.7em;
        background-color: #4a90e2;
        color: white;
        border: none;
        border-radius: 6px;
        font-size: 1em;
        cursor: pointer;
        transition: background-color 0.3s ease;
      }

      button:hover {
        background-color: #357abd;
      }

      #currentTime {
        text-align: center;
        margin: 1em 0;
        font-weight: bold;
        font-size: 1.1em;
      }

      .selectButton {
        margin-top: 10px;
        width: 200px;
      }

      #unselectAllBtn {
        background-color: red;
      }

      #evenyNDay {
        width: 100px;
      }
    </style>
  </head>
  <body>
    <div class="container">
      <h1>Add Event</h1>

      <form id="itemForm">
        <fieldset>
          <legend>Repeat:</legend>
          <select>
            <option>Every Day</option>
            <option>Weekdays</option>
            <option>Every (N) Day</option>
          </select>
          <button type="button" id="selectAllBtn" class="selectButton">
            Select All
          </button>
          <button type="button" id="unselectAllBtn" class="selectButton">
            Unselect All
          </button>
          <label
            ><input type="checkbox" name="weekday" value="0" />Sunday</label
          >
          <label
            ><input type="checkbox" name="weekday" value="1" />Monday</label
          >
          <label
            ><input type="checkbox" name="weekday" value="2" />Tuesday</label
          >
          <label
            ><input type="checkbox" name="weekday" value="3" />Wednesday</label
          >
          <label
            ><input type="checkbox" name="weekday" value="4" />Thursday</label
          >
          <label
            ><input type="checkbox" name="weekday" value="5" />Friday</label
          >
          <label
            ><input type="checkbox" name="weekday" value="6" />Saturday</label
          >
          <label> Every <input type="number" id="evenyNDay" /> day </label>
        </fieldset>

        <label>
          Time:
          <input type="time" name="time" required />
        </label>

        <label id="storageId">
          Storage ID:
          <select name="storageId" required></select>
        </label>

        <label>
          Amount:
          <input type="number" name="amountNum" min="1" value="1" required />
        </label>

        <button type="button" onclick="submitForm()">Submit</button>
      </form>
    </div>

    <script>
      const numOfStorage = 5;

      function syncTime() {
        const now = new Date();
        const formattedTime = now.toLocaleString();
        document.getElementById("currentTime").innerText =
          "Current Time: " + formattedTime;
      }
      // submit
      document
        .getElementById("itemForm")
        .addEventListener("submit", function (event) {
          event.preventDefault();
          const form = event.target;
          const weekdays = Array.from(
            form.querySelectorAll('input[name="weekday"]:checked')
          ).map((cb) => cb.value);
          const item = {
            weekday: weekdays,
            time: form.time.value,
            storageId: form.storageId.value,
            countNum: form.countNum.value,
          };
          console.log("Item added:", item);
          alert("Item added successfully!");
          form.reset();
        });

      const selectElement = document.querySelector("#storageId select");
      for (let i = 1; i <= numOfStorage; i++) {
        const option = document.createElement("option");
        option.value = i;
        option.textContent = `Storage ${i}`;
        selectElement.appendChild(option);
      }

      document
        .getElementById("selectAllBtn")
        .addEventListener("click", function () {
          const weekdayCheckboxes = document.querySelectorAll(
            'input[name="weekday"]'
          );
          weekdayCheckboxes.forEach((cb) => (cb.checked = true));
        });

      document
        .getElementById("unselectAllBtn")
        .addEventListener("click", function () {
          const weekdayCheckboxes = document.querySelectorAll(
            'input[name="weekday"]'
          );
          weekdayCheckboxes.forEach((cb) => (cb.checked = false));
        });

      const repeatSelect = document.querySelector("select");
      const weekdayLabels = document.querySelectorAll(
        'label input[name="weekday"]'
      );
      const selectAllBtn = document.getElementById("selectAllBtn");
      const unselectAllBtn = document.getElementById("unselectAllBtn");
      const everyNDayInput = document.getElementById("evenyNDay");

      // Group checkbox labels
      const weekdayContainer = [...weekdayLabels].map((cb) => cb.parentElement);

      // When repeat changes
      repeatSelect.addEventListener("change", function () {
        const value = this.value;

        if (value === "Every Day") {
          selectAllBtn.style.display = "none";
          unselectAllBtn.style.display = "none";
          everyNDayInput.parentElement.style.display = "none";
          weekdayContainer.forEach((el) => (el.style.display = "none"));
        } else if (value === "Weekdays") {
          selectAllBtn.style.display = "inline-block";
          unselectAllBtn.style.display = "inline-block";
          everyNDayInput.parentElement.style.display = "none";
          weekdayContainer.forEach((el) => (el.style.display = "block"));
        } else if (value === "Every (N) Day") {
          selectAllBtn.style.display = "none";
          unselectAllBtn.style.display = "none";
          everyNDayInput.parentElement.style.display = "block";
          weekdayContainer.forEach((el) => (el.style.display = "none"));
        }
      });

      // Trigger once on load
      repeatSelect.dispatchEvent(new Event("change"));

      // Submit form
      function submitForm() {
        const form = document.getElementById("itemForm");

        const repeatType = form.querySelector("select").value;
        const selectedWeekdays = Array.from(
          form.querySelectorAll('input[name="weekday"]:checked')
        ).map((cb) => cb.value);
        const everyNDayValue = document.getElementById("evenyNDay").value;
        const timeValue = form.time.value;
        const storageIdValue = form.storageId.value;
        const amountValue = form.amountNum.value;

        // Validation checks
        if (repeatType === "Weekdays" && selectedWeekdays.length === 0) {
          alert("Please select at least one weekday.");
          return;
        }

        if (
          repeatType === "Every (N) Day" &&
          (everyNDayValue === "" || everyNDayValue <= 0)
        ) {
          alert("Please enter a valid number for 'Every N Day'.");
          return;
        }

        if (!timeValue || !storageIdValue || !amountValue || amountValue <= 0) {
          alert("Please fill in all required fields correctly.");
          return;
        }

        // Construct data object
        const item = {
          Repeat: repeatType,
          Weekdays: selectedWeekdays.join("") || null,
          EveryNDays: repeatType === "Every (N) Day" ? everyNDayValue : null,
          Time: timeValue,
          StorageID: storageIdValue,
          Amount: amountValue,
        };

        console.log("Item added:", item);
        const queryString = new URLSearchParams(item).toString();
        fetch(`/submit?${queryString}`)
          .then((res) => res.text())
          .then((data) => {
            alert("Hardware: " + data);
            window.location.href = "/";
          })
          .catch((err) => {
            console.error("Error:", err);
            alert("Failed to reach hardware");
          });

        repeatSelect.dispatchEvent(new Event("change"));
      }
    </script>
  </body>
</html>

)rawliteral";

// Handle html
void handleRoot() {
  server.send(200, "text/html", homePageHtml);
}

void handleAddEvent() {
  server.send(200, "text/html", addEventPageHtml);
}

// get new event info
void handleSubmit() {
  String repeatMethod = server.arg("Repeat");
  String weekdays = server.arg("Weekdays");
  String everyNDays = server.arg("EveryNDays");
  String time = server.arg("Time");
  String storageID = server.arg("StorageID");
  String amount = server.arg("Amount");

  String repeat;

  Serial.println("Received form data:");
  Serial.println("Repeat: " + repeatMethod);
  Serial.println("Weekdays: " + weekdays);
  Serial.println("EveryNDays: " + everyNDays);
  Serial.println("Time: " + time);
  Serial.println("StorageID: " + storageID);
  Serial.println("Amount: " + amount);

  server.send(200, "text/plain", "Data received!");

  JsonObject obj = eventDataArray.createNestedObject();
  if (repeatMethod == "Weekdays"){
    repeat = weekdays;
  }else if (repeatMethod == "Every Day"){
    repeat = "0123456";
  }else if (repeatMethod == "Every (N) Day"){
    repeat = "*" + everyNDays;
  }
  obj["id"] = nextID;
  obj["repeat"] = repeat;
  obj["dayCount"] = everyNDays;
  obj["time"] = time;
  obj["storageId"] = storageID;
  obj["amount"] = amount;

  nextID += 1;
  
  serializeJson(eventDataArray, Serial);
}

void handleGetEvents() {
  String jsonOutput;
  serializeJson(eventDataArray, jsonOutput);
  server.send(200, "application/json", jsonOutput);
}

void handleDeleteEvent() {
  int idToDelete = server.arg("id").toInt();
  bool found = false;

  for (size_t i = 0; i < eventDataArray.size(); i++) {
    JsonObject obj = eventDataArray[i];
    if (obj["id"].as<int>() == idToDelete) {
      eventDataArray.remove(i);
      found = true;
      break;
    }
  }

  if (found) {
    server.send(200, "text/plain", "Event deleted");
  } else {
    server.send(404, "text/plain", "Event not found");
  }
}


void displayTime() {
  DateTime now = rtc.now(); 
  sprintf(timeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  Serial.print("Time: ");
  Serial.println(timeStr);

  lcd.setCursor(0, 1);
  lcd.print(timeStr);
}

void outputMedicine(int storageNum, int amount){
  status = 1;
  
  for (int i = 0; i < amount; i++) {
    // output one Medicine
    Serial.println("Output one medicine");
  }

}


void notificationOn(){
  // turn on LED
}

void notificationOff(){
  // turn off LED
  
}

void checkEventTask(void *parameter) {
  while (true) {
    DateTime now = rtc.now();
    if (currentTimestamp < now.unixtime()) {
      currentTimestamp += 60;
      DateTime currentTimestampDateTime(currentTimestamp);

      String currentTime = String(currentTimestampDateTime.hour()).length() < 2 ? "0" + String(currentTimestampDateTime.hour()) : String(currentTimestampDateTime.hour());
      currentTime += ":";
      currentTime += String(currentTimestampDateTime.minute()).length() < 2 ? "0" + String(currentTimestampDateTime.minute()) : String(currentTimestampDateTime.minute());
      Serial.println("Check Event (" + currentTime + ")");
      checkEvent(currentTime);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay to prevent CPU hogging
  }
}


void checkEvent(String currentTime) {
  DateTime now = rtc.now();
  int todayWeekday = now.dayOfTheWeek();
  
  for (JsonObject obj : eventDataArray) {
    String eventTime = obj["time"].as<String>();
    String repeat = obj["repeat"].as<String>();
    int storageId = obj["storageId"].as<int>();
    int amount = obj["amount"].as<int>();

    // Check if time matches
    if (eventTime == currentTime) {
      if (repeat.startsWith("*")) {
        // Every (N) Day logic
        int dayCount = obj["dayCount"].as<int>();
        if (dayCount == 0) {
          Serial.println("Output Medicine (SID: " + String(storageId) + "Amount: " + String(amount) + ")");
          outputMedicine(storageId, amount);
          int newCount = repeat.substring(1).toInt(); // Get N from "*N"
          obj["dayCount"] = newCount;
        } else {
          obj["dayCount"] = dayCount - 1;
        }
      } else {
        // Weekday logic
        std::vector<int> weekdays;
        
        for (char c : repeat) {
          if (isdigit(c)) { 
            weekdays.push_back(c - '0'); 
          }
        }
        
        for (int wd : weekdays) {
          if (wd == todayWeekday) {
            Serial.println("Output Medicine (SID: " + String(storageId) + "Amount: " + String(amount) + ")");
            outputMedicine(storageId, amount); 
            break; 
          }
        }
      }
    }
  }
  if (status == 1){
    notificationOn();
    status = 2;
    // Wait user to take
    Serial.println("Waiting for user");
    delay(1000);
    Serial.println("User took");
    status = 0;
    notificationOff();
  }
}


void setup() {
  Serial.println("Program start");
  Wire.begin(21, 22); 

  // LCD setup  
  Serial.println("LCD init");
  lcd.init();
  lcd.backlight();


  Serial.begin(115200);
  
  // Connect to Wi-Fi
  Serial.println("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Display IP address
  lcd.setCursor(0,0);
  lcd.print(WiFi.localIP());

  // Start the server
  server.on("/", handleRoot);
  server.on("/addEvent", handleAddEvent);
  server.on("/submit", handleSubmit);
  server.on("/events", handleGetEvents);
  server.on("/delete", handleDeleteEvent);


  server.begin();
  Serial.println("HTTP server started");

  // Clock setup
  
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
  }
  Serial.println("Clock reset");

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  currentTimestamp = rtc.now().unixtime();
  Serial.println("Current Timestamp: " + String(currentTimestamp));
  Serial.println("Check Event Task start");
  xTaskCreatePinnedToCore(
    checkEventTask,     // Task function
    "CheckEventTask",   // Name
    4096,               // Stack size
    NULL,               // Parameters
    1,                  // Priority
    NULL,               // Task handle
    1                   // Core (0 or 1)
  );
  Serial.println("Setup() done");
}


void loop() {
  // web server handling
  server.handleClient();
  // update display clock
  
  unsigned long currentMillis = millis();
  if (currentMillis - lastDisplayUpdate >= displayInterval) {
    lastDisplayUpdate = currentMillis;
    displayTime();
  }

}
