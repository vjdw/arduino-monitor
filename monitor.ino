#include <LiquidCrystal.h>
#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
//char server[] = "www.google.com";    // name address for Google (using DNS)
char server[] = "api.openweathermap.org";

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192,168,0,177);

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");

    // Sandhurst city id = 2638580
    client.println("GET /data/2.5/weather?id=2638580 HTTP/1.1");
    client.println("Host: api.openweathermap.org");
    client.println("Connection: close");
    client.println();
  } 
  else {
    // kf you didn't get a connection to the server:
    Serial.println("connection failed");
  }
  
  while (!client.available())
    delay(100);
  
  String weatherJson = getWeatherJson();
  Serial.println(weatherJson);
 
  String temperature = getJsonValue(weatherJson, "temp");
  int temperatureCelsius = temperature.toInt() - 279;
  String weatherDescription = getJsonValue(weatherJson, "description");
  
  String weatherText = String(temperatureCelsius) + "'C " + weatherDescription;
  
  Serial.println(weatherText);
}

void loop()
{  
//    // do nothing forevermore:
//    while(true);
//  }
}

String getWeatherJson()
{
  String weather = String();
  
  // if there are incoming bytes available 
  // from the server, read them and print them:
  while (client.available()) {
    //char c = client.read();
    //Serial.print(c);
    //weather += String((char)client.read());
    weather += String((char)client.read());
    //Serial.print(weather);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
  else {
    Serial.println("Didn't disconnect.");
  }
  
  return weather;
}


