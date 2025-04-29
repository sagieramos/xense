const statusDiv = document.getElementById('status');
const scanBtn = document.getElementById('scan-btn');
const wifiCards = document.getElementById('wifi-cards');
const spinner = document.getElementById('spinner');

let socket;

function getRssiColor(rssi) {
    if (rssi >= -50) return '#4caf50'; // strong - green
    if (rssi >= -70) return '#ff9800'; // okay - orange
    return '#f44336'; // weak - red
}

function showSpinner() {
    spinner.classList.remove('hidden');
}

function hideSpinner() {
    spinner.classList.add('hidden');
}

function connectWebSocket() {
    socket = new WebSocket('ws://' + window.location.host + '/ws');

    socket.onopen = () => statusDiv.textContent = 'Connected!';
    socket.onclose = () => {
        statusDiv.textContent = 'Disconnected. Reconnecting...';
        setTimeout(connectWebSocket, 2000);
    };
    socket.onerror = error => console.error('WebSocket Error', error);

    socket.onmessage = function(event) {
        hideSpinner(); // Hide spinner on response

        const data = JSON.parse(event.data);
        if (data.aps) {
            wifiCards.innerHTML = '';
            data.aps.forEach(ap => {
                const card = document.createElement('div');
                card.className = 'wifi-card';
                card.innerHTML = `
                    <h2>${ap.ssid || '(Hidden SSID)'}</h2>
                    <p><strong>BSSID:</strong> ${ap.bssid}</p>
                    <p><strong>RSSI:</strong> <span style="color:${getRssiColor(ap.rssi)}">${ap.rssi} dBm</span></p>
                    <p><strong>Authmode:</strong> ${ap.authmode}</p>
                    <p><strong>Cipher:</strong> ${ap.cipher}</p>
                    <p><strong>Bandwidth:</strong> ${ap.bandwidth}</p>
                `;
                wifiCards.appendChild(card);
            });
        }
    };
}

scanBtn.addEventListener('click', () => {
    if (socket && socket.readyState === WebSocket.OPEN) {
        socket.send('scan');
        showSpinner(); 
    }
});

connectWebSocket();
