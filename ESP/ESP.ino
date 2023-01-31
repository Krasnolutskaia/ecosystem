#include <DHT.h>
#include <AccelStepper.h>  
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>  

#define SOIL_PIN 32
#define PUMP_PIN 19
#define DHT_PIN 12
#define DHT_TYPE DHT21
#define MOTORPIN1 23
#define MOTORPIN2 5
#define MOTORPIN3 13 
#define MOTORPIN4 12
#define HALFSTEP 4

AccelStepper stepper(HALFSTEP, MOTORPIN1, MOTORPIN3, MOTORPIN2, MOTORPIN4);

DHT dht(DHT_PIN, DHT_TYPE);

const char *ssid = "1";       // имя вашей wifi точки доступа
const char *password = "0123456789"; // пароль wifi
const char* serverName_from = "http://192.168.101.229:80/from_greenhouse";
const char* serverName_to = "http://192.168.101.229:80/to_greenhouse";
JSONVar doc;

float hum_air, temp;
int hum_soil, hum_soil_min, light_av, light_max, light_min, volume;

int delay_time = 3000;
unsigned long last_time;

int auto_water = 1;
int auto_light = 1;


int light_vals[4];
int light_pins[4] = {35, 34, 36, 39};

void setup() {
  Serial.begin(115200);
  dht.begin();

  stepper.setMaxSpeed(1000);

  last_time = millis();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) // подключение к точке
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}


void get_light(){
  for(int i = 0; i < 4; i++)
  {
    light_vals[i] = analogRead(light_pins[i]);
  }
}

int get_hum_soil(){
  return analogRead(SOIL_PIN);
}

void rotate(int angle){
  if (-180 <= angle <= 180){
    stepper.moveTo(map(angle, -180, 180, -1024, 1024));
    stepper.setSpeed(200);
    stepper.runSpeedToPosition();
  }
}


void water(){
  if (auto_water == 1){
    if (hum_soil < hum_soil_min){
      digitalWrite(PUMP_PIN, HIGH);
      delay(5000);
      digitalWrite(PUMP_PIN, LOW);
    }
  }
}


void send_data(){
  if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      http.begin(client, serverName_from);

      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String httpRequestData = "hum_air=" + String(dht.readHumidity())
                                + "&temp=" + String(dht.readTemperature())
                                + "&hum_soil=" + String(hum_soil)
                                + "&rotate=" + String(doc["rotate"]);
      
      for(int i = 0; i < 4; i++)
      {
        httpRequestData += "&light_val" + String(i) + "=" + String(light_vals[i]);
      }

      int httpResponseCode = http.POST(httpRequestData);

      Serial.println(httpResponseCode);
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}


void get_data(){
    if(WiFi.status()== WL_CONNECTED){
              
      String server_data = httpGETRequest(serverName_to);
      doc = JSON.parse(server_data);
  
      if (JSON.typeof(doc) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
    
      Serial.print("JSON object = ");
      Serial.println(doc);
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    return;
}


String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  http.begin(client, serverName);

  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  return payload;
}


void loop() 
{
  if (millis() - last_time > delay_time)
  {
    hum_soil = get_hum_soil();
    
    get_light();

    send_data();
    get_data();

    last_time = millis();
  }

  if (doc["rotate"] == 1){
    rotate(doc["angle"]);
    doc["rotate"] = 0;
  } 
}
