#include <WebSocketsServer.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <WebServer.h>
#include <max6675.h>

/* ------------------ OBJETOS ------------------ */
SoftwareSerial sim800l(16, 17);
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHTesp dht;

/* ------------------ PINES ------------------ */
int higrometro = 35;
int sensor_uv = 12;
int sensor_lluvia = 27;
int DHTPin = 15;

int drive1 = 32;
int drive2 = 33;

int speed1 = 4;
int speed2 = 2;

int rele1 = 25;
int rele2 = 26;

const int ktcSO = 5;
const int ktcCS = 18;
const int ktcCLK = 19;
MAX6675 ktc(ktcCLK, ktcCS, ktcSO);

/* ------------------------------------------------------ VARIABLES ------------------------------------------------------ */
float lectura_suelo;
float suelo_seco = 4095.0;
float suelo_humedo = 2430.0;

float t = NAN;
float st = NAN;
float h = NAN;
float s = NAN;
float uv;
int l = 0;
float voltaje;

/* ------------------------------------------------------ TIMERS ------------------------------------------------------ */
unsigned long tSensores = 0;
unsigned long tLCD = 0;
unsigned long tMotor = 0;
unsigned long tRele = 0;
unsigned long tSMS =0;

unsigned long segundosSistema = 0;
const unsigned long INT_SENSORES = 1000;
const unsigned long INT_LCD = 1000;
const unsigned long TIEMPO_MOTOR = 5000;
const unsigned long TIEMPO_RELE = 10000;
const unsigned long TIEMPO_SMS = 700;

/* ------------------------------------------------------ FLAGS ------------------------------------------------------ */
bool motorActivo = false;
bool estadoRele = false;
bool smsSueloEnviado = false;
bool smsTempEnviado = false;
bool smsTempSueloEnviado = false;
bool smsHumEnviado = false;
bool smsLluviaEnviado = false;
bool smsUvEnviado = false;

/* ------------------------------------------------------ WEB SERVER ------------------------------------------------------ */
// const char* = a cadena de caracteres inmutables
const char* ssid = "Mi_ESP32_AP";
const char* password = "12345678";
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

/* ------------------------------------------------------ HTML ------------------------------------------------------ */
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta charset="UTF-8">
<title>ESP32 - Huerta Inteligente</title>
<style>

body{
    font-family:Arial,Helvetica,sans-serif;
    margin:10px;
    background:#222;
    color: #fff;
}
h1{
    text-align: center;
    margin: 30px;
}
.contenedor{
    display: flex;
    margin: auto;
    width: 100%;
    flex-flow: row wrap;
    justify-content: center;
    gap: 10px;
}
.card{
    background:#222;
    padding:12px;
    border-radius:8px;
    box-shadow:0 1px 3px rgba(0,0,0,0.12);
    margin-bottom:10px;
    flex: 0 1 150px;
}
.row{
    display:flex;
    gap:10px;
    flex-wrap:wrap;
    width: 6rem;
    flex: 1 0 4rem;
    border: 2px solid #fff;
    border-radius: 8px;
}
#grafica-temperatura{
    border: 2px solid #fff;
    border-radius: 10px;
    padding: 20px;
    padding-bottom: 20px;
    margin-top: 20px;
}
.value{
    font-size:1.6rem;
    font-weight:700;
}
#tempChart{
    width:100%;
    height:220px;
    display:block;
    border: 2px solid #fff;
    border-radius: 10px;
}
#uvChart{
    width:100%;
    height:220px;
    display:block;
    border: 2px solid #fff;
    border-radius: 10px;
}
#humChart{
    width:100%;
    height:220px;
    display:block;
    border: 2px solid #fff;
    border-radius: 10px;
}
.legend{
    font-size:0.9rem;
    margin-top:15px;
    color:#bbb;
}
th, td {
    border: 1px solid #77f;
    padding: 5px;
    text-align: center;
    font-family: sans-serif;
}
.tabla__datos {
    border: 2px solid #77f;
}
.table {
    overflow-x: scroll;
}
table::-webkit-scrollbar-thumb{
    background-color: #222;
    color: #222
}
table::-webkit-scrollbar-track{
    color: #222;
    background-color: #222;
}
button{
    border-radius: 15px;
    padding: 10px;
    background-color: #222;
    color: #fff;
    margin-top: 20px;
    border: 2px solid #fff;
}
</style>
</head>
<body>
<h1>ESP32 Monitor De Huerta</h1>
    <div class="contenedor">
    <div class="row">
        <div class="card" style="flex:1 1 140px">
            <div>Temperatura Ambiente</div>
            <div id="tempValue" class="value">-- °C</div>
        </div>
    </div>

    <div class="row">
        <div class="card" style="flex:1 1 140px">
            <div>Humedad Ambiente</div>
            <div id="humValue" class="value">-- %</div>
        </div>
    </div>

    <div class="row">
        <div class="card" style="flex:1 1 140px">
            <div>Humedad Suelo</div>
            <div id="soilValue" class="value">-- %</div>
        </div>
    </div>

    <div class="row">
        <div class="card" style="flex:1 1 140px">
            <div>Temperatura Suelo</div>
            <div id="soilTempValue" class="value">-- °C</div>
        </div>
    </div>

    <div class="row">
        <div class="card" style="flex:1 1 140px">
            <div>LLuvia</div>
            <div id="rainValue" class="value">--</div>
        </div>
    </div>

    <div class="row">
        <div class="card" style="flex:1 1 140px">
            <div>Radiacion UV</div>
            <div id="uvValue" class="value">--</div>
        </div>
    </div>
