// If json is like "{"temp":280.45,"humidity":86,"pressure":1024.5}" and key is "humidity", then will return "86".
String getJsonValue(const String& json, String key)
{
  // If key is temp, we need to search in JSON for: "temp":
  key = '\"' + key + "\":";

  int keyStart = json.indexOf(key);
  int valueStart = keyStart + key.length();
  int valueEnd = json.indexOf(',', valueStart);
  String value = json.substring(valueStart, valueEnd);
  
  //Serial.println("Found value = " + value + " for key = " + key);
  
  return value;
}
