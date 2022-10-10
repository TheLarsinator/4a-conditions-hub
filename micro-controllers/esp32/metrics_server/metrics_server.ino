/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

// Load library
#include <WiFi.h>
#include "DHT.h"

#define DHTPIN 25     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);

// Replace with your network credentials
const char* ssid = "";
const char* password = "";

const String room_name = "";

// Set your Static IP address
IPAddress local_IP(192, 168, 0, );
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);

IPAddress subnet(255, 255, 255, 0);

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  //Initiate Serial communication.
  Serial.begin(115200);

  Serial.println("Starting temperature measurements");
  dht.begin();

  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  
  // Connect to Wi-Fi network with SSID and password
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
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
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
              Serial.println("Data requested");
              float humidity = dht.readHumidity();
              float temperature = dht.readTemperature();
              float heat_index = dht.computeHeatIndex(temperature, humidity, false);
              Serial.print(humidity);
              Serial.print(",");
              Serial.print(temperature);
              Serial.print(",");
              Serial.println(heat_index);
              
              // Respond in OpenTelemetry metrics
              client.print("# HELP temperature_celcius The temperature in degrees celcius.\n");
              client.print("# TYPE temperature_celcius gauge\n");
              client.print("temperature_celcius{room=\"" + room_name + "\"} " + String(temperature, 2) + "\n");

              client.print("# HELP humitidy_percentage The humidity percentage.\n");
              client.print("# TYPE humidity_percentage gauge\n");
              client.print("humitidy_percentage{room=\"" + room_name + "\"} " + String(humidity,2) + "\n");

              client.print("# HELP heat_index_celcius The heat index in degrees celcius.\n");
              client.print("# TYPE heat_index_celcius gauge\n");
              client.print("heat_index_celcius{room=\"" + room_name + "\"} " + String(heat_index,2) + "\n");
            
              // Break out of the while loop
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
