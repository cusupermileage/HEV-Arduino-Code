#include <Servo.h>
#include <MegaSquirt.h>
#include <SoftwareSerial.h>
#include <SparkFunSerLCD.h>
#include <SPI.h>
#include <mcp4822.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"



//TR: means Tyler Removed 3/25/14
void setup()  
{
  // Wait for serial data before startup so we can see all debug output
  #ifdef DEBUGWAIT
    while(!DEBUGSERIAL_PORT.available()); 
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.println("ms\t Core: Booting...");
  #endif
  
  #ifdef VARIANT_GLII
  #ifdef MEGASQUIRT_ENABLE
    setup_megasquirt();
  #endif
  #endif
  
  #ifndef NOSOFTWARESERIAL
  setup_serial();
  #endif
  
  setup_inputs();
  setup_outputs();
  //TR:setup_lcd();
  
  #ifdef DEBUGMODE
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.println("ms\t Core: Initialized");
    #ifdef VARIANT_STING
      DEBUGSERIAL_PORT.print(millis());
      DEBUGSERIAL_PORT.println("ms\t Core: Configured as variant \"Sting\"");
    #endif
    #ifdef VARIANT_GLII
      DEBUGSERIAL_PORT.print(millis());
      DEBUGSERIAL_PORT.println("ms\t Core: Configured as variant \"GLII\"");    
    #endif
  #endif

  #ifdef SPIADC
    spi_adc_config();
  #endif

  #ifdef DEBUGMODE
    delay(1500);
  #endif
  
}

int looptimer = 0;

int timer_1s = 0;
int timer_1s_last = 0;

int timer_p5s = 0;
int timer_p5s_last = 0;


bool flashled = false;

bool killed = false; //TA:

void loop()
{
  /* Loop timer */
  looptimer = millis();
  
  /* 1s periodic timer */
  //TR: int t_now = millis();
  timer_1s += abs(timer_1s_last - looptimer);
  timer_1s_last = looptimer;

  /* .5s periodic timer */
  timer_p5s += abs(timer_p5s_last - looptimer);
  timer_p5s_last = looptimer;
  
  /* High-frequency calls (as fast as possible) */
  #ifndef NOSOFTWARESERIAL
  process_serial_commands();
  #endif

  // We don't deal with updating throttle on GLII
  #ifdef VARIANT_STING
    process_throttle();
  #endif
  
  
  
  if(timer_p5s > 150) {
    #ifdef VARIANT_GLII
    #ifdef MEGASQUIRT_ENABLE
      process_megasquirt_getdata(); // Takes 25ms  
    #endif
    #endif
    /*analogWrite(PIN_LED, flashled * 255);
    if(flashled) {
      flashled = false;
    }
    else {
      flashled = true;
    }*/
    
    //flash led 
    digitalWrite(PIN_LED, flashled);
    flashled = !flashled;    
    
    timer_p5s = 0;    
    
  }
  
  /* Low-frequency calls (every 1 second) */  
  if(timer_1s > 1000) {
    
    //request_temps_1s();   
    
    // We might want to periodically call *just* the updater for megasquirt data at a specified rate, depening on our loop time.
    // The serial command processor can just pull the data out of the table when it needs it, keeping latency low. This means that
    // serial requests do NOT guarantee freshness of data.
    
    // Print out loop time of 1s periodic loop
    /*
    #ifdef DEBUGMODE
      DEBUGSERIAL_PORT.print(millis());
      DEBUGSERIAL_PORT.print("ms\t Main: 1s Loop Complete. Duration = ");
      DEBUGSERIAL_PORT.println( abs(millis() -looptimer) );      
      DEBUGSERIAL_PORT.print(millis());   

      get_speed_MPH();
    #endif
    */
    // Check if speed is 0.
    check_stopped();
    
    xLedOff(); // Turn off OK serial command indicator
    timer_1s = 0;
  }
  looptimer = 0;
}

