#define PWM_PIN 33
#define FLOW_PIN 14
#include <WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "ADELE";
const char* password = "zucchini";
const char* mqtt_server = "broker.shiftr.io";

const int freq = 50;
const int ledChannel = 0;
const int resolution = 8;

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
float calibrationFactor = 9.3;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

WiFiClient espClient;
PubSubClient client(espClient);

void pulseCounter(){
  pulseCount++;
}

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
  if(strcmp("valve_1",topic)==0){

    float pwm_value=0.0;
    String msg = String(tilin);
    //Serial.println(msg);
    pwm_value= (msg.toInt()*14.0)/90.0+13;
    ledcWrite(ledChannel, (int)roundf(pwm_value));
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
      client.subscribe("valve_1");
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
  pinMode(FLOW_PIN,INPUT);
  
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), pulseCounter, RISING);

  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(PWM_PIN, ledChannel);
  
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
  String caudal;
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    
    pulse1Sec = pulseCount;
    pulseCount = 0;

    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
   
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(flowRate);
    Serial.print("L/min");
    Serial.println("\t");  
    flowRate = constrain(flowRate,0.0,30.0);    
    caudal=String(flowRate,2);
    client.publish("caudal",caudal.c_str());

  }
 delay(750);

}
