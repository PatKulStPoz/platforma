
// Generated with tools/generate_html.py
#pragma once
#define PAGE_COUNT 3
#include "esp_http_server.h"
#include "sdkconfig.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_log.h"

static const char* file_style_css = R"raw(
html {
    background-color: bisque;
}

.box {
    background-color: white;
    padding: 1px 5px;
}

#header {
    margin-bottom: 10px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    align-content: center;
    flex-wrap: wrap;
    flex-direction: row;
}

#header > #title {
    font-size: 2.5em;
}

#header > #websocket {
    font-size: 1.2em;
}

#terminal_window {
    display: block;
    height: 500px;
    background: black;
    color: white;
    overflow-y: scroll;
    justify-content: end;
    flex: 100;
}

#terminal > span {
    margin-top: 2px;
}

#terminal_box {
   flex-basis: 0;
   flex-grow: 1;
   padding: 10px;
   max-width: 900px;
}

#terminal {
    display: flex;
    flex-direction: column;
    flex-wrap: nowrap;
    align-content: flex-start;
    justify-content: flex-end;
    padding: 5px;
    min-width: 200px;
    flex-grow: 100;
}

#terminal_send {
    margin-top: 5px;
    display: flex;
}

#terminal_input {
    flex-grow: 100;
    margin-right: 5px;
}

#buttons  {
    min-width: 150px;
    max-width: 600px;
    display: flex;
    padding: 5px;
    flex-direction: column;
    flex-wrap: wrap;
    justify-content: space-evenly;
    align-items: center;
    flex: 1;
}

#buttons > div {
    display: flex;
    padding: 5px;
    flex-direction: row;
    flex-wrap: wrap;
    justify-content: space-evenly;
    align-items: center;
    flex: 1;
}

#buttons > hr {
    width: 100%;
}

#buttons > div > button {
    margin: 2px;
}

.selected {
    background-color: #c7c7ff;
}

.drive_item {
    width: auto;
    margin-bottom: 10px;
    width: 100%;
    display: flex;
}

.animated_wheel {
  height: 64px;
  width: 64px;
  min-height: 64px;
  min-width: 64px;
  background: #ffffff;
  background: linear-gradient(#000000, #888);
  border-radius: 100%;
  transition: transform 1000ms linear;
  transform: rotate(0deg);
  display: flex;
  justify-content: center;
  align-items: center;
  margin: 5px;
}

.animated_wheel > .point {
  height: 12px;
  width: 12px;
  display: block;
  position: absolute;
  background: red;
  border-radius: 100%;
  transform: translate(26px, 0px);
}

.animated_wheel > .point:nth-child(1) {
  background: green;
  transform: translate(-26px, 0px);
}

.animated_wheel > .point:nth-child(2) {
  background: blue;
  transform: translate(0, -26px);
}

.animated_wheel > .point:nth-child(3) {
  background: yellow;
  transform: translate(0, 26px);
}

#center_holder {    
    display: flex;
    flex-direction: row;
    flex-wrap: wrap;
    align-items: center;
    justify-content: space-between;
}

#center_holder > div {    
    margin: 10px;
}
)raw";
static esp_err_t page_get_file_style_css_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/css; charset=UTF-8");

    httpd_resp_send(
        req,
        file_style_css,
        HTTPD_RESP_USE_STRLEN
    );

    return ESP_OK;
}

static const char* file_page_html = R"raw(
<!DOCTYPE html>
<html>

<head>
    <title>Status platformy</title>
    <meta http-equiv="Content-Type" content="text/html;charset=utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">

    <link rel="stylesheet" href="style.css">
</head>

