/*
 * MegaSquirt Interface
 *
 * - Pulls data from MegaSquirt on Serial1 port
 * - Data object is available through get_current_megasquirt_data()
 * - Data cache is updated with process_megasquirt_getdata()
 * - Data is parsed on-demand from cache through object methods
 *
 */

// Object to parse MegaSquirt data
MegaSquirtData megaSquirtData;

// Temporary storage of serial data
byte MSTable[MS_TABLE_LENGTH];

// MegaSquirt initialization
void setup_megasquirt() {
  MegaSquirt::begin();
  
  #ifdef DEBUGMODE
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.println("ms\t MegaSquirt: Initialization Complete");
  #endif
}

// Grab data from MegaSquirt and load to object for on-demand parsing
void process_megasquirt_getdata() {
  if (MegaSquirt::getData(megaSquirtData.reg) != MS_COMM_SUCCESS) {
    // Comm failed
      #ifdef DEBUGMODE
        DEBUGSERIAL_PORT.print(millis());
        DEBUGSERIAL_PORT.println("ms\t MegaSquirt: Communication Failed!");
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.print("ms\t MegaSquirt: RPM: ");
    DEBUGSERIAL_PORT.println(megaSquirtData.rpm());

      #endif
  }
  else {
    //megaSquirtData.loadData(MSTable);
    #ifdef DEBUGMODE
      DEBUGSERIAL_PORT.print(millis());
      DEBUGSERIAL_PORT.print("ms\t MegaSquirt: RPM: ");
      DEBUGSERIAL_PORT.println(megaSquirtData.rpm());
    #endif
  }
}

// Get the current MegaSquirt data object for parsing
MegaSquirtData get_current_megasquirt_data() {
  return megaSquirtData;
}

// DEBUG: Print MegaSquirt data to the LCD
void print_lcd() {
  #ifdef DEBUGMODE
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.print("ms\t MegaSquirt: RPM: ");
    DEBUGSERIAL_PORT.println(megaSquirtData.rpm());
  #endif
  /*
  lcd.setCursor(0,1);
  lcd.print("RPM:");
  lcd.print(megaSquirtData.rpm());
  lcd.print(" ");
  lcd.print("TS:");
  lcd.print(megaSquirtData.tps());
  
  #ifdef DEBUGMODE
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.print("ms\t MegaSquirt: RPM=");
    DEBUGSERIAL_PORT.println(megaSquirtData.rpm());
  #endif
  */
  
}
