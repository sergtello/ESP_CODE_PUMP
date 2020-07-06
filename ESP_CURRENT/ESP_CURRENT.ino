 #include <WiFi.h>
 #include <HTTPClient.h>

#include <Adafruit_ADS1015.h>
#define CANTMUESTRAS 1000 // muestras a tomar ( mas muestras mas exactidud pero mas tiempo de ejecucion)
#define AMAXSENS 100      // corriente maxima del sensor en este caso es el SCT013 que ofrece 30A max a 1000 mV
#define MVMAXSENS 1000    // Mv maximos que ofrece el sensor en su corriente maxima soportada
#define VOLTRED 220       // tension de la red
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

float Peficaz;
//float Int_calculada;

//WiFi
const char * ssid = "ADELE";
const char * password = "zucchini";

String GOOGLE_SCRIPT_ID = "AKfycbzRTU1lGZ3kWccw9C-fsrMg1ZY8ZsG5KgDt21nG0Ep5MH7fR1OF"; // Replace by your GAS service id

const int sendInterval = 996 *5; // in millis, 996 instead of 1000 is adjustment, with 1000 it jumps ahead a minute every 3-4 hours

//------------- 



//updated 04.12.2019
const char * root_ca=\
"-----BEGIN CERTIFICATE-----\n" \
"MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n" \
"A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp\n" \
"Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1\n" \
"MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG\n" \
"A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n" \
"hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL\n" \
"v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8\n" \
"eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq\n" \
"tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n" \
"C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa\n" \
"zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB\n" \
"mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH\n" \
"V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n\n" \
"bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG\n" \
"3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs\n" \
"J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO\n" \
"291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS\n" \
"ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n" \
"AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7\n" \
"TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n" \
"-----END CERTIFICATE-----\n";


WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  delay(10);
  
  ads.setGain(GAIN_TWO);         // 2x gain   +/- 2.048V  1 bit =     0.0625mV
  ads.begin();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

Serial.println("Iniciado");
Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

Serial.println("Listo para comenzar");
}

String Int_calculada()
{
  long tiempoinicio = millis();                       // para medir cuanto tarda en realizar las mediciones
  int16_t bitsads;
  float mVporbit = 0.0625F;
  float Ieficaz;
  float Iinstant;
  float mVinstant;
  float sumIinstant=0;

  for (int i = 0; i < CANTMUESTRAS; i++) {
    bitsads = ads.readADC_Differential_0_1();
    mVinstant = bitsads * mVporbit;
    Iinstant = mVinstant * AMAXSENS / MVMAXSENS;   // regla de tres en base al sensor conectado ya que el sensor ofrece tension y la pasamos directemante proporcional a intensidad
    sumIinstant += sq(Iinstant);                   // suma de cuadrados
  }
  Ieficaz = sqrt(sumIinstant / CANTMUESTRAS);        // raiz cuadrada de la suma de cuadrados dividida por el numero de muestras

  long tiempofin = millis();
  Serial.print(tiempofin - tiempoinicio);
  Serial.println(" tiempo para medir");
  float Int_calculadafinal = Ieficaz * 1.36; // se multiplica por un valor de correccion basado en mediciones reales
  int entero=int(Int_calculadafinal);
  int decimal=int(100.0*Int_calculadafinal)-(100*entero);
  String valor=String(entero)+","+String(int(decimal));
  return (valor);
}



void loop() {

  

  sendData("Corriente(A)=" + String(Int_calculada()));

  
  //delay(sendInterval);

}

void SendAlarm()//use this function to notify if something wrong (example sensor says -128C)
// don't forget to set true for enableSendingEmails in google script
{
   sendData("alarm=fixme"); 
}

void sendData(String params) {
   HTTPClient http;
   String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+params;
   Serial.print(url);
    Serial.print("Making a request");
    http.begin(url, root_ca); //Specify the URL and certificate
    int httpCode = http.GET();  
    http.end();
    Serial.println(": done "+httpCode);
}
