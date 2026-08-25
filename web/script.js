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
    object.__hallRotation.textContent = (360 + (360 * object.hallTicks / object.hallTicksFullRotation) % 360) % 360
    object.__driverRotation.textContent = 360 * object.driverTicks / object.driverTicksFullRotation % 360
    object.__rpm.textContent = (object.hall_tick_time < 0xFF000000 ?  1 / (object.hall_tick_time / 1000 * object.hallTicksFullRotation / 60) : 0).toFixed(2)
    object.__direction.textContent = ["Brak", "Przód", "Brak", "Tył"][object.direction + 1]
    object.__target_direction.textContent = object.target_direction == -1 ? "Tył" : "Przód"
    object.__level.textContent = (object.level / 255).toFixed(2)
    object.__target_level.textContent = (object.target_level / 255).toFixed(2)
    object.__current_level.textContent = (object.current_level / 255).toFixed(2)
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

    connetion.send(`EXEC>${inputField.value}`);
    log(`EXEC>${inputField.value}`);
    inputField.value = "";
}