</div>

<div class="card" id="grafica-temperatura">
    <h2> GRAFICA DE TEMPERATURAS </h2>
    <canvas id="tempChart"></canvas>
    <div class="legend">
    Naranja = Temperatura Ambiente (°C) |
    Rojo = Temperatura Suelo (°C)
</div>

<div class="card" id="grafica-humedad">
    <h2> GRAFICA DE HUMEDADES </h2>
    <canvas id="humChart"></canvas>
    <div class="legend">
    Azul = Humedad Ambiente (%) |
    Verde = Humedad Suelo (%) 
</div>

<div class="card" id="grafica-uv">
    <h2> GRAFICA DE UV </h2>
    <canvas id="uvChart"></canvas>
    <div class="legend">
    Morado = radiacion uv |
</div>

<div class="card, table">
    <h2>Data Table</h2>
    <table class="tabla__datos" id="tabla">
        <tr id="Linea__Tiempo">
            <th class="tableH">TIEMPO (s)</th>
        </tr>
        <tr id="Linea__Temp__Suelo">
            <th class="tableH">TEMPERATURA <br> SUELO (°C)</th>
        </tr>
        <tr id="Linea__Temp__Ambiente">
            <th class="tableH">TEMPERATURA <br> AMBIENTE (°C)</th>
        </tr>
        <tr id="Linea__Hum__Suelo">
            <th class="tableH">HUMEDAD <br> SUELO (%)</th>
        </tr>
        <tr id="Linea__Hum__Ambiente">
            <th class="tableH">HUMEDAD <br> AMBIENTE (%)</th>
        </tr>
        <tr id="Linea__uv">
            <th class="tableH">NIVEL DE <br> RADIACION UV</th>
        </tr>
    </table>
</div>
    <button onclick="borrarDatos()">Reiniciar Datos</button>
<script>
const maxPoints = 50;

let tempHistory = [];
let soilTempHistory = [];

let humHistory = [];
let soilHistory = [];

let uvHistory = [];
let rainHistory = [];

let timeHistory = [];
let startTime = Date.now();

function addPoint(arr, val){
    arr.push(val);
    if(arr.length > maxPoints) arr.shift();
}

function guardarLocal() {
    const data = {
        tempHistory,
        soilTempHistory,
        humHistory,
        soilHistory,
        timeHistory,
        rainHistory,
        uvHistory,
        tabla: document.getElementById("tabla").innerHTML
    };
    localStorage.setItem("huertaData", JSON.stringify(data));
}

function cargarLocal() {
    const guardado = localStorage.getItem("huertaData");
    if(!guardado) return;

    const data = JSON.parse(guardado);

    tempHistory = data.tempHistory || [];
    soilTempHistory = data.soilTempHistory || [];
    humHistory = data.humHistory || [];
    soilHistory = data.soilHistory || [];
    timeHistory = data.timeHistory || [];
    uvHistory = data.uvHistory || [];
    rainHistory = data.rainHistory || [];

    if(data.tabla) {
        document.getElementById("tabla").innerHTML = data.tabla;
    }

    drawChart("tempChart", [tempHistory, soilTempHistory], ["orange","red"], 0, 1024);
    drawChart("humChart", [humHistory, soilHistory], ["blue","green"], 0, 100);
    drawChart("uvChart", [uvHistory], ["purple"], 0, 14);
}

cargarLocal();