<body>
    <div class="box" id="header">
        <span id="title">Platforma Jeżdząca</span>
        <span id="websocket">Bateria: <span id="state:battery_percentage">???</span>%</span>
        <span id="websocket">WebSocket: <span id="ws-status">Łączenie...</span></span>
    </div>
    <div id="center_holder">
        <div>
            <div id="left_drive" class="drive_item box"> 
                <div class="info">
                    <h3>Lewe koło</h3>
                    <p>Oprót koła: <span id="left_hall_rotation">--</span> (<span id="left_rpm">--</span> RPM)</p>
                    <p>Oprót silnika: <span id="left_driver_rotation">--</span></p>
                    <p>Zachowanie: <span id="state:l_behavior">--</span></p>
                    <p>Sterowanie: <span id="left_level">--</span>% (<span id="left_current_level">--</span> / <span id="left_target_level">--</span>) / <span id="left_target_direction">--</span>
                    <p>Kierunek obrotu koła: <span id="left_direction">--</span></p>
                    <p>Odczyt halla: <span id="left_hallTicks">--</span> / <span id="state:l_hallTicksFullRotation">--</span> (<span id="state:l_hallTicks">--</span>)</p>
                    <p>Odczyt silnika: <span id="left_driverTicks">--</span> / <span id="state:l_driverTicksFullRotation">--</span> (<span id="state:l_driverTicks">--</span>)</p>

                </div>
                <div class="animated_wheel" id="left_animated_wheel">
                    <div class="point"></div>
                    <div class="point"></div>
                    <div class="point"></div>
                    <div class="point"></div>
                </div>
            </div>
            <div id="right_drive" class="drive_item box">
                <div class="info">
                    <h3>Prawe koło</h3>
                    <p>Oprót koła: <span id="right_hall_rotation">--</span> (<span id="right_rpm">--</span> RPM)</p>
                    <p>Oprót silnika: <span id="right_driver_rotation">--</span></p>
                    <p>Zachowanie: <span id="state:r_behavior">--</span></p>
                    <p>Sterowanie: <span id="right_level">--</span>% (<span id="right_current_level">--</span> / <span id="right_target_level">--</span>) / <span id="right_target_direction">--</span>
                    <p>Kierunek obrotu koła: <span id="right_direction">--</span></p>
                    <p>Odczyt halla: <span id="right_hallTicks">--</span> / <span id="state:r_hallTicksFullRotation">--</span> (<span id="state:r_hallTicks">--</span>)</p>
                    <p>Odczyt silnika: <span id="right_driverTicks">--</span> / <span id="state:r_driverTicksFullRotation">--</span> (<span id="state:r_driverTicks">--</span>)</p>
                </div>
                <div class="animated_wheel" id="right_animated_wheel">
                    <div class="point"></div>
                    <div class="point"></div>
                    <div class="point"></div>
                    <div class="point"></div>
                </div>
            </div>
        </div>
        <div class="box" id="buttons">
            <div>
                <button type="button" onclick="k = ''; buttonSelect(this, 'wheel_selector')" class="wheel_selector selected" >Oba koła</button>
                <button type="button" onclick="k = 'l:'; buttonSelect(this, 'wheel_selector')" class="wheel_selector">Lewe koło</button>
                <button type="button" onclick="k = 'r:'; buttonSelect(this, 'wheel_selector')"  class="wheel_selector">Prawe koło</button>
            </div>
            <hr>
            <div>
                <button type="button" onclick="sendCommand(`${k}start now`);">Start</button>
                <button type="button" onclick="sendCommand(`${k}stop now`);">Stop</button>
                <button type="button" onclick="sendCommand(`${k}setbrake 0`);">Zwolnij hamulce</button>
                <button type="button" onclick="sendCommand(`${k}setbrake 1`);">Zaciśnij hamulce</button>
                <button type="button" onclick="sendCommand(`${k}setdir f`);">Przód</button>
                <button type="button" onclick="sendCommand(`${k}setdir b`);">Tył</button>
                <button type="button" onclick="sendCommand(`${k}reset`);">Reset</button>

            </div>
            <div>
                <button type="button" onclick="sendCommand(`${k}setlevel 0 now`);">Moc silnika 0%</button>
                <button type="button" onclick="sendCommand(`${k}setlevel 25 now`);">Moc silnika 25%</button>
                <button type="button" onclick="sendCommand(`${k}setlevel 50 now`);">Moc silnika 50%</button>
                <button type="button" onclick="sendCommand(`${k}setlevel 75 now`);">Moc silnika 75%</button>
                <button type="button" onclick="sendCommand(`${k}setlevel 100 now`);">Moc silnika 100%</button>
            </div>
            <div>
                <button type="button" onclick="sendCommand(`${k}rotate 30;${k}waitf`);">Obrót kołem 30°</button>
                <button type="button" onclick="sendCommand(`${k}rotate 60;${k}waitf`);">Obrót kołem 60°</button>
                <button type="button" onclick="sendCommand(`${k}rotate 90;${k}waitf`);">Obrót kołem 90°</button>
                <button type="button" onclick="sendCommand(`${k}rotate 180;${k}waitf`);">Obrót kołem 180°</button>
                <button type="button" onclick="sendCommand(`${k}rotate 360;${k}waitf`);">Obrót kołem 360°</button>
            </div>
        </div>
        <div id="terminal_box">
            <div id="terminal_window"><div id="terminal"></div></div>
            <div id="terminal_send">
                <input type="text" id="terminal_input" name="command" />
                <button id="terminal_button">Wyślij</button>
            </div>
        </div>
    </div>
        <script src="/script.js" type="module"></script>
    <script>
        var k = '';
    </script>
