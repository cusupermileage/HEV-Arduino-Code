/*
 * Serial I/O Functions
 * 
 * - USB serial port used as debug console
 * - blueSerial is a SoftwareSerial port for communicating via bluetooth to tablet
 *
 */

//SoftwareSerial blueSerial =  SoftwareSerial(PIN_BLUERX, PIN_BLUETX);

// Define commands
enum commands {
  DUMP_DATA = 42,
  VBATT1_VOLT = 48,        // 0
  VBATT2_VOLT,	           // 1
  ENGINE_RPM,		   // 2 MegaSquirt
  ENGINE_TEMP,		   // 3 MegaSquirt
  IAT,			   // 4 MegaSquirt ??????
  LEFT_KILL,		   // 5
  MAIN_BATTERY_VOLT,	   // 6
  MAP,			   // 7 MegaSquirt
  MASTER_KILL,		   // 8
  OXYGEN,		   // 9 MegaSquirt ?????
  REL_WIND_SPEED,	   // :
  RIGHT_KILL,		   // ;
  SPEED_INST,		   // < Input from interupt pin (in MPH)
  THROTTLE_PERCENT,	   // =
  TOTAL_DISTANCE,	   // >
  DRIVE_MOTOR_VOLT,        // ?
  ARM_STANDBY,             // @
  ENGINE_AFR,               // A
  
  //SPI ADC Chip - Urbie Only
  CAP_VOLT,                // B
  GEN_VOLT,                // C
  DRIVE_VOLT,              // D
  KILL_1,                  // E
  KILL_2,                  // F
  KILL_3,                  // G
  GEN_TEMP_1,              // H
  GEN_TEMP_2,              // I
  DRIVE_TEMP_1,            // J
  DRIVE_TEMP_2,            // K
  ENGINE_TEMP_2,           // L
  GEN_CURRENT,             // M
  DRIVE_CURRENT,           // N
  REGEN_CURRENT            // O
};

#define FIRST_DUMP_COMMAND VBATT1_VOLT
#define LAST_DUMP_COMMAND REGEN_CURRENT

// Maximum command binary value
#define MAX_VALID_CMD 100

// int commandLengthGet[16]
int commandLengthGet[100];

// Serial buffer
char buffer[15];

// Current position in buffer
int bufferIndex = 0;

boolean isDumping = false;
uint8_t dumpCounter = FIRST_DUMP_COMMAND;

void SoftSerialISR() {
 SoftSerialPort.recv();
 //DEBUGSERIAL_PORT.println("SoftSerial: Interrupt Trigger"); 
}

void setup_serial() {
  attachInterrupt(0, SoftSerialISR, CHANGE); 
  /* Initialize bluetooth serial port */
  BLUESERIAL_PORT.begin(BLUESERIAL_BAUD); // We can speed this up if we want (up to 115200)

  /* Initialize debug serial port */
  Serial.begin(DEBUGSERIAL_BAUD);
  
  #ifdef DEBUGMODE
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.println("ms\t SerialIO: Initialization Complete");

    BLUESERIAL_PORT.print("\r\r"); // This may be needed to kill issue with first byte sent being junk
  #endif
  
  // Test command lengths
  commandLengthGet[33] = 3; // set num arguments for "GET_EXCLAMATION" command
  commandLengthGet[34] = 0;
  commandLengthGet[49] = 0;
  commandLengthGet[50] = 0;
  commandLengthGet[42] = 0;
  
  // Zero all command lengths
  for(int i=0; i<100; i++) {
    commandLengthGet[i] = 0;
  }
}

