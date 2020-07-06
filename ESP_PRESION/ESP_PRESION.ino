#define PRES1_PIN 34
#define PRES2_PIN 32

#include <WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "ADELE";
const char* password = "zucchini";
const char* mqtt_server = "broker.shiftr.io";
const float  OffSet = 0.058 ;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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


}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
  if (client.connect(clientId.c_str(),"33cbb1f4","48d5e98e8266d3e0")) {
      Serial.println("connected");
      // Once connected, publish an announcement...

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
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  float v_1, p_1, v_2, p_2;
  String presion1, presion2;
  v_1 = analogRead(34)*3.3/4096.0;     
  p_1 = (v_1 - OffSet) * 400;

  v_2 = analogRead(32)*3.3/4096;
  p_2 =(v_2 - OffSet)*400;

  Serial.print("Voltaje 1:");
  Serial.print(v_1, 3);
  Serial.println("V");

  Serial.print("Presion 1:");
  Serial.print(p_1, 2);
  Serial.println(" KPa");
  Serial.println();
  presion1=String(p_1,2);
  client.publish("pressure_1",presion1.c_str());
  
  delay(500);
  
  Serial.print("Voltaje 2:");
  Serial.print(v_2, 3);
  Serial.println("V");

  Serial.print("Presion 2:");
  Serial.print(p_2, 2);
  Serial.println(" KPa");
  Serial.println();
  presion2=String(p_2,2);
  client.publish("pressure_2",presion2.c_str());
  
  delay(500);
}
