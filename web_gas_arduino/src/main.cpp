#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#define pinoAnalogico A0
#define pinoDigital 7

int buzzerPin = 8;
SoftwareSerial esp01s(2, 3); // RX, TX
const char *host = "192.168.1.74";

// http://192.168.1.74:8080
// http://192.168.163.1:8080 celular
const int port = 8080;

const char *rede_name = "*";
const char *password = "*";

void send_notification(String endpoint);

bool send_command(String command, int timeout = 1000)
{
  long int time = millis();
  esp01s.println(command);
  String response = "";

  while ((millis() - time) < timeout)
  {
    while (esp01s.available())
    {
      response += (char)esp01s.read();
    }
    if (response.indexOf("OK") != -1 || response.indexOf("CONNECT") != -1)
    {
      Serial.println(command + " OK");
      return true;
    }
  }

  Serial.println(command + " ERRO: " + response);
  return false;
}

void setup()
{
  Serial.begin(9600);
  // Testa diferentes baud rates para o ESP-01S
  long baudRates[] = {115200, 9600, 57600, 38400};
  bool espOk = false;
  for (int i = 0; i < 4 && !espOk; i++) {
    esp01s.begin(baudRates[i]);
    delay(500);
    espOk = send_command("AT");
    if (espOk) {
      Serial.print("ESP-01S respondendo em baud rate: ");
      Serial.println(baudRates[i]);
      break;
    }
  }
  if (!espOk) {
    Serial.println("ESP-01S não respondeu em nenhum baud rate!");
    while (1);
  }
  Wire.begin();
  pinMode(pinoAnalogico, INPUT);
  pinMode(pinoDigital, INPUT);
  send_command("AT+RST");
  send_command("AT+CWMODE=1");
  send_command("AT+CIPMODE=0");
  String connectWiFi = "AT+CWJAP=\"" + String(rede_name) + "\",\"" + String(password) + "\"";
  bool is_connected = send_command(connectWiFi, 5000);
  while (!is_connected)
  {
    is_connected = send_command(connectWiFi, 5000);
    if (is_connected)
    {
      Serial.println("Conectado ao Wi-Fi.");
    }
  }
  pinMode(buzzerPin, OUTPUT);
  tone(buzzerPin, 220, 250);
}

void loop()
{

  int leitura_analogica = analogRead(pinoAnalogico);
  int leitura_digital = digitalRead(pinoDigital);

  Serial.print("leitura do sensor: ");
  Serial.println(leitura_analogica);
  Serial.println(leitura_digital);

  send_notification("/update?ppm=" + String(leitura_analogica));

  if (leitura_digital == LOW)
  {
    Serial.println("Gás tóxico detectado!");
    tone(buzzerPin, 300, 250);
    send_notification("/update?ppm=" + String(leitura_analogica));
    tone(buzzerPin, 500, 250);
  }

  delay(2000);
}

void send_notification(String endpoint)
{
  send_command("AT+CIPCLOSE", 1000);
  bool send_msg = false;

  while (!send_msg)
  {
    String connectCmd = "AT+CIPSTART=\"TCP\",\"" + String(host) + "\"," + String(port);
    if (send_command(connectCmd, 5000))
    {
      Serial.println("Conectado ao servidor.");

      String httpRequest = "GET " + String(endpoint) + " HTTP/1.1\r\n";
      httpRequest += "Host: " + String(host) + "\r\n";
      httpRequest += "Connection: close\r\n\r\n";
      String sendCmd = "AT+CIPSEND=" + String(httpRequest.length());
      if (send_command(sendCmd, 5000))
      {
        esp01s.print(httpRequest);
        Serial.println("Notificação enviada.");
        send_msg = true;
      }
      else
      {
        Serial.println("Falha ao enviar dados.");
      }
    }
    else
    {
      Serial.println("Falha ao conectar ao servidor.");
    }

    delay(2000);
  }
}