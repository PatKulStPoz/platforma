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
    <p>Status: <span id="l_state">OFF</span></p>
    <p>Licznik impulsów: <span id="l_PWM_counter">--</span></p>
    <p>Licznik impulsów (Hall): <span id="l_hall_counter">--</span></p>

    <h3>Prawe koło</h3>
    <p>Status: <span id="r_state">OFF</span></p>
    <p>Licznik impulsów: <span id="r_PWM_counter">--</span></p>
    <p>Licznik impulsów (Hall): <span id="r_hall_counter">--</span></p>

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

        console.log("Licznik impulsów(L):", data.l_PWM_counter);
        console.log("Licznik impulsów (Hall)(L):", data.l_hall_counter);
        console.log("Status(L):", data.l_state);

        console.log("Licznik impulsów(R):", data.r_PWM_counter);
        console.log("Licznik impulsów (Hall)(R):", data.r_hall_counter);
        console.log("Status(R):", data.r_state);

        document.getElementById("l_PWM_counter").textContent =
            data.l_PWM_counter;

        document.getElementById("l_hall_counter").textContent =
            data.l_hall_counter;

        document.getElementById('l_state').textContent =
            data.l_state ? 'ON' : 'OFF';
        
        document.getElementById("r_PWM_counter").textContent =
            data.r_PWM_counter;

        document.getElementById("r_hall_counter").textContent =
            data.r_hall_counter;

        document.getElementById('r_state').textContent =
            data.r_state ? 'ON' : 'OFF';
        };
  
    </script>

  </body>
</html>
)raw";
