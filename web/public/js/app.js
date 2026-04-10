const humidityEl = document.getElementById('humidity');
const temperatureEl = document.getElementById('temperature');
const statusEl = document.getElementById('status');
const historyList = document.getElementById('historyList');
const lastUpdateEl = document.getElementById('lastUpdate');

let currentHumidity = '--';
let currentTemperature = '--';

// MQTT WebSocket connection (HiveMQ public broker)
const client = mqtt.connect('wss://broker.hivemq.com:8884/mqtt');

client.on('connect', function () {
    statusEl.textContent = 'Connected';
    statusEl.className = 'status connected';

    // Subscribe to sensor topics
    client.subscribe('bp8266/sensor/humidity');
    client.subscribe('bp8266/sensor/temperature');
    console.log('MQTT connected & subscribed!');
});

client.on('error', function (err) {
    statusEl.textContent = 'Disconnected';
    statusEl.className = 'status disconnected';
    console.error('MQTT error:', err);
});

client.on('offline', function () {
    statusEl.textContent = 'Disconnected';
    statusEl.className = 'status disconnected';
});

client.on('reconnect', function () {
    statusEl.textContent = 'Reconnecting...';
    statusEl.className = 'status connecting';
});

client.on('message', function (topic, message) {
    const value = parseFloat(message.toString()).toFixed(1);
    const now = new Date().toLocaleTimeString();

    if (topic === 'bp8266/sensor/humidity') {
        currentHumidity = value;
        humidityEl.textContent = value;
    }

    if (topic === 'bp8266/sensor/temperature') {
        currentTemperature = value;
        temperatureEl.textContent = value;
    }

    lastUpdateEl.textContent = 'Last update: ' + now;

    // Add to history (only when both values available)
    if (currentHumidity !== '--' && currentTemperature !== '--') {
        addHistory(now, currentHumidity, currentTemperature);
    }
});

function addHistory(time, hum, temp) {
    // Don't add duplicate entries within the same second
    if (historyList.firstChild && historyList.firstChild.dataset.time === time) {
        historyList.firstChild.innerHTML =
            '<span>' + time + '</span><span>💧 ' + hum + '% &nbsp; 🌡️ ' + temp + '°C</span>';
        return;
    }

    const item = document.createElement('div');
    item.className = 'history-item';
    item.dataset.time = time;
    item.innerHTML = '<span>' + time + '</span><span>💧 ' + hum + '% &nbsp; 🌡️ ' + temp + '°C</span>';
    historyList.insertBefore(item, historyList.firstChild);

    // Keep only last 20 readings
    while (historyList.children.length > 20) {
        historyList.removeChild(historyList.lastChild);
    }
}
