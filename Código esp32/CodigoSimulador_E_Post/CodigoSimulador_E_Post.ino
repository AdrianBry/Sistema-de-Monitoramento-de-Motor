#include <WiFi.h>
#include <HTTPClient.h>
#include <vector>

// =====================================
// WIFI
// =====================================

const char* ssid = "WifidoBAJA";
const char* password = "12345678";

const char* servidorNode =
"https://reposit-rio.onrender.com/update";


// PWM VENTOINHA
const int FAN_PIN = 2;
const int PWM_FREQ = 25000;
const int PWM_RESOLUTION = 8;

std::vector<float> temp_motor_array;
std::vector<float> time_array;

unsigned long lastTime = 0;
unsigned long LastArrayTime = 0;
float temp_motor = 0;
int rpm = 0;
int ventoinha = 0;


// ARRAY -> JSON
String arrayToJson(const std::vector<float>& arr) {

  String json = "[";

  for (size_t i = 0; i < arr.size(); i++) {
    json += String(arr[i], 2);

    if (i < arr.size() - 1) {
      json += ",";
    }
  }
  json += "]";
  return json;
}

// GERA DADOS ALEATÓRIOS
void gerarValores() {

  // TEMPERATURA ENTRE 70°C E 120°C
  temp_motor = random(700, 1200) / 10.0;

  // RPM ENTRE 1500 E 7000
  rpm = random(1500, 7000);

  Serial.println("====================");

  Serial.print("Temp Motor: ");
  Serial.print(temp_motor);
  Serial.println(" °C");

  Serial.print("RPM: ");
  Serial.println(rpm);
}

// CONTROLE PWM DA VENTOINHA
void controlarVentoinha() {
  /* Range de Temperatura do motor de carro de passeio
      < 80     = desligada
      80-90    = baixa
      90-105   = média
      105-110  = alta
      > 110    = máxima
  */

  if (temp_motor < 80) {
    ventoinha = 0;
  }

  else if (temp_motor < 90) {
    ventoinha = map(temp_motor * 10, 800, 900, 20, 40);
  }

  else if (temp_motor < 105) {
    ventoinha = map(temp_motor * 10, 900, 1050, 40, 75);
  }

  else if (temp_motor < 110) {
    ventoinha = map(temp_motor * 10, 1050, 1100, 75, 90);
  }

  else {
    ventoinha = 100;
  }

  // CONVERSÃO PWM 0-255
  int pwmValue = map(ventoinha, 0, 100, 0, 255);

  // ESCREVE PWM
  ledcWrite(FAN_PIN, pwmValue);

  Serial.print("Ventoinha: ");
  Serial.print(ventoinha);
  Serial.println("%");

  Serial.print("PWM: ");
  Serial.println(pwmValue);
}

void atualizarArrays() {

  if (millis() - LastArrayTime >= 60000) {
    temp_motor_array.push_back(temp_motor);
    time_array.push_back(millis() / 60000.0);
    LastArrayTime = millis();
  }
}

void enviarDadosServidor() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(servidorNode);
    http.addHeader("Content-Type","application/json");

    String json = "{";json += "\"temp_motor\":"  + String(temp_motor, 2) + ",";
    json += "\"rpm\":" + String(rpm) + ",";
    json += "\"ventoinha\":" + String(ventoinha) + ",";
    json += "\"temp_motor_array\":" + arrayToJson(temp_motor_array) + ",";
    json += "\"time_array\":" + arrayToJson(time_array);
    json += "}";
  
    int httpCode = http.POST(json);

    Serial.print("HTTP Code: ");
    Serial.println(httpCode);

    http.end();
  }
}

void manterConexaoWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.print("Conectando WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(1000);
    }

    Serial.println();
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }
}

void Callback() {
  if (millis() - lastTime >= 2000) {
    gerarValores();
    controlarVentoinha();
    atualizarArrays();
    enviarDadosServidor();
    lastTime = millis();
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));
  // PWM
  ledcAttach(FAN_PIN, PWM_FREQ, PWM_RESOLUTION);
  WiFi.mode(WIFI_STA);

  manterConexaoWiFi();
}

void loop() {
  manterConexaoWiFi();
  Callback();
}