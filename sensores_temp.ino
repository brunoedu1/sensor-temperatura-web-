#include <U8g2lib.h>            // display OLED
#include <Wire.h>               // comunicação I2C (display)
#include <ESP8266WiFi.h>        //  conectar o ESP8266 ao Wi-Fi
#include <ESP8266WebServer.h>   // servidor HTTP no ESP
#include <ESP8266HTTPClient.h>  //  requisições HTTP (GET/POST)

//config. da rede wi-fi (pc do professor)

const char* sta_ssid = "BANDOCH";           // nome da rede
const char* sta_password = "SENAI_IOT";     // senha da rede
const char* servidorPHP = "192.168.0.100";  // ip do servidor (xampp)

unsigned long ultimo_envio = 0;         // guarda o ultimo envio
unsigned long intervalo_envio = 20000;  // "delay" de 20s

// config do modo ap do ESP8266
const char* ssid = "Bruno_Souza";      // wi-fi criado pelo esp8266
const char* password = "senha_senha";  // senha do wi-fi criado pelo esp8266

// lm35 conectados ao MUX
const int SENSOR_TEMP[] = { 0, 1, 2, 3 };  // canais do MUX
const int muxPins[] = { D6, D5, D7 };      // pinos que controlam o MUX

float temp[3], analogVal[3];  // guarda temperaturas e as leituras analogicas

// display OLED
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C displayLED(U8G2_R0, U8X8_PIN_NONE);

//  servidor web na porta 80
ESP8266WebServer server(80);


String s1, s2, s3;

// ------------------------------------------------------
// MUX
// ------------------------------------------------------
void selecionarCanal(int porta) {
  // canal 1
  if (porta == SENSOR_TEMP[1]) {
    digitalWrite(muxPins[0], 0);  // S0 = 0
    digitalWrite(muxPins[1], 0);  // S1 = 0
    digitalWrite(muxPins[2], 0);  // S2 = 0
  }

  // canal 2
  else if (porta == SENSOR_TEMP[2]) {
    digitalWrite(muxPins[0], 0);  // S0 = 0
    digitalWrite(muxPins[1], 1);  // S1 = 1
    digitalWrite(muxPins[2], 0);  // S2 = 0
  }

  // canal 3
  else {
    digitalWrite(muxPins[0], 0);  // S0 = 0
    digitalWrite(muxPins[1], 1);  // S1 = 1
    digitalWrite(muxPins[2], 1);  // S2 = 1
  }
}

// ------------------------------------------------------
// Leitura das temperaturas
// ------------------------------------------------------
void calcularTemperaturas() {
  for (int i = 1; i <= 3; i++)  // loop para percorrer os 3 sensores
  {
    selecionarCanal(SENSOR_TEMP[i]);  // vai escolher o canal do MUX correspondente ao sensor
    delay(5);                         // delay ;-;

    analogVal[i] = analogRead(A0);  // le o valor

    temp[i] = ((100.0 * analogVal[i] * 3.3) / (float)1023.0);
    // converte o valor lido em temperatura (celsos)
  }
}

// ------------------------------------------------------
// Display OLED
// ------------------------------------------------------
void printDisplay()

{
  displayLED.clearBuffer();

  displayLED.setCursor(0, 10);
  displayLED.print("S1: ");
  displayLED.print(temp[1], 1);

  displayLED.setCursor(0, 20);
  displayLED.print("S2: ");
  displayLED.print(temp[2], 1);

  displayLED.setCursor(0, 30);
  displayLED.print("S3: ");
  displayLED.print(temp[3], 1);

  displayLED.sendBuffer();
}

// ------------------------------------------------------
// Página principal HTML
// ------------------------------------------------------
void paginaPrincipal() {
  String pagina = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="UTF-8">
    <title>Temperaturas  - NodeMCU</title>
    <style>
      body { font-family: Arial; text-align: center; }
      .box {
        width: 260px; margin: 0 auto;
        padding: 15px; border: 2px solid #333;
        border-radius: 10px;
      }
      .linha { font-size: 1.2em; margin: 10px 0; }
    </style>
    <script>
      function atualizar() {
        fetch('/dados')
          .then(r => r.json())
          .then(d => {
            document.getElementById('s1').textContent = d.s1;
            document.getElementById('s2').textContent = d.s2;
            document.getElementById('s3').textContent = d.s3;
          });
      }
      setInterval(atualizar, 500);
      window.onload = atualizar;
    </script>
  </head>
  <body>
    <h2>Temperaturas Sensores</h2>

    <div class="box">
      <div class="linha"><b>Sensor 1:</b> <span id="s1">---</span></div>
      <div class="linha"><b>Sensor 2:</b> <span id="s2">---</span></div>
      <div class="linha"><b>Sensor 3:</b> <span id="s3">---</span></div>
    </div>

  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", pagina);
}

