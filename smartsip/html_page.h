#ifndef HTML_PAGE_H_
#define HTML_PAGE_H_

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>SmartSip</title>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
  <style>
    body { font-family: Arial, sans-serif; max-width: 600px; margin: auto; padding: 20px; }
    .container { display: flex; flex-direction: column; gap: 20px; }
    .display, .controls { border: 1px solid #ccc; padding: 20px; border-radius: 10px; }
    .display { text-align: left; }
    .display img { width: 100%; height: auto; max-width: 200px; max-height: 200px; }
    .controls { display: flex; flex-direction: column; gap: 15px; }
    .controls label { display: flex; justify-content: space-between; }
    .progress-container { width: 100%; background-color: #ddd; border-radius: 5px; }
    .progress-bar { width: 0; height: 20px; background-color: #4CAF50; border-radius: 5px; }
    button { width: 100%; padding: 10px; font-size: 16px; border: none; border-radius: 5px; cursor: pointer; }
    button:active { background-color: #ddd; }
    @media (min-width: 768px) {
      .container { flex-direction: row; gap: 30px; }
      .display, .controls { width: 100%; }
    }
  </style>
</head>
<body>
  <h2>SmartSip</h2>
  <div class="container">
    <div class="display">
      <h3>Current Status</h3>
      <p><i class="fas fa-thermometer-half"></i>Water Temperature(C): <span id="tempDisplay">20</span></p>
      <p><i class="fas fa-tint"></i>Water Volume(ml): <span id="volumeDisplay">150</span></p>
      <p><i class="fas fa-sticky-note"></i>Description: <span id="descriptionDisplay">This is the SmartSip</span></p>
      <img id="waterImage" src="https://via.placeholder.com/240x240" alt="Default Image">
    </div>

    <div class="controls">
      <h3>Control Panel</h3>
      <label>Water Volume (100-500ml): <span id="volumeValue">150</span></label>
      <input type="range" id="volume" min="100" max="500" value="150" oninput="updateVolume(this.value)">
      
      <label>Water Temperature (20-100C): <span id="tempValue">20</span></label>
      <input type="range" id="temperature" min="20" max="100" value="20" oninput="updateTemp(this.value)">

      <button onclick="sendCommand('start')">Add Water</button>
      <button onclick="sendCommand('stop')">Stop</button>

    </div>
  </div>
)rawliteral"
R"rawliteral(
  <script>
    const socket = new WebSocket("ws://" + location.hostname + ":81/");
    socket.onmessage = function(event) {
      const message = JSON.parse(event.data);
      if(typeof message['degree'] === 'string' && message['degree'].trim() !== "") {
        document.getElementById("tempDisplay").textContent = message['degree'];
      }

      if(typeof message['volume'] === 'string' && message['volume'].trim() !== "") {
        document.getElementById("volumeDisplay").textContent = message['volume'];
      }

      if(typeof message['type'] === 'string' && message['type'].trim() !== "") {
        document.getElementById("descriptionDisplay").textContent = message['type'];
      }
      if(typeof message['image'] === 'string' && message['image'].trim() !== "") {
        document.getElementById("waterImage").src = "data:image/jpeg;base64," + message['image'];
      } else {
        document.getElementById("waterImage").src = "https://via.placeholder.com/240x240"
      }
    };
    function updateVolume(value) {
      document.getElementById("volumeValue").textContent = value;
    }
    function updateTemp(value) {
      document.getElementById("tempValue").textContent = value;
    }
    function sendCommand(command) {
      const volume = document.getElementById("volume").value;
      const temp = document.getElementById("temperature").value;
      const data = { type: command, volume: volume, degree: temp };
      socket.send(JSON.stringify(data));
    }
  </script>
</body>
</html>
)rawliteral";

#endif