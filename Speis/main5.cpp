#include <Wire.h>
#include <SparkFunISL29125.h>

#include <Adafruit_NeoPixel.h>
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>
#include <Adafruit_BluefruitLE_UART.h>
#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

#include <BluefruitConfig.h>

#define PIN 6
#define NUMPIXELS 2
#define DELAYVAL 0
#define DELAYEND 0

// Relevante verdier:

#define LIMIT 6000 //triggerpunkt for lyssensor
#define R 40    //Rødt lys ved mørke
#define G 40    //Grønt lys ved mørke
#define B 40    //Blått lys ved mørke
#define W 40

#define RX 100    //Rød lysstyrke ved lys/lyd
#define GX 100    //Grønn lysstyrke ved lys/lyd
#define BX 100    //Blå lysstyrke ved lys/lyd

//

#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "DISABLE"

SFE_ISL29125 RGB_sensor;

// Example for demonstrating the TSL2591 library - public domain!

// connect SCL to SCL
// connect SDA to SDA
// connect Vin to 3.3V
// connect GROUND to GND
Adafruit_NeoPixel strip = Adafruit_NeoPixel(2, PIN, NEO_GRBW + NEO_KHZ800);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}



/**************************************************************************/
/*
    Program entry point for the Arduino sketch
*/
/**************************************************************************/
void setup(void) 
{
  strip.begin();
  strip.show();
  Serial.begin(9600);
  
  

  /* Configure the sensor */
  if (RGB_sensor.init())
  {
    Serial.println("Sensor Initialization Successful\n\r");
  }

  RGB_sensor.config(CFG1_MODE_R | CFG1_375LUX, CFG2_IR_ADJUST_HIGH, CFG3_NO_INT | CFG1_12BIT);
  strip.begin();

/* Initialise the ble module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println(F("Then Enter characters to send to Bluefruit"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }

  Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set module to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));

  
  

  // Now we're ready to get readings ... move on to loop()!

  
}


/**************************************************************************/
/*
    Performs a read using the Adafruit Unified Sensor API.
*/
/**************************************************************************/
// 0 to 255
void brighten() {
  uint16_t i, j;

  for (j = R; j < 255; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, j, j, j, j);
    }
    strip.show();
    delay(6);
  }
  //delay(1500);
}

// 255 to 0
void darken() {
  uint16_t i, j;

  for (j = 255; j > R; j--) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, j, j, j, j);
    }
    strip.show();
    delay(1);
  }
  //delay(1500);
}

void unifiedSensorAPIRead(void)


{
  Serial.begin(9600);
  unsigned int red = RGB_sensor.readRed();
  Serial.print("Red: "); Serial.println(red);

  if ((red < LIMIT))
  {
    /* If event.light = 0 lux the sensor is probably saturated */
    /* and no reliable data could be generated! */
    /* if event.light is +/- 4294967040 there was a float over/underflow */
    for(int i=0; i<NUMPIXELS; i++) { // For each pixel...

    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    strip.setPixelColor(i, strip.Color(R, G, B, W));
    /*ble.print(event.light);*/
    /*ble.println();*/
    /*Serial.print(event.light); Serial.println(F(" lux RED"));*/
    strip.show();   // Send the updated pixel colors to the hardware.

    delay(DELAYVAL); // Pause before next pass through loop
    }
  
    
  }
}


/**************************************************************************/
/*
    Arduino loop function, called once 'setup' is complete (your own code
    should go here)
*/
/**************************************************************************/
void loop(void) 
{ 
  unsigned int red = RGB_sensor.readRed();

  unifiedSensorAPIRead();
  if ((red > LIMIT)){
    ble.println(red);
    brighten();
    darken();}
  else
  {
    delay(DELAYEND);
  }
}