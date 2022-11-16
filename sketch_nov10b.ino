#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Fibertel WiFi 3me4 2.4GHZ";
const char* password = "8duvdbx2";
const char* mqtt_server = "nd199c1c.us-east-1.emqx.cloud";

unsigned long dataMillis = 0;
bool estadoAnterior = false;
bool estado = false;

const int pinTrigger = 14;  //Variable que contiene el número del pin al cual conectamos la señal "trigger" pin D5
const int pinEcho = 13;     //Variable que contiene el número del pin al cual conectamos la señal "echo"  pin D7

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int contlleno = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);  // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";

    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), "esp8266", "esp8266")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(pinTrigger, OUTPUT);    //Configuramoms el pin de "trigger" como salida
  pinMode(pinEcho, INPUT);        //Configuramoms el pin de "echo" como entrada
  digitalWrite(pinTrigger, LOW);  //Ponemos en voltaje bajo(0V) el pin de "trigger"

  pinMode(BUILTIN_LED, OUTPUT);  // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 15354);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long t;  //Variable de tipo unsigned long que contendrá el tiempo que le toma a la señal ir y regresar
  long d;          //Variable de tipo float que contendrá la distancia en cm

  digitalWrite(pinTrigger, HIGH);  //Ponemos en voltaje alto(5V) el pin de "trigger"
  delayMicroseconds(10);           //Esperamos en esta línea para conseguir un pulso de 10us
  digitalWrite(pinTrigger, LOW);   //Ponemos en voltaje bajo(0V) el pin de "trigger"

  t = pulseIn(pinEcho, HIGH);        //Utilizamos la función  pulseIn() para medir el tiempo del pulso/echo
  d = t * 0.000001 * 34300.0 / 2.0;  //Obtenemos la distancia considerando que la señal recorre dos veces la distancia a medir y que la velocidad del sonido es 343m/s

  Serial.print("Distancia: ");
  Serial.print(d);
  Serial.print("cm");
  Serial.println();

  /*if(d<10)
  //{
  //  contlleno++;

    if(contlleno > 3000){
      Serial.println("El colillero esta lleno");
      Serial.println();
    }
  }
  else{
    contlleno=0;
  }*/

  //Si la distancia es menor a 10 se cuenta una colilla
 /*
  if(d <= 10)
  {
    snprintf(msg, MSG_BUFFER_SIZE, "Ingreso un colilla");
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("esp8266/sensor/level", msg);
  }
  */

  //si el sensor mide una distancia menor a 10 cm durante 30 seg manda un aviso
  //*
  if (d<10){
    contlleno++;

    if(contlleno > 3000){
      snprintf(msg, MSG_BUFFER_SIZE, "El colillero esta lleno");
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("esp8266/sensor/motion", msg);
    }
    
  }
  else{
    contlleno = 0;
  }
  //*/

  delay(100);
}