// If json is like "{"temp":280.45,"humidity":86,"pressure":1024.5}" and key is "humidity", then will return "86".
String getJsonValue(const String& json, String key)
{
  // If key is temp, we need to search in JSON for: "temp":
  key = '\"' + key + "\":";

  int keyStart = json.indexOf(key);
  int valueStart = keyStart + key.length();
  int valueEnd = json.indexOf(',', valueStart);
  String value = json.substring(valueStart, valueEnd);
  
  //trace(String("Found value = ") + String(value) + String(" for key = ") + String(key));
  
  return value;
}

String getJsonTextValue(const String& json, String key)
{
  String jsonValue = getJsonValue(json, key);
  return jsonValue.substring(1, jsonValue.length() - 1);
}

int getJsonNumberValue(const String& json, String key)
{
  return getJsonValue(json, key).toInt();
}