function borrarDatos() {

    // Borrar almacenamiento
    localStorage.removeItem("huertaData");

    // Reiniciar arreglos
    tempHistory = [];
    soilTempHistory = [];
    humHistory = [];
    soilHistory = [];
    uvHistory = [];
    rainHistory = [];
    timeHistory = [];

    startTime = Date.now();

  // Restaurar tabla a su estado inicial
    document.getElementById("tabla").innerHTML = `
    <tr id="Linea__Tiempo"><th>TIEMPO (s)</th></tr>
    <tr id="Linea__Temp__Suelo"><th>TEMPERATURA SUELO (°C)</th></tr>
    <tr id="Linea__Temp__Ambiente"><th>TEMPERATURA AMBIENTE (°C)</th></tr>
    <tr id="Linea__Hum__Suelo"><th>HUMEDAD SUELO (%)</th></tr>
    <tr id="Linea__Hum__Ambiente"><th>HUMEDAD AMBIENTE (%)</th></tr>
    <tr id="Linea__uv"><th class="tableH">NIVEL DE <br> RADIACION UV</th></tr>
    `;

    // Redibujar gráficas vacías para limpiar el canvas
    drawChart("tempChart", [tempHistory, soilTempHistory], ["orange","red"]);
    drawChart("humChart", [humHistory, soilHistory], ["blue","green"], 0, 100);
    drawChart("uvChart", [uvHistory], ["purple"], 0, 14);
}

const socket = new WebSocket(`ws://${location.hostname}:81`);

socket.onmessage = function(event) {
    const d = JSON.parse(event.data);

    const t  = parseFloat(d.temperature);
    const h  = parseFloat(d.humidity);
    const s  = parseFloat(d.soil);
    const st = parseFloat(d.sTemperature);
    const l = d.rain;
    const uv = parseFloat(d.uv);

    const elapsedSeconds = ((Date.now() - startTime) / 1000).toFixed(1);
    const segundos = ((Date.now() - startTime) / 1000).toFixed(0);

    if(!isNaN(t) && !isNaN(h) && !isNaN(s) && !isNaN(st) && !isNaN(uv)) {

    addPoint(tempHistory, t);
    addPoint(humHistory, h);
    addPoint(soilHistory, s);
    addPoint(soilTempHistory, st);
    addPoint(uvHistory, uv);
    addPoint(timeHistory, elapsedSeconds);

    document.getElementById('tempValue').innerText = t.toFixed(2) + ' °C';
    document.getElementById('humValue').innerText  = h.toFixed(2) + ' %';
    document.getElementById('soilValue').innerText = s.toFixed(2) + ' %';
    document.getElementById('soilTempValue').innerText = st.toFixed(2) + ' °C';
    document.getElementById('uvValue').innerText = uv.toFixed(2);

    drawChart("tempChart", [tempHistory, soilTempHistory], ["orange","red"]);
    drawChart("humChart", [humHistory, soilHistory], ["blue","green"], 0, 100);
    drawChart("uvChart", [uvHistory], ["purple"], 0, 14);

    añadirTabla("Linea__Tiempo", segundos);
    añadirTabla("Linea__Temp__Suelo", st.toFixed(1));
    añadirTabla("Linea__Temp__Ambiente", t.toFixed(1));
    añadirTabla("Linea__Hum__Suelo", s.toFixed(1));
    añadirTabla("Linea__Hum__Ambiente", h.toFixed(1));
    añadirTabla("Linea__uv", uv.toFixed(1));

    if (l === 1) {
          document.getElementById('rainValue').innerText = "SI";
          document.getElementById('rainValue').style.color = "green";
        } else if (l === 0) {
          document.getElementById('rainValue').innerText = "NO";
          document.getElementById('rainValue').style.color = "red";
    }

    guardarLocal();
    }
};

function añadirTabla (identidad, dato) {
    const fila = document.getElementById(identidad);
    const celda = document.createElement("td");
    celda.textContent = dato;
    fila.appendChild(celda);
}

