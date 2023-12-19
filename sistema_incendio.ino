// versão com method post, buzzer e leds

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#define buzzer D2
#define redTemp D6
#define greenTemp D4
#define redGasFu D5
#define greenGasFu D8

String StatusLeitura;

/*** Sensor DTH11 ***/ 
#include "DHT.h"
#define DHTPIN 13
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);
int tempMax = 30;

/*** Sensor chama ***/ 
int sensorChama = D1; 

/*** Sensor MQ-2 ***/ 
int pinSensor = A0; //Pino Sensor
int var = 0;
int ValDesarm = 20; //Variável para selecionar a quantidade de Gás/Fumaça detectada

const char *ssid = "REDE-WIFI";
const char *password = "SENHA-WIFI";
const int http_port = 8080;
WiFiClient client;

void setup() {
  pinMode(sensorChama, INPUT); //Configuração do pino do sensor como entrada
  dht.begin(); // inicializa sensor dht
  Serial.begin(9600); // Configuração da comunicação serial

  pinMode(buzzer, OUTPUT);
  pinMode(redTemp, OUTPUT);
  pinMode(greenTemp, OUTPUT);
  pinMode(redGasFu, OUTPUT);
  pinMode(greenGasFu, OUTPUT);

  WiFi.mode(WIFI_OFF);delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println(".");
  Serial.print("Conectando...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);Serial.print(".");}
    Serial.println ("");
    Serial.print ("Conectado a ");
    Serial.println (ssid);
    Serial.print ("Endereço IP: ");
    Serial.println (WiFi.localIP ());
    Serial.print("MAC: ");    
    Serial.println(WiFi.macAddress()); 
}

void envia(String postData){
    HTTPClient http;
    http.begin(client,"http://trabalhofinal.migueldebarba.com.br/DaDaCa/piloto_insert.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.POST(postData);
    String payload = http.getString();
    Serial.println("httpCode:"+String(httpCode));
    Serial.println("payload:"+payload);
    http.end();
    if (payload=="OK"){digitalWrite(D0,HIGH);delay(1000);}
    digitalWrite(D0,LOW);
}

void loop() {
  String Mac=WiFi.macAddress(); 

  /*** Sensor chama ***/ 
    int SensorChama = digitalRead(sensorChama);// Passando o valor obtido no sensor para a variável "leituraSensor"
    Serial.print("Chama: "); 
    Serial.println(SensorChama);
    // Se for detectado o espectro IR da chama imprime a mensagem 
    if (SensorChama == LOW) {  
      Serial.println("Sensor de chama detectou fogo!"); 
      delay(500);  
    } // Se NÃO for detectado o espectro IR da chama imprime no serial a mensagem 
    else { 
      Serial.println("Sensor de chama ok."); 
      delay(500); 
    }
    Serial.println(" "); delay(300);

  /*** Sensor MQ-2 ***/ 
    int SensorGasFu = analogRead(pinSensor); //Faz a leitura da entrada do sensor
    SensorGasFu = map(SensorGasFu, 0, 1023, 0, 100); //Faz a conversão da variável para porcentagem
    Serial.print("GasFu: "); 
    Serial.print(SensorGasFu); 
    Serial.println(" %");
    if (SensorGasFu>=ValDesarm){ //Condição, se SensorGasFu continuar maior que ValDesarm faça:
        Serial.println("Sensor MQ-2 detectou gás ou fumaça!"); //Apresenta mensagem na porta serial
        delay(500); //Tempo de disparo do alarme
    }else{
        Serial.println("Sensor MQ-2 ok.");
        delay(500); 
    }
    Serial.println(" "); delay(300);

  /*** Sensor DTH11 ***/ 
    float SensorTemp = dht.readTemperature();
    Serial.print("Temperatura: ");
    Serial.print(SensorTemp);
    Serial.println(" °C ");
    if(SensorTemp >= tempMax){
      Serial.println("Sensor temperatura alta!");
    }
    else{
      Serial.println("Sensor temperatura ok.");
    }
  Serial.println(" "); delay(300); 

  String postData = 
    "MMac="+String(Mac)+"&Ssensorchama="+String(SensorChama)+"&Ssensorgasfu="+String(SensorGasFu)+"&Ssensortemp="+String(SensorTemp)+"&Sstatusleitura="+String(StatusLeitura);
  envia(postData); 

  Serial.println(" "); 
  delay(100);//intervalo entre as leituras e respostas

// lógica separada para ativar buzzer e leds
  // chama
  if (SensorChama == LOW){
    digitalWrite(buzzer, HIGH);
    digitalWrite(buzzer, LOW); 
  }
// gasFu
  if (SensorGasFu>=ValDesarm){
    digitalWrite(greenGasFu, LOW);
    digitalWrite(redGasFu, HIGH); 
  }
  else if (SensorGasFu<ValDesarm){
    digitalWrite(redGasFu, LOW);
    digitalWrite(greenGasFu, HIGH); 
  }
// temp
  if(SensorTemp >= tempMax){
    digitalWrite(greenTemp, LOW); 
    digitalWrite(redTemp, HIGH); 
  }
  else if (SensorTemp<tempMax){
    digitalWrite(redTemp, LOW);
    digitalWrite(greenTemp, HIGH); 
  }
  //buzzer tempFu
  if (SensorGasFu>ValDesarm && SensorTemp>tempMax){
    digitalWrite(buzzer, HIGH); 
    digitalWrite(buzzer, LOW); 
  }
  // status
  if(SensorTemp<33 && SensorGasFu<ValDesarm){
    StatusLeitura = "Normal";
  }
  else if (SensorGasFu>=ValDesarm || SensorTemp>=33){
    StatusLeitura = "Risco";
  }
  else if(SensorTemp>35 || SensorGasFu>30 || SensorChama==LOW){
    StatusLeitura = "Alto risco";
  } 

}