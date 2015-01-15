#include <LiquidCrystal.h>
#include <SPI.h>
#include <Ethernet.h>

const int LCD_COLS = 16;
const boolean TRACE_ENABLED = true;
const int MAIN_LOOP_DELAY_MS = 250;
const long WEATHER_API_POLL_MS = 3600000; // 1 hour

long loopTimeCount = 0;
String weather = "Weather not yet acquired.";

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192,168,0,178);

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  lcd.begin(LCD_COLS, 2);
  lcd.setCursor(0,0);
  
  lcd.print("Getting weather...");

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    trace("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }

  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void loop()
{
  trace(String("loopTimeCount = ") + String(loopTimeCount));
  
  if (loopTimeCount == 0)
  {
    lcd.clear();
    lcd.print("Getting weather...");
    weather = getWeather();
    lcd.clear();
    lcd.print(weather);
    trace(weather);
  }

  if (weather.length() > LCD_COLS)
  {
    lcd.scrollDisplayLeft();
  }
   
  loopTimeCount += MAIN_LOOP_DELAY_MS;
  if (loopTimeCount >= WEATHER_API_POLL_MS)
  {
    loopTimeCount = 0;
  }
  
  delay(MAIN_LOOP_DELAY_MS);  
}

void trace(const String& message)
{
  if (TRACE_ENABLED)
  {
    Serial.println(message);
  }
}

String getWeather()
{
  // if you don't want to use DNS (and reduce your sketch size)
  // use the numeric IP instead of the name for the server:
  //IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
  char server[] = "api.openweathermap.org";
  
  // Initialize the Ethernet client library
  // with the IP address and port of the server 
  // that you want to connect to (port 80 is default for HTTP):
  EthernetClient client;
  
  trace("connecting...");
  
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    trace("connected");

    // Sandhurst city id = 2638580
    client.println("GET /data/2.5/weather?id=2638580 HTTP/1.1");
    client.println("Host: api.openweathermap.org");
    client.println("Connection: close");
    client.println();
  } 
  else {
    // kf you didn't get a connection to the server:
    trace("connection failed");
    return "";
  }
  
  while (!client.available())
    delay(100);
  
  String weatherJson = readResponse(client);
  
  String weatherText = String();
  
  // Sometimes get an HTTP error code of 511, and no JSON content.
  // This is a quick check that we have more than just HTTP headers in the response.
  if (weatherJson.length() > 250)
  {
    int temperature = getJsonNumberValue(weatherJson, "temp") - 273;
    String weatherDescription = getJsonTextValue(weatherJson, "description");

    weatherText = String(temperature) + char(223) + "C " + weatherDescription;
  }
  
  return weatherText;
}

String readResponse(EthernetClient& client)
{
  String response = String();
  
  // if there are incoming bytes available 
  // from the server, read them and print them:
  while (client.available()) {
    response += String((char)client.read());
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    trace("disconnecting.");
    client.stop();
  }
  else {
    trace("Didn't disconnect.");
  }
  return response;
}