function drawChart(canvasId, datasets, colors, minFixed, maxFixed) {
    const canvas = document.getElementById(canvasId);
    
    const ctx = canvas.getContext('2d');

    const rect = canvas.getBoundingClientRect();
    
    const ratio = window.devicePixelRatio || 1;
    
    canvas.width = rect.width * ratio;
    canvas.height = rect.height * ratio;
    
    ctx.setTransform(ratio, 0, 0, ratio, 0, 0);

    const padLeft = 15, padBottom = 15, padTop = 10, padRight = 10;
    const w = rect.width, h = rect.height;
    const innerW = w - padLeft - padRight;
    const innerH = h - padTop - padBottom;

    ctx.clearRect(0, 0, w, h);

    if (datasets[0].length < 2) return;

  // Ejes
    ctx.strokeStyle = '#fff';
    ctx.beginPath();
    ctx.moveTo(padLeft, padTop);
    ctx.lineTo(padLeft, h - padBottom);
    ctx.lineTo(w - padRight, h - padBottom);
    ctx.stroke();

  // Escala
    let allData = datasets.flat();
    let minY = allData ? Math.min(...allData): minFixed;
    let maxY = allData ? Math.max(...allData): maxFixed;
    let rangeY = maxY - minY || 1;

    function drawLine(data, color){
    ctx.beginPath();
    data.forEach((v,i)=>{
        const x = padLeft + innerW * (i/(data.length-1));
        const y = padTop + innerH * (1 - (v - minY)/rangeY);
        if(i===0) ctx.moveTo(x,y);
        else ctx.lineTo(x,y);
    });
    ctx.strokeStyle = color;
    ctx.lineWidth = 3;
    ctx.stroke();
    }

    datasets.forEach((d,i)=> drawLine(d, colors[i]));
}
window.addEventListener('resize', drawChart);
</script>
</body>
</html>
)rawliteral";

/* ------------------------------------------------------ HandldeRoot ------------------------------------------------------ */
void handleRoot() {
    server.send_P(200, "text/html", index_html);
}
/* ------------------------------------------------------ WebSocket ------------------------------------------------------ */

void enviarDatosWebSocket() {

    segundosSistema = millis() / 1000;

    String json = "{";

    json += "\"temperature\":";
    json += isnan(t) ? "null" : String(t,2);
    json += ",";

    json += "\"humidity\":";
    json += isnan(h) ? "null" : String(h,2);
    json += ",";

    json += "\"soil\":";
    json += isnan(s) ? "null" : String(s,2);
    json += ",";

    json += "\"sTemperature\":";
    json += isnan(st) ? "null" : String(st,2);
    json += ",";

    json += "\"uv\":";
    json += isnan(uv) ? "null" : String(uv,2);
    json += ",";

    json += "\"rain\":";
    json += l;
    json += ",";

    json += "\"time\":";
    json += String(segundosSistema);

    json += "}";

    webSocket.broadcastTXT(json);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_CONNECTED:
        Serial.println("Cliente WebSocket conectado");
        break;

        case WStype_DISCONNECTED:
        Serial.println("Cliente WebSocket desconectado");
        break;
    }
}

/* ------------------------------------------------------ SETUP ------------------------------------------------------ */
void setup() {
    Serial.printf("Creando AP: %s\n", ssid);
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    server.on("/", handleRoot);
    server.begin();
    Serial.println("Servidor HTTP iniciado");

    dht.setup(DHTPin, DHTesp::DHT22);

    pinMode(sensor_uv, INPUT);

    pinMode(higrometro, INPUT);
    pinMode(sensor_lluvia, INPUT);

    pinMode(drive1, OUTPUT);
    pinMode(drive2, OUTPUT);

    pinMode(rele1, OUTPUT);
    pinMode(rele2, OUTPUT);

    lcd.init();
    lcd.backlight();

    sim800l.begin(9600);
    Serial.begin(9600);

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    leerSensores();
}

