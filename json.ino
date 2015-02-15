const float FLOAT_VAL_NOT_FOUND = -9999;
const int INT_VAL_NOT_FOUND = -9999;

// If json is like "{"temp":280.45,"humidity":86,"pressure":1024.5}" and key is "humidity", then will return "86".
void getJsonValue(const String& json, const String& key, String& value)
{
  // If key is temp, we need to search in JSON for: "temp":
  String keyJson = String(key);
  keyJson = String("\"") + keyJson + "\":";

  int keyStart = json.indexOf(keyJson);
  int valueStart = keyStart + keyJson.length();
  int valueEnd = json.indexOf(',', valueStart);
  value = "";

  if (keyStart > -1 && valueEnd > -1) {
    value = json.substring(valueStart, valueEnd);
  }
  
  //trace(String("Found value = ") + String(value) + String(" for key = ") + String(key));
}

void getJsonTextValue(const String& json, const String& key, String& value)
{
  getJsonValue(json, key, value);

  // Trim quotes around the text value.
  if (value.length() > 1)
    value = value.substring(1, value.length() - 1);
}

int getJsonIntValue(const String& json, const String& key)
{
  int intFromJson = INT_VAL_NOT_FOUND;
  String jsonValue = "";
  getJsonValue(json, key, jsonValue);
  
  if (jsonValue.length() > 0)
    intFromJson = jsonValue.toInt();
    
  return intFromJson;
}

float getJsonFloatValue(const String& json, const String& key)
{
  float floatFromJson = FLOAT_VAL_NOT_FOUND;
  String jsonValue = "";
  getJsonValue(json, key, jsonValue);

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