//----------------------- Função para envio pro BD -----------------------------
void enviarBanco(String t1, String t2, String t3) {  //Função de envio dos dados que recebe 3 Strings como parâmetro.
  if (WiFi.status() == WL_CONNECTED) {               //Condicional que testa se o Node está conectado à rede "BANDOCH"
    WiFiClient client;
    HTTPClient http;
    String url = "http://" + String(servidorPHP) + "/api/inserir_dados_noite.php";
    url += "?sistema_id=Bruno_Souza";                               // Trocar "PROF_VINI" pelo seu nome!!!
    url += "&sensor1=" + t1 + "&sensor2=" + t2 + "&sensor3=" + t3;  //Essa URL define a execução do script php (que organiza as informações e manda-as para o banco de dados)

    http.begin(client, url);
    int httpCode = http.GET();  // Envia a requisição GET
    if (httpCode > 0) {         //Essa condicional testa o valor do código de retorno HTTP, onde 200 é HTTP OK
      Serial.println("Dados enviados ao banco com sucesso!");
      Serial.println(httpCode);
    } else {
      Serial.println("Falha ao enviar dados ao banco!");
      Serial.println(http.errorToString(httpCode));
    }
    http.end();
  } else {
    Serial.println("WiFi (modo STA) desconectado. Nao foi possivel enviar ao banco.");
  }
}
//------------------------------------------------------------------------------

// ------------------------------------------------------
// SETUP
// ------------------------------------------------------
void setup() {
  Serial.begin(9600);

  //----------------------- Configuração do Node como AP-STA -----------------------------
  WiFi.mode(WIFI_AP_STA);              //Configuração de rede do Node como modo híbrido: Access Point + Station
  WiFi.begin(sta_ssid, sta_password);  //Conexão do Node à rede Wi-Fi identificada anteriormente
  Serial.print("Conectando-se à rede local ");
  Serial.println(sta_ssid);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {  //Loop que tenta conectar o Node à rede e incrementa a variável "tentativas" toda vez que a conexão não é bem sucedida
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {  //Condicional que testa se foi possível conectar o Node à rede
    Serial.println("\nConectado à rede local!");
    Serial.print("IP local do Node: ");
    Serial.println(WiFi.localIP());  //Imprime o endereço IP do Node dentro da rede local "BANDOCH"
  } else {
    Serial.println("\nFalha ao conectar à rede local.");
  }
  //--------------------------------------------------------------------------------------

  // Display
  displayLED.begin();
  displayLED.setPowerSave(0);
  displayLED.setFont(u8g2_font_5x7_tf);

  IPAddress local_ip(192, 168, 10, 1);  // define o ip do NodeMCU quando estiver em AP
  IPAddress gateway(192, 168, 10, 1);   // define o geteway
  IPAddress subnet(255, 255, 255, 0);   // define a mascara de rede


  // AP
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  Serial.println("AP iniciado em: " + WiFi.softAPIP().toString());

  // Rotas
  server.on("/", paginaPrincipal);

  server.on("/dados", []() {
    String json = "{";

    json += "\"s1\":\"" + s1 + "\",";
    json += "\"s2\":\"" + s2 + "\",";
    json += "\"s3\":\"" + s3 + "\"}";
    server.send(200, "application/json", json);
  });

  server.begin();

  pinMode(muxPins[0], OUTPUT);
  pinMode(muxPins[1], OUTPUT);
  pinMode(muxPins[2], OUTPUT);
}

// ------------------------------------------------------
// LOOP
// ------------------------------------------------------
void loop() {
  calcularTemperaturas();
  printDisplay();

  // prepara strings para o JSON

  s1 = String(temp[1], 1);
  s2 = String(temp[2], 1);
  s3 = String(temp[3], 1);

  server.handleClient();
  unsigned long tempo_atual = millis();

  if (tempo_atual - ultimo_envio > intervalo_envio) {
    ultimo_envio = tempo_atual;
    enviarBanco(s1, s2, s3);  //Chamada da função de envio do BD a cada 20 segundos
    //Trocar "temp1", "temp2" e "temp3" pelas variáveis de armazenamento de temperatura que você definiu no seu código
  }
  delay(500);
}