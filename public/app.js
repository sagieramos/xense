document.getElementById('scanButton').addEventListener('click', startWebSocket);

function startWebSocket() {
  const socket = new WebSocket('ws://<ESP32_IP>/ws');

  socket.onopen = function(event) {
    console.log('WebSocket connection established');
  };

  socket.onmessage = function(event) {
    const data = JSON.parse(event.data);
    updateWifiTable(data.networks);
  };

  socket.onerror = function(event) {
    console.error('WebSocket error:', event);
  };

  socket.onclose = function(event) {
    console.log('WebSocket connection closed');
  };
}

function updateWifiTable(networks) {
  const tbody = document.getElementById('wifiTable').getElementsByTagName('tbody')[0];
  tbody.innerHTML = ''; // Clear existing table data

  networks.forEach(network => {
    const row = tbody.insertRow();
    row.insertCell(0).textContent = network.ssid;
    row.insertCell(1).textContent = network.rssi;
    row.insertCell(2).textContent = network.channel;
  });
}
