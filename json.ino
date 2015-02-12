const float FLOAT_VAL_NOT_FOUND = -9999;
const int INT_VAL_NOT_FOUND = -9999;

// If json is like "{"temp":280.45,"humidity":86,"pressure":1024.5}" and key is "humidity", then will return "86".
String getJsonValue(const String& json, String key)
{
  // If key is temp, we need to search in JSON for: "temp":
  key = '\"' + key + "\":";

  int keyStart = json.indexOf(key);
  int valueStart = keyStart + key.length();
  int valueEnd = json.indexOf(',', valueStart);
  String value = "";
  if (valueStart > -1 && valueEnd > -1)
  {
    value = json.substring(valueStart, valueEnd);
  }
  
  //trace(String("Found value = ") + String(value) + String(" for key = ") + String(key));
  
  return value;
}

String getJsonTextValue(const String& json, String key)
{
  String jsonValue = getJsonValue(json, key);

  // Trim quotes around the text value.
  if (jsonValue.length() > 1)
    jsonValue = jsonValue.substring(1, jsonValue.length() - 1);
    
  return jsonValue;
}

int getJsonIntValue(const String& json, String key)
{
  int intFromJson = -9999;
  String jsonValue = getJsonValue(json, key);
  
  if (jsonValue.length() > 0)
    intFromJson = jsonValue.toInt();
    
  return intFromJson;
}

float getJsonFloatValue(const String& json, String key)
{
  float floatFromJson = -9999;
  String jsonValue = getJsonValue(json, key);

  if (jsonValue.length() > 0)
    floatFromJson = toFloat(jsonValue);  

  return floatFromJson;
}

float toFloat(const String& str)
{
  char buffer[10];
  str.toCharArray(buffer, 10);
  return atof(buffer);
}
