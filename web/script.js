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

const term = document.getElementById("terminal");
const termWind = document.getElementById("terminal_window")
setInterval(() => {
    updateState(left);
    updateState(right);
}, 500)


const ctx = document.getElementById('left_chart');

  new Chart(ctx, {
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