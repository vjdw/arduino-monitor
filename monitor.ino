#include <LiquidCrystal.h>
#include <SPI.h>
#include <Ethernet.h>

// These come from json.ino
extern const float FLOAT_VAL_NOT_FOUND;
extern const int INT_VAL_NOT_FOUND;

const int LCD_COLS = 16;
const int LCD_COLS_FOR_TEMP = 4;
const boolean TRACE_ENABLED = true;
const int LOOP_DELAY_MS = 250;
const long WEATHER_API_POLL_MS = 3600000; // 1 hour
const int NETWORK_TIMEOUT_MS = 5000;

long loopTimeCount = 0;
int weatherDescriptionScrollLocation = 0;
String weatherDescription;
String weatherHumidity;
String weatherPressure;
String weatherTemperature;

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
 
  if (loopTimeCount == 0) {
    lcd.clear();
    lcd.print("Getting weather...");

    // If getWeather errors (returns false) then weatherDescription should contain the error message, so don't modify it.    
    if (getWeather(weatherTemperature, weatherDescription, weatherHumidity, weatherPressure))
    {
      weatherDescription = weatherDescription + " P:" + weatherPressure + " H:" + weatherHumidity;
    }
    else
    {
      // If getting weather failed don't wait an hour to retry, try again in 5 minutes.
      loopTimeCount = 19 * (WEATHER_API_POLL_MS / 20);
    }
    
    weatherDescription = weatherDescription + " - ";
    
    trace(weatherDescription);
  }
  loopTimeCount += LOOP_DELAY_MS;
  if (loopTimeCount >= WEATHER_API_POLL_MS) {
    loopTimeCount = 0;
  }
  delay(LOOP_DELAY_MS);

  // On characters 4 to 15 of LCD row 0, scroll through weather description (everything except temperature, which is fixed on characters 0 to 3).
  lcd.clear();   
  lcd.setCursor(LCD_COLS_FOR_TEMP,0);
  String substringForScroll = weatherDescription.substring(weatherDescriptionScrollLocation++);
  if (substringForScroll.length() > (LCD_COLS - LCD_COLS_FOR_TEMP)) {
    lcd.print(substringForScroll);
  }
  else {
    // To fake endless scrolling, append first few characters of the description when substringForScroll is getting short.
    lcd.print(substringForScroll + weatherDescription.substring(0, (LCD_COLS - LCD_COLS_FOR_TEMP)));
  }
  if (weatherDescriptionScrollLocation >= weatherDescription.length()) {
    weatherDescriptionScrollLocation = 0;
  }
    
  // Temperature display doesn't scroll, position is fixed.
  lcd.setCursor(0,0);
  lcd.print(weatherTemperature);
  
  // On the second row, display time until next weather update.
  lcd.setCursor(0,1);
  long timeRemaining = WEATHER_API_POLL_MS - loopTimeCount;
  int seconds = (timeRemaining % 60000) / 1000;
  lcd.print(String((timeRemaining / 60000)) + "m" + (seconds < 10 ? "0" : "") + (seconds) + "s");
  
  // This will send messages to MPD when buttons are pressed.
  checkButtonState();
}

void formatTimeDigits(char strOut[2], int num)
{
  strOut[0] = '0' + (num / 10);
  strOut[1] = '0' + (num % 10);
}

void trace(const String& message)
{
  if (TRACE_ENABLED)
  {
    Serial.println(message);
  }
}

boolean getWeather(String& temperature, String& description, String& humidity, String& pressure)
{
  boolean errorOccurred = false;
  
  weatherDescription = "";
  
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
    trace("connection failed");
    weatherDescription = "connection failed";
    errorOccurred = true;
  }
  
  if (!errorOccurred)
  {
    int timeoutRemaining = NETWORK_TIMEOUT_MS;
    while (timeoutRemaining > 0 && !client.available())
    {
      delay(LOOP_DELAY_MS);
      timeoutRemaining -= LOOP_DELAY_MS;
    }
    if (!client.available())
    {
      trace("client not available");
      weatherDescription = "client  not available";
      errorOccurred = true;
    }
  }
  
  if (!errorOccurred)
  {
    String weatherJson = readResponse(client);

    float tempKelvin = getJsonFloatValue(weatherJson, "temp");
    if (tempKelvin == FLOAT_VAL_NOT_FOUND)
    {
      temperature = "err";
      errorOccurred = true;
    }
    else
    {
      temperature = String((int)(tempKelvin - 273.15)) + char(223) + "C";
    }

    description = getJsonTextValue(weatherJson, "description");
    
    int jsonInt = getJsonIntValue(weatherJson, "humidity");
    if (jsonInt == INT_VAL_NOT_FOUND)
    {
      humidity = "err";
      errorOccurred = true;
    }
    else
    {
      humidity = String(getJsonIntValue(weatherJson, "humidity")) + "%";
    }
      
    jsonInt = getJsonIntValue(weatherJson, "pressure");
    if (jsonInt == INT_VAL_NOT_FOUND)
    {
      pressure = "err";
      errorOccurred = true;
    }
    else
    {
      pressure = String(getJsonIntValue(weatherJson, "pressure"));
    }
  }
  
  return !errorOccurred;
}

String readResponse(EthernetClient& client)
{
  String response = String();
  boolean foundJsonStart = false;
  char currentChar;
  
  // if there are incoming bytes available 
  // from the server, read them and print them:
  while (client.available()) {
    currentChar = (char)client.read();
    foundJsonStart = foundJsonStart || currentChar == '{';
    if (foundJsonStart) {
      response += currentChar;
    }
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


