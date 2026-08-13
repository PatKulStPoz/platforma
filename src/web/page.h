
// Generated with tools/generate_html.py
#pragma once
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

#terminal_send {
    margin-top: 5px;
    display: flex;
}

#terminal_input {
    flex-grow: 100;
    margin-right: 5px;
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


#terminal {
    display: block;
}

#center_holder {    
    display: flex;
    flex-direction: row;
    flex-wrap: wrap;
    align-items: center;
    justify-content: space-between;

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
        <span id="websocket">WebSocket: <span id="ws-status">Łączenie...</span></span>
    </div>
    <div id="center_holder">
        <div>
            <div id="left_drive" class="drive_item box"> 
                <div class="info">
                    <h3>Lewe koło</h3>
                    <p>l_driverTicksFullRotation: <span id="state:l_driverTicksFullRotation">--</span></p>
                    <p>l_halTicksFullRotation: <span id="state:l_halTicksFullRotation">--</span></p>
                    <p>l_driverTicks: <span id="state:l_driverTicks">--</span></p>
                    <p>l_rotationTick: <span id="state:l_rotationTick">--</span></p>
                    <p>l_driverTicksPerHal: <span id="state:l_driverTicksPerHal">--</span></p>
                    <p>l_halTicks: <span id="state:l_halTicks">--</span></p>
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
                    <p>r_driverTicksFullRotation: <span id="state:r_driverTicksFullRotation">--</span></p>
                    <p>r_halTicksFullRotation: <span id="state:r_halTicksFullRotation">--</span></p>
                    <p>r_driverTicks: <span id="state:r_driverTicks">--</span></p>
                    <p>r_rotationTick: <span id="state:r_rotationTick">--</span></p>
                    <p>r_driverTicksPerHal: <span id="state:r_driverTicksPerHal">--</span></p>
                    <p>r_halTicks: <span id="state:r_halTicks">--</span></p>
                </div>
                <div class="animated_wheel" id="right_animated_wheel">
                    <div class="point"></div>
                    <div class="point"></div>
                    <div class="point"></div>
                    <div class="point"></div>
                </div>
            </div>
        </div>
        <div>
            <div id="terminal"></div>
            <div id="terminal_send">
                <input type="text" id="terminal_input" name="command" />
                <button id="terminal_button">Wyślij</button>
            </div>
        </div>
    </div>

    <script src="/script.js" type="module"></script>

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
//import * as xterm from '/xterm/xterm.mjs';

let socket = new WebSocket("ws://" + window.location.host + "/ws");
const statusElement = document.getElementById("ws-status");

function createObj(name) {
    return {
        __animatedWheel: document.getElementById(`${name}_animated_wheel`),
        halTicksFullRotation: 9,
        halTicks: 0
    }
}

const left = createObj("left");
const right = createObj("right");
function updateState(object) {
    object.__animatedWheel.style.transform = `rotate(${ (360 * object.halTicks / object.halTicksFullRotation) % 36000 }deg)`
}

setInterval(() => {
    updateState(left);
    updateState(right);
}, 500)

//const term = new xterm.Terminal();
//term.open(document.getElementById('terminal'));

function log(...data) {
    console.log(data);
    /*for (const i of data) {
        term.write(i.toString());
        term.write("\n\r");
    }*/
}

function error(...data) {
    console.error(data);
    /*term.write("\x1B[1;3;31m");
    for (const i of data) {
        term.write(i.toString());
        term.write("\n\r");
    }
    term.write("\x1B[0m");*/
}

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
    log("[WEB] WebSocket disconnected");
    statusElement.textContent = "ROZŁĄCZONO";
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
    }
};

const inputField = document.getElementById("terminal_input");
const sendButton = document.getElementById("terminal_button");

sendButton.onclick = () => {
    socket.send(`EXEC>${inputField.value}`);
    log(`EXEC>${inputField.value}`);
    inputField.value = "";
}
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