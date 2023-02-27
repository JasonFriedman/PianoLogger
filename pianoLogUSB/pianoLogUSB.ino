// NOTE: NEED TO CHANGE UsbCore.h (in Documents/Arduino/libraries/USB_Host_Shield_Library_2.0) this line to:
// typedef MAX3421e<P3, P9> MAX3421E; // Official Arduinos (UNO, Duemilanove, Mega, 2560, Leonardo, Due etc.), Intel Edison, Intel Galileo 2 or Teensy 2.0 and 3.x

// Bottom layer is the UNO board

// Middle layer is the USB host shield
// Need to cut the solder jumper connector of SS, and solder a wire from SS to pin 2 (4th from the end, NOT on the side with the reset button)
// Need to solder the 3.3V and 5V jumpers (next to the reset button), and the 5V jumper next to where is it written VBUS PWR

// Top layer is the data logging shield
// Make sure there is a battery in the holder (will need to then set the time)
// Solder CD to pin 4 (on the side of the board with the reset button)
// Bend the IOREF and RST pins (next to the battery), and the SCL and SDA pins so it fits on the USB host shield

#include <SPI.h>
#include <TimeLib.h>
#include <DS1307RTC.h> 
#include <SdFat.h>
// The next two are to check for memory issues
//#include <MemoryFree.h>
//#include <pgmStrToRAM.h>

#include <usbh_midi.h>
#include <usbhub.h>

// red/green LED, from ebay page:
// Red forward voltage: 1.8 ~ 2.2
// Green forward voltage: 3.0 ~ 3.4
// Maximum continuous forward current: 20 mA
// For 2.0 V, need 150 ohm
// for 3.2 V, need 91 ohm (use 100 ohm)
#define RED_LED 8   // middle length - 150 ohm
#define GREEN_LED 7 // the shortest lead - 100 ohm
// longest lead is the ground

USB Usb;
//USBHub Hub(&Usb);
USBH_MIDI  Midi(&Usb);

const uint8_t SDchipSelect = 10;
const uint8_t cardDetect = 4;
SdFat sd;


time_t starttime;
long startMillis;

File fd;
char fileName[13]; // SD library only supports up to 8.3 names
bool alreadyBegan = false;  // SD.begin() misbehaves if not first call

bool writeError = true;

long lastswitch = -1;

void setup() {
  pinMode(3,OUTPUT);
  pinMode(4,INPUT);
  pinMode(10,OUTPUT);
  pinMode(cardDetect, INPUT_PULLUP);

  pinMode(RED_LED,OUTPUT);
  pinMode(GREEN_LED,OUTPUT);

  // Start with it red
  digitalWrite(RED_LED,1);
  digitalWrite(GREEN_LED,0);

  if (Usb.Init() == -1)
    Serial.println(F("OSC did not start."));

  setSyncProvider(RTC.get);

  Serial.begin(115200);
  Serial.println(F("Piano logger"));

  // Get the time from the RTC and the current millis
  starttime = now();
  long startMillis = millis();

  // Connect to the SD card
  initializeCard();
}

void loop() {
  Usb.Task();
  if ( Usb.getUsbTaskState() == USB_STATE_RUNNING )
  {
    MIDI_poll();
    digitalWrite(RED_LED,0);
    digitalWrite(GREEN_LED,1);
  }
  else {
    // MIDI is disconnected - set the red light
    digitalWrite(RED_LED,1);
    digitalWrite(GREEN_LED,0);
    return;
  }
    
  time_t timeNow = now();
  // only need to check if the 2nd character (of the day) is different
  int thisday = day(timeNow);
  if (thisday % 10 +48 != (int) fileName[1])
    setFileName();

  if (writeError) {
     long timepassed = millis() - lastswitch;
      if (timepassed<500) {
        digitalWrite(RED_LED,1);
        digitalWrite(GREEN_LED,0);
      }
      else if (timepassed<1000) {
        digitalWrite(RED_LED,0);
        digitalWrite(GREEN_LED,0);
      }
      else {
        // reset the counter
        lastswitch = millis();
        digitalWrite(RED_LED,1);
        digitalWrite(GREEN_LED,0);
      }
  } else {
    lastswitch = -1;
    digitalWrite(RED_LED,0);
    digitalWrite(GREEN_LED,1);
  }
}

