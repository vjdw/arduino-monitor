#define BTN_RIGHT  0
#define BTN_UP     1
#define BTN_DOWN   2
#define BTN_LEFT   3
#define BTN_SELECT 4
#define BTN_NONE   5

const char* MPD_HOST_NAME = "frapefruit";
const int MPD_HOST_PORT = 6600;

int buttonDownAtLastCheck = BTN_NONE;

void checkButtonState()
{
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
          mpdAddAllAndPlayRandom();
          break;
        }
        case BTN_UP:
        {
          sendMpdMessage("previous\n");
          break;
        }
        case BTN_DOWN:
        {
          sendMpdMessage("next\n");
          break;
        }
        case BTN_LEFT:
        {
          sendMpdMessage("pause\n");
          break;
        }
        case BTN_RIGHT:
        {
          break;
        }
        default:
          break;
      }
    }
  }

  buttonDownAtLastCheck = buttonDownNow;
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

void mpdAddAllAndPlayRandom()
{
  EthernetClient client;
  
  // if you get a connection, report back via serial:
  if (client.connect(MPD_HOST_NAME, MPD_HOST_PORT)) {
    trace("connected");

    client.println("clear\n");    // clears current playlist.
    client.println("add /\n");    // adds the root directory to the current (empty) playlist.
    client.println("random 1\n"); // make sure we're in random playback mode.
    client.println("play\n");     // start playing!
    client.stop();
  } 
  else {
    trace("MPD connection failed");
  }
}
