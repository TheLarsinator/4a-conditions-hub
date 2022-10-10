#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DHT.h"

/*Put your SSID & Password*/
const char* ssid = "";
const char* password = "";

// Set your Static IP address
IPAddress local_IP(192, 168, 0, );
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);

IPAddress subnet(255, 255, 0, 0);

const String room_name = "";

ESP8266WebServer server(80);

#define DHTPIN 2       // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // Set sensor type to DHT 11

// Configure temperature sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  delay(100);
  
  // Start temperature sensor.
  dht.begin();

  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  server.on("/data", handle_data);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handle_data() {
  Serial.println("Handling data");
  server.send(200, "text/raw", GetConditionsData()); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String GetConditionsData()
{
    Serial.println("Getting data.");
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    float heat_index = dht.computeHeatIndex(temperature, humidity, false);
    
    // Respond in OpenTelemetry metrics
    String response = "";
    response += "# HELP temperature_celcius The temperature in degrees celcius.\n";
    response +="# TYPE temperature_celcius gauge\n";
    response +="temperature_celcius{room=\"" + room_name + "\"} " + String(temperature, 2) + "\n";

    response +="# HELP humitidy_percentage The humidity percentage.\n";
    response +="# TYPE humidity_percentage gauge\n";
    response +="humitidy_percentage{room=\"" + room_name + "\"} " + String(humidity,2) + "\n";

    response +="# HELP heat_index_celcius The heat index in degrees celcius.\n";
    response +="# TYPE heat_index_celcius gauge\n";
    response +="heat_index_celcius{room=\"" + room_name + "\"} " + String(heat_index,2) + "\n";

    return response;
}