</body>

</html>
)raw";
static esp_err_t page_get_file_page_html_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html; charset=UTF-8");

    httpd_resp_send(
        req,
        file_page_html,
        HTTPD_RESP_USE_STRLEN
    );

    return ESP_OK;
}

static const char* file_script_js = R"raw(
let connetion = new WebSocket("ws://" + window.location.host + "/ws");
const statusElement = document.getElementById("ws-status");

function createObj(name) {
    return {
        __receivedCount: 0,
        __animatedWheel: document.getElementById(`${name}_animated_wheel`),
        __hallRotation: document.getElementById(`${name}_hall_rotation`),
        __driverRotation: document.getElementById(`${name}_driver_rotation`),
        __direction: document.getElementById(`${name}_direction`),
        __target_direction: document.getElementById(`${name}_target_direction`),
        __level: document.getElementById(`${name}_level`),
        __current_level: document.getElementById(`${name}_current_level`),
        __target_level: document.getElementById(`${name}_target_level`),
        __hallTicks: document.getElementById(`${name}_hallTicks`),
        __driverTicks: document.getElementById(`${name}_driverTicks`),

        __rpm: document.getElementById(`${name}_rpm`),

        __initialRotation: 0,
        halTicksFullRotation: 9,
        halTicks: 0
    }
}

const left = createObj("left");
const right = createObj("right");
function updateState(object) {
    if (object.__receivedCount == 0) {
        object.__initialRotation = Math.floor(object.halTicks / object.halTicksFullRotation) * 360
    }
    object.__receivedCount++;

    object.__animatedWheel.style.transform = `rotate(${ (360 * object.hallTicks / object.hallTicksFullRotation) - object.__initialRotation }deg)`
    object.__hallRotation.textContent = ((360 + (360 * object.hallTicks / object.hallTicksFullRotation) % 360) % 360).toFixed(1)
    object.__driverRotation.textContent = (360 * object.driverTicks / object.driverTicksFullRotation % 360).toFixed(1)
    object.__rpm.textContent = (object.hall_tick_time < 0xFF000000 ?  1 / (object.hall_tick_time / 1000 * object.hallTicksFullRotation / 60) : 0).toFixed(2)
    object.__direction.textContent = ["Brak", "Przód", "Brak", "Tył"][object.direction + 1]
    object.__target_direction.textContent = object.target_direction == -1 ? "Tył" : "Przód"
    object.__level.textContent = (object.level / 255 * 100).toFixed(1)
    object.__target_level.textContent = (object.target_level / 255 * 100).toFixed(1)
    object.__current_level.textContent = (object.current_level / 255 * 100).toFixed(1)
    object.__hallTicks.textContent = (object.hallTicksFullRotation + (object.hallTicks % object.hallTicksFullRotation)) % object.hallTicksFullRotation
    object.__driverTicks.textContent = object.driverTicks % object.driverTicksFullRotation

}

const term = document.getElementById("terminal");
const termWind = document.getElementById("terminal_window")


const ctx = document.getElementById('left_chart');

 /* new Chart(ctx, {
    type: 'line',
    data: {
      labels: ['Red', 'Blue', 'Yellow', 'Green', 'Purple', 'Orange'],
      datasets: [{
        label: '# of Votes',
        data: [12, 19, 3, 5, 2, 3],
        borderWidth: 1
      }]
    },
    options: {
      scales: {
        y: {
          beginAtZero: true
        }
      }
    }
  });
*/
function log(...data) {
    console.log(data);
    const shouldScroll = termWind.scrollHeight - termWind.clientHeight <= termWind.scrollTop + 1;
    for (const i of data) {
        const span = document.createElement("span")
        span.textContent = i;
        term.appendChild(span)
    }
    if (shouldScroll) {
        termWind.scrollTop = termWind.scrollHeight - termWind.clientHeight;
    }
}