/* ------------------------------------------------------ LOOP ------------------------------------------------------ */
void loop() {
    webSocket.loop();
    server.handleClient();
    unsigned long now = millis();
  /* ------------------------------------------------------ INT_SENSORES = 2000 ------------------------------------------------------ */
    if (now - tSensores >= INT_SENSORES) {
        tSensores = now;
        leerSensores();
        enviarDatosWebSocket();
    }
  /* ------------------------------------------------------ INT_LCD = 1000 ------------------------------------------------------ */
    if (now - tLCD >= INT_LCD) {
        tLCD = now;
        actualizarLCD();
    }
    controlarMotor();
    controlarRele();
    alertas();
}
/* ------------------------------------------------------ FUNCIONES ------------------------------------------------------ */
/* ------------------------------------------------------ LEER SENSORES ------------------------------------------------------ */
void leerSensores() {
    TempAndHumidity d = dht.getTempAndHumidity();
    t = d.temperature;
    h = d.humidity;
    st = ktc.readCelsius();

    lectura_suelo = analogRead(higrometro);
    s = 100.0 * (suelo_seco - lectura_suelo) / (suelo_seco - suelo_humedo);
    s = constrain(s, 0, 100);
    l = digitalRead(sensor_lluvia);

    voltaje = analogRead(sensor_uv);
    uv = voltaje * (3.3 / 4095.0);
    uv = uv * 10;
}
/* ------------------------------------------------------ ACTUALIZAR LCD ------------------------------------------------------ */
void actualizarLCD() {
    lcd.setCursor(0, 0);
    lcd.print("T:" + String(t) + "C");
    lcd.setCursor(0, 1);
    lcd.print("H:" + String(h) + "%");
    lcd.setCursor(9, 0);
    lcd.print("S:" + String(s) + "%");
    lcd.setCursor(9, 1);
    lcd.print("ST:" + String(st) + "°C");
}
/* ------------------------------------------------------ ENCENDER MOTOR ------------------------------------------------------ */
/* ------------------------------------------------------ motorActivo = false ------------------------------------------------------ */
void controlarMotor() {
    unsigned long now = millis();
    if (s <= 50 && !motorActivo) {
        motorActivo = true;
        tMotor = now;
        digitalWrite(drive1, HIGH);
        digitalWrite(drive2, HIGH);
        analogWrite(speed1, 255);
        analogWrite(speed2, 255);
    }
  /* ------------------------------------------------------ ITIEMPO_MOTOR = 5000 ------------------------------------------------------ */
    if (motorActivo && now - tMotor >= TIEMPO_MOTOR) {
        motorActivo = false;
        tMotor = now;
        digitalWrite(drive1, LOW);
        digitalWrite(drive2, LOW);
        analogWrite(speed1, 0);
        analogWrite(speed2, 0);
    }
}
/* ------------------------------------------------------ ENCENDER rele ------------------------------------------------------ */
/* ------------------------------------------------------ estadoRele = false ------------------------------------------------------ */
void controlarRele() {
    unsigned long now = millis();
    if (!estadoRele && t >= 45) {
        estadoRele = true;
        tRele = now;
        digitalWrite(rele1, HIGH);
        digitalWrite(rele2, HIGH);
    }
    /* ------------------------------------------------------ ITIEMPO_RELE = 10000 ------------------------------------------------------ */
    if (estadoRele && now - tRele >= TIEMPO_RELE) {
        tRele = now;
        estadoRele = false;
        digitalWrite(rele1, LOW);
        digitalWrite(rele2, LOW);
    }
}
/* ------------------------------------------------------ ALERTAS ------------------------------------------------------ */
/* ------------------------------------------------------ smsSueloEnviado = false ------------------------------------------------------ */
void alertas() {

    if (s <= 40 && !smsSueloEnviado) {
        enviarSMS("Humedad del suelo baja, revise la parcela");
        smsSueloEnviado = true;
    }
    if (s > 45) {
        smsSueloEnviado = false;
    }


    if (t <= 30 && !smsTempEnviado) {
        enviarSMS("Temperatura ambiente baja, revise la parcela");
        smsTempEnviado = true;
    }
    if (t > 32) {
        smsTempEnviado = false;
    }


    if (st <= 30 && !smsTempSueloEnviado) {
        enviarSMS("Temperatura suelo baja, revise la parcela");
        smsTempEnviado = true;
    }
    if (st > 32) {
        smsTempSueloEnviado = false;
    }


    if (h <= 50 && !smsHumEnviado) {
        enviarSMS("Humedad ambiental baja, revise la parcela");
        smsHumEnviado = true;
    }
    if (h > 55) {
        smsHumEnviado = false;
    }

    if ((uv >= 11 || uv <= 3) && !smsUvEnviado) {
    enviarSMS("La radiacion esta peligrosa en tu huerta");
    smsUvEnviado = true;
    }
    if ((uv > 3 && uv < 11) && smsUvEnviado) {
    smsUvEnviado = false;
    }


    if (l == 1 && !smsLluviaEnviado) {
    enviarSMS("Esta lloviendo en tu huerta");
    smsLluviaEnviado = true;
    }
    if (l == 0) {
    smsLluviaEnviado = false;
    }
}
/* ------------------------------------------------------ ENVIAS SMS ------------------------------------------------------ */
void enviarSMS(String mensaje) {
    sim800l.println("AT");
    SerialCheck();
    sim800l.println("AT+CMGF=1");
    SerialCheck();
    sim800l.println("AT+CMGS=\"+573192506735\"");
    SerialCheck();
    sim800l.print(mensaje);
    SerialCheck();
    sim800l.write(26);
}
void SerialCheck() {
    unsigned long now = millis();
    if (now - tSMS >= TIEMPO_SMS){
        while (Serial.available()) {
        sim800l.write(Serial.read());
        }
        while (sim800l.available()) {
        Serial.write(sim800l.read());
        }
    }
}
