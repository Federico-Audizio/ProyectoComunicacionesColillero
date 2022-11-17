#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Datos del wifi que se quiere conectar
const char* ssid = "Utn_Libre Max";
const char* password = "";
//Url del servidor
const char* mqtt_server = "nd199c1c.us-east-1.emqx.cloud";

/*
unsigned long dataMillis = 0;
bool estadoAnterior = false;
bool estado = false;*/

#define colilla D1
#define lleno D5
#define pinLED 2

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

int col, last_col = 1, full = 0, last_full = 0, full_Counter = 0;

void setup_wifi() {

  delay(10);
  // Inicia la conexion a Wifi
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
  // Trata de conectarse en un loop a el servidor
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";

    clientId += String(random(0xffff), HEX);
    // Verifica la conexion
    if (client.connect(clientId.c_str(), "esp8266", "esp8266")) {
      Serial.println("connected");
      // Se conecto con exito

      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      //si no se conecto espera 5 segundos y vuelve a probar
      delay(5000);
    }
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(pinLED, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(colilla, INPUT);     // Inicializa los pines en ingreso de datos
  pinMode(lleno, INPUT);
  digitalWrite(pinLED, LOW);   // Inicializa el pin el bajo(apagado)

  setup_wifi();
  client.setServer(mqtt_server, 15354);
  client.setCallback(callback);
}

void loop() {

  //Si el cliente no esta conectado el servidor lo envia a conectarse
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //lee lo que ingresar en el sersor de la colilla      
  col = digitalRead(colilla);
  
  //Verifica que cambie el estado en relacion al estado anterior  
  if (col != last_col) {
    // 
    if (col == LOW) {
      //Ingresa si el boton marca un bajo, que seria cuando ingresa una colilla, y lo envia a la base de datos
      /*      
      snprintf(msg, MSG_BUFFER_SIZE, "Ingreso un colilla");
      Serial.print("Publish message: ");
      Serial.println(msg);
      */
      client.publish("esp8266/sensor/motion", "");

    }
    delay(50);
  }
  // Cambia el estado anterior y lo iguala al alcual para utilizarlo en el proximo loop
  last_col = col;

  //lee el sensor de capacidad
  full = digitalRead(lleno);
  if (full == LOW) {
    // si el sensor registra un bajo inicializa el contador
    full_Counter++;
    /*
    Serial.print("Contador: ");
    Serial.println(full_Counter);
    */
  }
  else {
    if (full != last_full) {
      // Si el estado "lleno" cambia envia a la base de datos que el colillero se vacio
      //Reestablece el contador en 0
      full_Counter = 0;
      //Apaga el led
      digitalWrite(pinLED, HIGH);
      /*
      snprintf(msg, MSG_BUFFER_SIZE, "Vacio");
      Serial.print("Publish message: ");
      Serial.println(msg);
      */
      client.publish("esp8266/sensor/empty", "");
    }
  }
  //Actualiza el estado a vacio
  last_full = full;

  //Verifica si el contador de lleno llega a 500
  if (full_Counter == 500) {
    //Si llega a 500 envia que el colillero esta lleno a la base de datos
    //Enciende el Led para que se vea que esta lleno
    digitalWrite(pinLED, LOW);
    /*    
    snprintf(msg, MSG_BUFFER_SIZE, "El colillero esta lleno");
    Serial.print("Publish message: ");
    Serial.println(msg);
    */
    client.publish("esp8266/sensor/level", "");
  }
  delay(50);
}