/* Process serial data for incoming commands/data */
void process_serial_commands() {
  if(isDumping) {
    if(dumpCounter <= LAST_DUMP_COMMAND) {
      execute_command(dumpCounter);
      dumpCounter++;
    }
    else {
      #ifdef SERIALDEBUG
        DEBUGSERIAL_PORT.print(millis());
        DEBUGSERIAL_PORT.println("ms\t SerialIO: Dump Complete: ");
      #endif
      // End dump
      BLUESERIAL_PORT.print('}');
      dumpCounter = FIRST_DUMP_COMMAND;
      isDumping = false; 
    }
  }
  else {
    // If port is available, parse a byte
    if(BLUESERIAL_PORT.available() > 0) {
      
      // Pull off a byte from the hardware buffer
      buffer[bufferIndex] = BLUESERIAL_PORT.read();
      #ifdef SERIALDEBUG
        DEBUGSERIAL_PORT.print(millis());
        DEBUGSERIAL_PORT.print("ms\t SerialIO: Have bytes on port: ");
        DEBUGSERIAL_PORT.print(buffer[bufferIndex]);
        DEBUGSERIAL_PORT.print(" ");
        DEBUGSERIAL_PORT.println(bufferIndex);
      #endif
  
      // '[' recieved to denote start of command sequence
      if (bufferIndex == 0 && buffer[bufferIndex] == '[') {
        #ifdef SERIALDEBUG
          DEBUGSERIAL_PORT.print(millis());
          DEBUGSERIAL_PORT.println("ms\t SerialIO: Got start bracket");
        #endif
        bufferIndex++;
      }
      // Command value
      else if (bufferIndex == 1) {
        #ifdef SERIALDEBUG
          DEBUGSERIAL_PORT.print(millis());
          DEBUGSERIAL_PORT.print("ms\t SerialIO: Checking command ID of ");
          DEBUGSERIAL_PORT.println(buffer[1]);
        #endif
  
        // Don't continue if the command ID is invalid
        if(buffer[1] >= MAX_VALID_CMD) {
          #ifdef SERIALDEBUG
            DEBUGSERIAL_PORT.print(millis());
            DEBUGSERIAL_PORT.println("ms\t SerialIO: Command ID invalid");
          #endif
          bufferIndex = 0;
        }
        else {
          #ifdef SERIALDEBUG
            DEBUGSERIAL_PORT.print(millis());
            DEBUGSERIAL_PORT.println("ms\t SerialIO: Recieved more data");
          #endif
          bufferIndex++;
        }
      }
      // Check that the bufferIndex is for an argument
      else if (bufferIndex > 1 && (bufferIndex - 2) < commandLengthGet[buffer[1]] ) {
        bufferIndex++;
      }
      // ']' recieved denoting end of command sequence
      else if (buffer[bufferIndex] == ']') {
        #ifdef SERIALDEBUG
          DEBUGSERIAL_PORT.print(millis());
          DEBUGSERIAL_PORT.println("ms\t SerialIO: Got end of sequence");
        #endif
        execute_command(buffer[1]);
        bufferIndex = 0;
      }
      // If input is invalid, reset
      else {
        bufferIndex = 0;
      }
    }     
  }
}

