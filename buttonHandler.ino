#define BTN_RIGHT  0
#define BTN_UP     1
#define BTN_DOWN   2
#define BTN_LEFT   3
#define BTN_SELECT 4
#define BTN_NONE   5

const char* MPD_HOST_NAME = "frapefruit";
const int MPD_HOST_PORT = 6600;

int buttonDownAtLastCheck = BTN_NONE;
int currentPlaylist = 0;

String checkButtonState()
{
  String messageForLcd = "";
  
  int buttonDownNow = readLcdButtons();

  if (buttonDownNow != BTN_NONE)  
  {
    if (buttonDownAtLastCheck == BTN_NONE)
    {
      // So this is a new button push, handle it now.
      switch (buttonDownNow)
      {
        case BTN_SELECT:
        {
          // Load the next playlist.
          messageForLcd = mpdPlayNextPlaylist();
          break;
        }
        case BTN_UP:
        {
          // Previous track.
          sendMpdMessage("previous");
          messageForLcd = getCurrentTrack();
          break;
        }
        case BTN_DOWN:
        {
          // Next track.
          sendMpdMessage("next");
          messageForLcd = getCurrentTrack();
          break;
        }
        case BTN_LEFT:
        {
          // Toggle pause/play.
          if (toggleMpdSetting("state")) {
            messageForLcd = getCurrentTrack();
          }
          else {
            messageForLcd = "Paused";
          }
          break;
        }
        case BTN_RIGHT:
        {
          if (toggleMpdSetting("random")) {
            messageForLcd = "Random on";
          }
          else {
            messageForLcd = "Random off";
          }
          break;
        }
        default:
          break;
      }
    }
  }

  buttonDownAtLastCheck = buttonDownNow;
  
  return messageForLcd;
}

int readLcdButtons()
{
    int adc_key_in = analogRead(0);       // read the value from the sensor 

    // Buttons are centered at these valies: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    // We make this the 1st option for speed reasons since it will be the most likely result

    if (adc_key_in > 1000) return BTN_NONE; 
    
    if (adc_key_in < 50)   return BTN_RIGHT;  
    if (adc_key_in < 195)  return BTN_UP; 
    if (adc_key_in < 380)  return BTN_DOWN; 
    if (adc_key_in < 555)  return BTN_LEFT; 
    if (adc_key_in < 790)  return BTN_SELECT;  

    return BTN_NONE;
}

void sendMpdMessage(String message)
{
  EthernetClient client;
  
  // if you get a connection, report back via serial:
  if (client.connect(MPD_HOST_NAME, MPD_HOST_PORT)) {
    trace("connected");

    client.println(message);
    client.stop();
  } 
  else {
    trace("MPD connection failed");
  }
}

String mpdPlayNextPlaylist()
{
  EthernetClient client;
  String playlistName = "";
  
  trace("trying to connect...");
  if (client.connect(MPD_HOST_NAME, MPD_HOST_PORT)) {
    trace("connected");

    // Get the next available playlist.    
    playlistName = getPlaylistAtIndex(client, &currentPlaylist);
    currentPlaylist++;

    // And start playing that playlist.
    startPlaylist(client, playlistName);
    
    client.stop();
  } 
  else {
    trace("MPD connection failed");
    client.stop();
  }
  
  return playlistName;
}

String getPlaylistAtIndex(EthernetClient& client, int* playlistIndex)
{
  client.println("listplaylists");
  String playlistResponse = readMpdResponse(client);  

  String listNames[10]; 
  String mpdPlaylistResponseKey = String("playlist: ");
  int listCount = getMpdValueListFromResponse(playlistResponse, mpdPlaylistResponseKey, listNames);

  if (*playlistIndex >= listCount) {
    *playlistIndex = 0;
  }

  return listNames[*playlistIndex];
}

String readMpdResponse(EthernetClient& client)
{
  String playlistResponse = "";
  int mpdTimeout = 5000;
  
  while (client.available() || mpdTimeout > 0) {
    if (client.available()) {
      mpdTimeout = 0;
      playlistResponse += (char)client.read();
    }
    else {
      trace("Still waiting for MPD client...");
      mpdTimeout -= 250;
      delay(250);
    }
  }
  
  return playlistResponse;
}

// The response to the MPD command listplaylists can contain several playlist.
// This function returns an array of the names of those playlists in the listNames parameter.
int getMpdValueListFromResponse(const String& mpdResponse, const String& key, String* listNames)
{
  int listCount = 0;
  int valueFoundAt = 0;
  do {
    String value = "";
    valueFoundAt = getMpdValueFromResponse(mpdResponse, valueFoundAt, key, value);
    if (valueFoundAt > -1) {
      listNames[listCount++] = value;
      valueFoundAt += key.length();
      trace(value);
    }
  } while (valueFoundAt > -1);
  
  return listCount;
}

int getMpdValueFromResponse(const String& mpdResponse, int startFrom, const String& key, String& value)
{
  int keyStart = mpdResponse.indexOf(key, startFrom);
  int valueStart = -1;
  value = "";
  
  if (keyStart > -1) {
    valueStart = keyStart + key.length();
    int valueEnd = mpdResponse.indexOf('\n', valueStart);
    if (valueEnd > -1) {
      value = mpdResponse.substring(valueStart, valueEnd);
    }
  }
  
  return valueStart;
}

void startPlaylist(EthernetClient& client, const String& playlistName)
{
  // Clear the current playlist, load the selected one, and play.
  client.println("clear");
  client.println(String("load \"") + playlistName + "\"");
  client.println("play");
}

// Depending on settingName, will either toggle between play/pause or random-on/random-off.
boolean toggleMpdSetting(String settingName)
{
  boolean newSettingState;
  EthernetClient client;
  
  trace("trying to connect...");
  // if you get a connection, report back via serial:
  if (client.connect(MPD_HOST_NAME, MPD_HOST_PORT)) {
    trace("connected");
    
    client.println("status");
    
    String mpdResponse = readMpdResponse(client);
    
    String currentSettingState;
    getMpdValueFromResponse(mpdResponse, 0, settingName + ": ", currentSettingState);
    
    trace(currentSettingState);
    
    if (currentSettingState == "0") {
      client.println(settingName + " 1");
      newSettingState = true;
    }
    else if (currentSettingState == "1") {
      client.println(settingName + " 0");
      newSettingState = false;
    }
    else if (currentSettingState == "play") {
      client.println("pause");
      newSettingState = false;
    }
    else {
      client.println("play");
      newSettingState = true;
    }
    
    client.stop();
  } 
  else {
    trace("MPD connection failed");
    client.stop();
  }
  
  return newSettingState;
}

String getCurrentTrack()
{
  String currentTrack = "<No title>";
  EthernetClient client;
  
  trace("trying to connect...");
  // if you get a connection, report back via serial:
  if (client.connect(MPD_HOST_NAME, MPD_HOST_PORT)) {
    trace("connected");
    
    client.println("currentsong");
    
    String mpdResponse = readMpdResponse(client);
    client.stop();
    
    getMpdValueFromResponse(mpdResponse, 0, "Title: ", currentTrack);
  } 
  else {
    trace("MPD connection failed");
    client.stop();
  }
  
  return currentTrack;
}

