# Projeto: Monitoramento de Temperatura com ESP8266 e Display OLED


## Itens do projeto 
Este projeto utiliza um **ESP8266**, um **display OLED (U8g2)** e um **multiplexador (MUX)** para leitura de múltiplos sensores de temperatura, exibindo os valores em tela e enviando periodicamente os dados para um servidor PHP.

---

## Funcionalidades

* Leitura de múltiplos sensores via MUX.
* Exibição das temperaturas no display OLED.
* Envio periódico dos dados para um servidor PHP (requisições HTTP).
* Conexão Wi‑Fi em modo **STA** e configuração opcional em modo **AP**.
* Servidor web interno para visualizar informações.

---

## Tecnologias e Bibliotecas Utilizadas

* **ESP8266WiFi.h** – conexão à rede Wi‑Fi.
* **ESP8266WebServer.h** – servidor HTTP interno.
* **ESP8266HTTPClient.h** – envio de requisições ao servidor.
* **Wire.h** – comunicação I2C.
* **U8g2lib.h** – controle do display OLED.

---

## Configuração de Rede

```cpp
const char* sta_ssid = "BANDOCH";
const char* sta_password = "SENAI_IOT";
const char* servidorPHP = "192.168.0.100";  // IP do servidor PHP/XAMPP
```

O sistema envia dados automaticamente a cada **20 segundos**:

```cpp
unsigned long intervalo_envio = 20000;
```

---

## Display OLED

O display mostra **temperaturas individuais** e outras informações conforme sua implementação.
A biblioteca U8g2 permite:

* Textos,
* Gráficos,
* Ajuste de fontes

---

## Multiplexador (MUX)

A função abaixo seleciona o canal do sensor:

```cpp
void selecionarCanal(int porta) {
    // lógica para ativação dos pinos S0, S1, S2, S3
}
```

> Isso permite ler vários sensores com poucos pinos digitais.

---

## Servidor Web Interno

O ESP8266 cria um servidor local:

```cpp
ESP8266WebServer server(80);
```

Com isso, você pode acessar o IP do microcontrolador no navegador e ver informações ou realizar ações.

---

## Envio dos Dados ao Servidor PHP

O ESP realiza chamadas HTTP, enviando os valores dos sensores para um endpoint PHP que registra os dados.

Exemplo simplificado:

```cpp
HTTPClient http;
http.begin("http://192.168.0.100/salvar.php?temp1=...&temp2=...");
http.GET();
```

---