/* Execute a serial command. References buffer for data. */
void execute_command(int commandID) {
  // Notes:
  //   Data is being sent as a decimal formatted string

  xLedOn(); // Turn command execute OK LED on (turned off in 1s loop)
  
  #ifdef SERIALDEBUG
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.println("ms\t SerialIO: Executing serial command");  
  #endif
  
  switch(commandID) {
    case VBATT1_VOLT:
    {
      send_command(commandID, get_batt1_voltage());
    } break;
    
    case VBATT2_VOLT:
    {
      send_command(commandID, get_batt2_voltage());
    } break;

    case DRIVE_MOTOR_VOLT: // Should be the current voltage we are writing out to the DAC
    {
      send_command(commandID, get_last_motor_voltage());
    }break;
    
    case ENGINE_RPM:
    {
      send_command(commandID, (double) get_current_megasquirt_data().rpm());
    } break;
    
    // TODO: FROM MEGASQUIRT?
    case ENGINE_TEMP:
    {
      send_command(commandID, -1);
    }break;
    
    case IAT: // Megasquirt ??
    {
      send_command(commandID, 12.3);
    }break;
    
    case LEFT_KILL:
    {
      send_command(commandID, get_kills(0));
    }break;
    
    case MAIN_BATTERY_VOLT:
    {
      send_command(commandID, get_batt_temperature());
    }break;
    
    case MAP:
    {
      send_command(commandID, (double) get_current_megasquirt_data().map());
    }break;
    
    case MASTER_KILL:
    {
      send_command(commandID, get_kills(2));
    }break;
    
    case OXYGEN: // Megasquirt??
    {
      send_command(commandID, 1234.3);
    }break;
    
    case REL_WIND_SPEED:
    {
      send_command(commandID, get_rel_windspeed());
    }break;
    
    case RIGHT_KILL:
    {
      send_command(commandID, get_kills(1));
    }break;
    
    case SPEED_INST:
    {
      #ifdef USE_GPS_SPEED
        send_command(commandID, -999);
      #endif
      #ifndef USE_GPS_SPEED
        send_command(commandID, get_speed_MPH());
      #endif
    }break;
    
    case THROTTLE_PERCENT:
    {
      send_command(commandID, ((double) get_throttle()));
    }break;
    
    case TOTAL_DISTANCE:
    {
      send_command(commandID, ((double) get_total_distance()));
    }break;

    case ARM_STANDBY:
    {
      send_command(commandID, get_armstatus());
    }break;

    case DUMP_DATA: {
      #ifdef SERIALDEBUG
        DEBUGSERIAL_PORT.print(millis());
        DEBUGSERIAL_PORT.println("ms\t SerialIO: Starting Dump");
      #endif

      // Start dump
      BLUESERIAL_PORT.print('{');
      isDumping = true;
      execute_command(dumpCounter);
      dumpCounter++;
    } break;

    case ENGINE_AFR: {
      double afr = get_current_megasquirt_data().afrtgt1();
      send_command(commandID, afr / 10);
    } break;
    
    case CAP_VOLT: { send_command(commandID, get_gen_voltage()); } break;
    case GEN_VOLT: { send_command(commandID, get_gen_voltage()); } break;
    case DRIVE_VOLT: { send_command(commandID, get_gen_voltage()); } break;
    case KILL_1: { send_command(commandID, get_gen_voltage()); } break;
    case KILL_2: { send_command(commandID, get_gen_voltage()); } break;
    case KILL_3: { send_command(commandID, get_gen_voltage()); } break;
    case GEN_TEMP_1: { send_command(commandID, get_gen_voltage()); } break;
    case GEN_TEMP_2: { send_command(commandID, get_gen_voltage()); } break;
    case DRIVE_TEMP_1: { send_command(commandID, get_gen_voltage()); } break;
    case DRIVE_TEMP_2: { send_command(commandID, get_gen_voltage()); } break;
    case ENGINE_TEMP_2: { send_command(commandID, get_gen_voltage()); } break;
    case GEN_CURRENT: { send_command(commandID, get_gen_voltage()); } break;
    case DRIVE_CURRENT: { send_command(commandID, get_gen_voltage()); } break;
    case REGEN_CURRENT: { send_command(commandID, get_gen_voltage()); } break;
    
    default:
      // not a valid command, return some error (maybe boolean return or something)
      xLedOff(); // Don't show valid LED
      send_command(commandID, -1.0);
      break;
  }
}

/* Send a command and/or data to the tablet */
void send_command(int commandID, double data) {
  BLUESERIAL_PORT.write('[');
  BLUESERIAL_PORT.write(commandID);
  BLUESERIAL_PORT.print(data);
  BLUESERIAL_PORT.write(']');
}

/* Send a command and/or data to the tablet */
void send_command(int commandID, int data) {
  BLUESERIAL_PORT.write('[');
  BLUESERIAL_PORT.write(commandID);
  BLUESERIAL_PORT.print(data);
  BLUESERIAL_PORT.write(']');
}


