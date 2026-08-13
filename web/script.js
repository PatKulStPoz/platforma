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