void MIDI_poll()
{
  uint16_t  rcvd;
  uint8_t bufMidi[65];
  delay(10);
  
  if (Midi.RecvData( &rcvd,  bufMidi) == 0 ) {
    uint8_t command = ((uint8_t)bufMidi[1])/16;
    uint8_t channel = ((uint8_t)bufMidi[1])%16;
    uint8_t note = (uint8_t)bufMidi[2];
    uint8_t velocity = (uint8_t)bufMidi[3];
    if (command==9)
      handleNoteOn(channel,note,velocity);
    
    else if (command==8)
       handleNoteOff(channel,note,velocity);
    else
       Serial.println(F("Ignoring other MIDI command"));  
      // Ignore other commands for now 
  }
}

void handleNoteOn(uint8_t inChannel, uint8_t inNote, uint8_t inVelocity)
{ 
  char s[38];
  printTime(s);
  snprintf_P(s+23,
             15,
             PSTR(",1,%03d,%03d,%03d"),
             inChannel, inNote, inVelocity); // 1 = on
  Serial.println(s);
  digitalWrite(RED_LED,1);
  digitalWrite(GREEN_LED,0);
  writeToSDCard(s);
  digitalWrite(RED_LED,0);
  //Serial.print(F("Free RAM = "));
  //Serial.println(freeMemory(), DEC);
}

void handleNoteOff(uint8_t inChannel, uint8_t inNote, uint8_t inVelocity)
{
  char s[38];
  printTime(s);
  snprintf_P(s+23,
             15,
             PSTR(",2,%03d,%03d,%03d"),
             inChannel, inNote, inVelocity); // 2 = off
  Serial.println(s);
  digitalWrite(RED_LED,1);
  digitalWrite(GREEN_LED,0);
  writeToSDCard(s);
  digitalWrite(RED_LED,0);
  //Serial.print(F("Free RAM = "));
  //Serial.println(freeMemory(), DEC);
}

int getTime(time_t* newtime) {
  // Get the time
  long timediff = millis() - startMillis;
  uint32_t newtimeSeconds = starttime + timediff / 1000;
  int ms = timediff % 1000;
  return ms;
}

// datestring should be a char array with size at least 24 (23 characater + null terminator)
void printTime(char* datestring) {
  time_t newtime = now();
  int ms = getTime(&newtime);

  snprintf_P(datestring,
             24,
             PSTR("%02u/%02u/%04u,%02u:%02u:%02u.%03u"),
             day(newtime),
             month(newtime),
             year(newtime),
             hour(newtime),
             minute(newtime),
             second(newtime),
             ms);
}

void initializeCard(void)
{
  Serial.print(F("Initializing SD card..."));

  // Is there even a card?
  // On this board - cardDetect is 0 means there is a card
  if (digitalRead(cardDetect))
  {
    Serial.println(F("No card detected. Waiting for card."));
    while (digitalRead(cardDetect)) {
        digitalWrite(RED_LED,1);
        digitalWrite(GREEN_LED,0);
        delay(500);
        digitalWrite(RED_LED,0);
        delay(500);
    }
    delay(250); // 'Debounce insertion'
  }

  // Card seems to exist. 
  while (!sd.begin(SDchipSelect))  // begin uses half-speed...
  {
    Serial.println(F("Initialization failed!"));
    digitalWrite(RED_LED,1);
    digitalWrite(GREEN_LED,0);
    delay(500);
    digitalWrite(RED_LED,0);
    delay(500);
  }
  Serial.println(F("Initialization done."));

  setFileName();

  Serial.print(fileName);
  if (sd.exists(fileName))
  {
    Serial.println(F(" exists."));
  }
  else
  {
    Serial.println(F(" doesn't exist. Creating."));
  }

  Serial.print(F("Opening file: "));
  Serial.println(fileName);

  // Set the LED to green after connection
  writeError = false;
}

void writeToSDCard(char* message) {
  fd = sd.open(fileName, FILE_WRITE);
  if (fd) {
    fd.println(message);
    //fd.flush(); // This is done automatically when the file is closed
    fd.close();
    writeError = false;
  }
  else {
    Serial.println(F("*** Did not succeed to write to SD card ***"));
    writeError = true;
  }
}

void setFileName() {
  // fileName based on the date
  time_t newtime = now();

  snprintf_P(fileName,
             13,
             PSTR("%02u%02u%04u.csv"),
             day(newtime),
             month(newtime),
             year(newtime));
  Serial.print(F("Set filename to "));
  Serial.println(fileName);
}
