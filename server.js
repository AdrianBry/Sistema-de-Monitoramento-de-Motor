const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const path = require('path');
const cors = require('cors');
const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));


// DADOS USADOS NO HTML
let sensorData = {temp_motor: 0, rpm: 0, ventoinha: 0, time_array: [], temp_motor_array: []};


// ROTA PRINCIPAL
app.get('/', (req, res) => {res.sendFile(path.join(__dirname, 'public', 'index.html'));});


// RECEBE DADOS DO ESP32
app.post('/update', (req, res) => {
    const data = req.body;
    sensorData = {...sensorData,...data,
        temp_motor_array: data.temp_motor_array || sensorData.temp_motor_array,
        time_array: data.time_array || sensorData.time_array
    };


// ENVIA PARA O HTML
    const payload = JSON.stringify(sensorData);
    wss.clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(payload);
        }
    });

    res.sendStatus(200);
});


// WEBSOCKET
wss.on('connection', ws => {
    console.log('Cliente conectado');
    ws.send(JSON.stringify(sensorData));
    ws.on('close', () => {console.log('Cliente desconectado');});
});


// SERVIDOR
const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {console.log(`Servidor rodando na porta ${PORT}`);});