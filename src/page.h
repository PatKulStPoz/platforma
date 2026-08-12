#pragma once

static const char *html_page = R"raw(
<!DOCTYPE html>
<html>
  <head>
    <title>Status platformy</title>
  </head>
  <body>
    <h3>
    WebSocket:
    <span id="ws-status">Łączenie...</span>
    </h3>

    <h3>Lewe koło</h3>
    <p>l_driverTicksFullRotation: <span id="l_driverTicksFullRotation">--</span></p>
    <p>l_halTicksFullRotation: <span id="l_halTicksFullRotation">--</span></p>
    <p>l_driverTicks: <span id="l_driverTicks">--</span></p>
    <p>l_rotationTick: <span id="l_rotationTick">--</span></p>
    <p>l_driverTicksPerHal: <span id="l_driverTicksPerHal">--</span></p>
    <p>l_halTicks: <span id="l_halTicks">--</span></p>

    <h3>Prawe koło</h3>
    <p>r_driverTicksFullRotation: <span id="r_driverTicksFullRotation">--</span></p>
    <p>r_halTicksFullRotation: <span id="r_halTicksFullRotation">--</span></p>
    <p>r_driverTicks: <span id="r_driverTicks">--</span></p>
    <p>r_rotationTick: <span id="r_rotationTick">--</span></p>
    <p>r_driverTicksPerHal: <span id="r_driverTicksPerHal">--</span></p>
    <p>r_halTicks: <span id="r_halTicks">--</span></p>

    <script>
        let socket = new WebSocket("ws://" + window.location.host + "/ws");
        socket.onopen = function() {
            console.log("WebSocket connected");
            document.getElementById("ws-status").textContent = "POŁĄCZONO";
            socket.send("CLIENT CONNECTED");
        };

        socket.onmessage = function(event) {
            console.log("ESP32:", event.data);
        };

        socket.onerror = function(error) {
            console.error("WebSocket error:", error);
            document.getElementById("ws-status").textContent = "BŁĄD";
        };

        socket.onclose = function() {
            console.log("WebSocket disconnected");
            document.getElementById("ws-status").textContent = "ROZŁĄCZONO";
        };

        socket.onmessage = function(event) {

        const data = JSON.parse(event.data);

        console.log("l_driverTicksFullRotation:", data.l_driverTicksFullRotation);
        console.log("l_halTicksFullRotation:", data.l_halTicksFullRotation);
        console.log("l_driverTicks:", data.l_driverTicks);
        console.log("l_rotationTick:", data.l_rotationTick);
        console.log("l_driverTicksPerHal:", data.l_driverTicksPerHal);
        console.log("l_halTicks:", data.l_halTicks);

        console.log("r_PWM_counter:", data.r_PWM_counter);
        console.log("r_hall_counter:", data.r_hall_counter);
        console.log("Status(R):", data.r_state);

        document.getElementById("l_driverTicksFullRotation").textContent =
            data.l_driverTicksFullRotation;

        document.getElementById("l_halTicksFullRotation").textContent =
            data.l_halTicksFullRotation;

        document.getElementById("l_driverTicks").textContent =
            data.l_driverTicks;

        document.getElementById("l_rotationTick").textContent =
            data.l_rotationTick;

        document.getElementById("l_driverTicksPerHal").textContent =
            data.l_driverTicksPerHal;

        document.getElementById("l_halTicks").textContent =
            data.l_halTicks;
        
        document.getElementById("r_driverTicksFullRotation").textContent =
            data.r_driverTicksFullRotation;

        document.getElementById("r_halTicksFullRotation").textContent =
            data.r_halTicksFullRotation;

        document.getElementById("r_driverTicks").textContent =
            data.r_driverTicks;

        document.getElementById("r_rotationTick").textContent =
            data.r_rotationTick;

        document.getElementById("r_driverTicksPerHal").textContent =
            data.r_driverTicksPerHal;

        document.getElementById("r_halTicks").textContent =
            data.r_halTicks;
        };
  
    </script>

  </body>
</html>
)raw";