function error(...data) {
    console.error(data);
    const shouldScroll = termWind.scrollHeight - termWind.clientHeight <= termWind.scrollTop + 1;
    for (const i of data) {
        const span = document.createElement("span")
        span.textContent = i;
        span.style.color = "red"
        term.appendChild(span);
    }
    if (shouldScroll) {
        termWind.scrollTop = termWind.scrollHeight - termWind.clientHeight;
    }
}

function setup_socket(socket) {
    socket.onopen = function () {
        log("[WEB] WebSocket connected");
        statusElement.textContent = "POŁĄCZONO";
        socket.send("CONNECTED>");
    };

    socket.onmessage = function (event) {
        log("[ESP32] " + event.data);
    };

    socket.onerror = function (err) {
        error("[WEB] WebSocket error:", err);
        statusElement.textContent = "BŁĄD";
    };

    socket.onclose = function () {
        log("[WEB] WebSocket disconnected... Reconnecting in 5 seconds");
        statusElement.textContent = "ROZŁĄCZONO";

        setTimeout(() => {
            log("[WEB] Attempting to reconnect...");
            connetion = new WebSocket("ws://" + window.location.host + "/ws")
            setup_socket(connetion);
        }, 5000)
    };

    socket.onmessage = function (event) {
        const data = JSON.parse(event.data);
        if (data["__type"] == "update_state") {
            for (const property in data) {
                if (property.startsWith("_")) continue;
                
                const doc = document.getElementById(`state:${property}`);
                if (doc) {
                    doc.textContent = data[property];
                }

                if (property.startsWith("l_")) {
                    left[property.substring(2)] = data[property];
                } else if (property.startsWith("r_")) {
                    right[property.substring(2)] = data[property];
                }
            }

            updateState(left);
            updateState(right);
        } else if (data["__type"] == "print") {
            log(data.text)
        }
    };

}

setup_socket(connetion);

const inputField = document.getElementById("terminal_input");
const sendButton = document.getElementById("terminal_button");

function sendCommand(command) {
    if (connetion.readyState === connetion.OPEN) {
        connetion.send(`EXEC>${command}`);
        log(`EXEC>${command}`);
    } else {
        log(`[WEB] Failed to sent '${command}' command! Not connected!`);
    }
}

window.sendCommand = sendCommand;

inputField.addEventListener("keypress", function(event) {
  if (event.key === "Enter") {
    event.preventDefault();

    sendButton.click();
  }
});

sendButton.onclick = () => {
    if (inputField.value == '') {
        return;
    }

    sendCommand(inputField.value)
    inputField.value = "";
}


function buttonSelect(button, clazz) {
    for (let b of document.getElementsByClassName(clazz)) {
        b.classList.remove("selected")
    }
    button.classList.add("selected");
}

window.buttonSelect = buttonSelect;
)raw";
static esp_err_t page_get_file_script_js_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/javascript; charset=UTF-8");

    httpd_resp_send(
        req,
        file_script_js,
        HTTPD_RESP_USE_STRLEN
    );

    return ESP_OK;
}

void init_pages(httpd_handle_t server) {
    httpd_uri_t uri_root = {};


    uri_root.uri = "/style.css";
    uri_root.method = HTTP_GET;
    uri_root.handler = page_get_file_style_css_handler;
    uri_root.user_ctx = nullptr;

    ESP_ERROR_CHECK(
        httpd_register_uri_handler(
            server,
            &uri_root
        )
    );
    
    uri_root.uri = "/page.html";
    uri_root.method = HTTP_GET;
    uri_root.handler = page_get_file_page_html_handler;
    uri_root.user_ctx = nullptr;

    ESP_ERROR_CHECK(
        httpd_register_uri_handler(
            server,
            &uri_root
        )
    );
    
    uri_root.uri = "/script.js";
    uri_root.method = HTTP_GET;
    uri_root.handler = page_get_file_script_js_handler;
    uri_root.user_ctx = nullptr;

    ESP_ERROR_CHECK(
        httpd_register_uri_handler(
            server,
            &uri_root
        )
    );
    }