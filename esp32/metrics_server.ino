#include <WiFi.h>
#include "DHT.h"

#define DHTPIN 25       // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // Set sensor type to DHT 11

// Replace with your network credentials
const char* ssid = "";
const char* password = "";

// Set web server port number to 80
WiFiServer server(80);

// Configure temperature sensor
DHT dht(DHTPIN, DHTTYPE);

unsigned long current_time = millis();
unsigned long previous_time = 0; 
const long request_timeout_time = 2000;

// Variable to store the HTTP request
String header;

void setup() {
  //Initiate Serial communication.
  Serial.begin(115200);

  // Start temperature sensor.
  dht.begin();
  
  // Connect to Wi-Fi network with SSID and password.
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  Serial.println("Data server ready for requests.");
}

void loop(){
  WiFiClient client = server.available();

  // Respond to any client that is connected.
  if (client) {                             
    current_time = millis();
    previous_time = current_time;
    Serial.println("New client connected.");
    String currentLine = "";

    // Loop while the client's connected 
    while (client.connected() && current_time - previous_time <= request_timeout_time) {  
      current_time = millis();

      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/raw");
            client.println("Connection: close");
            client.println();

            // data collection
            if (header.indexOf("GET /data") >= 0) 
            {
                Serial.println("Getting data.");
                float humidity = dht.readHumidity();
                float temperature = dht.readTemperature();
                float heat_index = dht.computeHeatIndex(temperature, humidity, false);
                
                // Respond in OpenTelemetry metrics
                client.print("# HELP temperature_celcius The temperature in degrees celcius.\n");
                client.print("# TYPE temperature_celcius gauge\n");
                client.print("temperature_celcius " + String(temperature, 2) + "\n");

                client.print("# HELP humitidy_percentage The humidity percentage.\n");
                client.print("# TYPE humidity_percentage gauge\n");
                client.print("humitidy_percentage " + String(humidity,2) + "\n");

                client.print("# HELP heat_index_celcius The heat index in degrees celcius.\n");
                client.print("# TYPE heat_index_celcius gauge\n");
                client.print("heat_index_celcius " + String(heat_index,2) + "\n");
                client.println();
                break;
              } 
              
            } else { // if you got a newline, then clear currentLine
              currentLine = "";
            }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }

    // Clear the header variable
    header = "";
    
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
