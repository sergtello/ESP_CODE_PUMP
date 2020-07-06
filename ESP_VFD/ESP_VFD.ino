#define DAC_PIN 25
#define START_STOP_PIN 13
#define DIR_PIN 14

#include <WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "ADELE";
const char* password = "zucchini";
const char* mqtt_server = "broker.shiftr.io";

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
  char tilin[5];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    tilin[i]= (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if(strcmp("variador",topic)==0){
    int dac_value=0;
    float volt_value=0.0;
    String msg = String(tilin);
    //Serial.println(msg);
    dac_value= (msg.toInt()*255)/60;
    volt_value=dac_value*3.3/255.0;
    Serial.print("Voltaje ESP: " );
    Serial.print(volt_value);
    Serial.print("\t\tVoltaje VFD: ");
    Serial.println(volt_value*10.0/3.3);
    dacWrite(DAC_PIN,dac_value);
    return;
  }
  
  if(strcmp("motor",topic)==0){
    if((char)payload[0]=='H'){
      Serial.println("***** Encendiendo Bomba *****");
      digitalWrite(START_STOP_PIN,LOW);
    }
    else{
      if((char)payload[0]=='L'){
        Serial.println("***** Parando Bomba *****");
        digitalWrite(START_STOP_PIN,HIGH);
      }
    }
    return;
  }
  if(strcmp("direction",(char*)topic)==0){
    if((char)payload[0]=='L'){
      Serial.println("***** Giro Sentido Antihorario *****");
      digitalWrite(DIR_PIN,LOW);
    }
    else{
      if((char)payload[0]=='R'){
        Serial.println("***** Giro Sentido Horario *****");
        digitalWrite(DIR_PIN,HIGH);
      }
    }
    return;
  }
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

      client.subscribe("variador");
      client.subscribe("direction");
      client.subscribe("motor");
      
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
  pinMode(START_STOP_PIN,OUTPUT);
  pinMode(DIR_PIN,OUTPUT);
  digitalWrite(START_STOP_PIN,HIGH);
  digitalWrite(DIR_PIN,HIGH);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();


}
