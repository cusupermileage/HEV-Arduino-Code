/* 
 * HEV Controller 1.0
 * Configuration File
 */

/* Debug Ouput Configuration */
  
  #define DEBUGMODE
  //#define INPUTDEBUG
  //#define OUTPUTDEBUG
  //#define DEBUGWAIT // NOTE: NEVER ENABLE DURING COMPETITION
  #define SERIALDEBUG
  //#define MEGASQUIRT_ENABLE
  //#define CAPTUREDATA
  //#define NOSOFTWARESERIAL
  #define SPIDEBUG
  
/* Compilation Target Vehicle */

  #define VARIANT_STING
  //#define VARIANT_GLII
  
  #ifdef VARIANT_STING
    // #define USE_GPS_SPEED
  #endif
  #ifdef VARIANT_GLII
    //#define USE_GPS_SPEED // Send speed of -999 so the tablet uses GPS for speed
  #endif
  
  
/* Physical Constraints */

  // Wheel Measurements
  #ifdef VARIANT_STING
    #define CIRCUMFERENCE 62.0f
    #define PULSESPERROTATION 8.0f
  #endif
  #ifdef VARIANT_GLII
    #define CIRCUMFERENCE 77.0f
    #define PULSESPERROTATION 8.0f
  #endif
  
  
  // Throttle Voltage Min/Max
  #ifdef VARIANT_STING
    // Note: this is INVERTED on sting! Make sure the code handles this!
    #define THROTTLEMAX 360
    #define THROTTLEMIN 673
  #endif
  #ifdef VARIANT_GLII
    #define THROTTLEMAX 855
    #define THROTTLEMIN 188
  #endif
  
  // Synqor Constants (Volts)
  #define MOTOR_MAX_VOLTAGE 37.0f
  #define SYNQOR_MAX_TRIM 1.78f //(2.366f - (2.316f * (MOTOR_MAX_VOLTAGE / 60.0f )))
  #define SYNQOR_MIN_TRIM 2.65f
  
  // Trottle Servo Min/Max (Degrees)
  #define SERVO_MAX_DEGREES 132
  #define SERVO_MIN_DEGREES 53  
  
/* Pin Configuration */
  #ifdef VARIANT_STING
    #define PIN_PITOT A5
    #define PIN_VBATT1 A4
    #define PIN_VBATT2 A1 // FIXME
    #define PIN_THROTTLE A3
    #define PIN_KILL1 6 // LEFT KILL
    #define PIN_KILL2 12 // RIGHT KILL
    #define PIN_KILL3 4 // MASTER KILL
    #define PIN_ARMSTATUS 9
    #define PIN_AUTOMANUALSW 8
  
    #define PIN_1WIRETEMP 7
    #define PIN_ARMDISARMSET 10
  
    #define PIN_SERVO 18 // remapped from armdisarmset
    #define PIN_LED 11
    #define PIN_DAC_CS A2
    #define BLUESERIAL_PINRX 3
    #define BLUESERIAL_PINTX 5
    #define LCD_PIN 13
    #define PIN_WHEELSPEED 2
  #endif
  #ifdef VARIANT_GLII
    #define PIN_PITOT A5
    #define PIN_VBATT1 A4
    #define PIN_VBATT2 A1 // FIXME
    #define PIN_THROTTLE A0
    #define PIN_KILL1 6 // LEFT KILL
    #define PIN_KILL2 12 // RIGHT KILL
    #define PIN_KILL3 4 // MASTER KILL
    #define PIN_ARMSTATUS 9
    #define PIN_AUTOMANUALSW 8
  
    #define PIN_1WIRETEMP 7
    #define PIN_ARMDISARMSET 17
    #define PIN_SERVO 10  
    #define PIN_LED 11
    #define PIN_DAC_CS A2
    #define BLUESERIAL_PINRX 3
    #define BLUESERIAL_PINTX 5
    #define LCD_PIN 13
    #define PIN_WHEELSPEED 2

  #endif
  
  
  #define ISR_WHEELSPEED 1
  #define EVENT_WHEELSPEED FALLING

/* Serial Port Configuration */
  
  SoftwareSerial SoftSerialPort(BLUESERIAL_PINRX, BLUESERIAL_PINTX);
  #define BLUESERIAL_PORT SoftSerialPort // Sting can use Serial1 for bluetooth, GLII can use BlueSerial
  #define BLUESERIAL_BAUD 19200 //19200 // Bluetooth to tablet, max is 115200 for softwareserial
  #define ISR_BLUESERIAL 0

  #define MEGASQUIRT_PORT Serial1 // Megasquirt uses hardware serial port

  #define DEBUGSERIAL_PORT Serial
  #define DEBUGSERIAL_BAUD 9600


/* LCD Configuration */
  
  #define LCD_ROWS 4
  #define LCD_COLS 20


/* 1-Wire Temperature Sensor Configuration */
  
  #define TEMPERATURE_PRECISION 9

