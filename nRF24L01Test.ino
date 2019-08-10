#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

// PINs configuration
const byte pinChipEnable = 9; // Pin attached to Enable Chip
const byte pinChipSelect = 8; // Pin attached to Chip Select
const byte pinLed = 7;        // Pin attached to LED
const byte pinCommand = 6;    // Pin attached to PushButton

// RF module uses SPI
// Uno :
// - MOSI = 11
// - MISO = 12
// - SCK  = 13
RF24 radio(pinChipEnable,pinChipSelect); // RF module object

// Channel used
byte address[6]="PFtst";
// Command to toggle the remote LED
#define MAKEWORD(c1,c2) ((((unsigned int)(c1))<<8) | (c2))
#define MAKEDWORD(c1,c2,c3,c4) ((((unsigned long)MAKEWORD(c1,c2))<<16) | MAKEWORD(c3,c4))
unsigned long TOGGLE  = MAKEDWORD('T','O','G','L');

// Button state to detect when button is released
bool pressed = false;
// Led State
int ledState = LOW;

// Toggle Led State (local or remote)
void ToggleLedState(bool local=true)
{
  if(local)
  {
    // Toggle Value
    ledState=(ledState==LOW)?HIGH:LOW;
    // Apply New Led State
    digitalWrite(pinLed,ledState);
  }
  else
  {
    // Stop Listening while sending command
    radio.stopListening();
    // Send command
    radio.write(&TOGGLE,sizeof(TOGGLE));
    
    // Restart Listening after command sent
    radio.startListening();
  }
}
// AutoTest : check LED and PushButton Connection
// Blink LED every .5s until button is pressed and released
// Faster if RF24 not found
void Test(long period=500)
{
  long lTime = millis();  // Last time the LED state is changed

  // Repeat Loop until Button is released
    do
    {
      if(digitalRead(pinCommand)==HIGH) 
        // Button is pressed
        pressed = true;
      else if(pressed) 
        // Button is not pressed but was before => button released, exit loop
        break;

      if(millis()-lTime>=period) 
      {
        // Last time the Led State changes was (at least) 500ms ago
        // Toggle local Led State
        ToggleLedState();
        // Remember last time the led state was changed
        lTime = millis();
      }
      // small wait
      delay(100);  
    } while(true);

    // Set Led OFF
    digitalWrite(pinLed,LOW);
    //reset pressed state
    pressed=false;
}



void setup() {
  // Configure pin Mode (Led=OUT, Command=IN)
  pinMode(pinLed,OUTPUT);
  pinMode(pinCommand,INPUT);

  // Start Radio Module
  radio.begin();
  // Set Output level (LOW for the demo) (use RF24_PA_MAX for long range)
  radio.setPALevel(RF24_PA_LOW);
  // Open writing and reading pipe
  radio.openWritingPipe(address);
  radio.openReadingPipe(1,address);

  // Start Listening
  radio.startListening();

  // Test Led/button
  Test(radio.isChipConnected() ? 500 : 100 );
}

void loop() 
{
  // Check Button State
  if(digitalRead(pinCommand)==HIGH)
    pressed=true;
  else if(pressed)
  {
    // Button is released, toggle remote LED
    ToggleLedState(false /* remote */);
    pressed = false;
  }

  // Check Radio Data
  if(radio.available())
  {
    unsigned long incoming;
    radio.read(&incoming,sizeof(incoming));

    if(incoming==TOGGLE)
      ToggleLedState();
  }
}
