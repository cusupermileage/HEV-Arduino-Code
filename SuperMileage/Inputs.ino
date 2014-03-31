/*
 * Input Functions
 *
 * - Read state of throttle, count wheel pulses, etc.
 *
 */

// Wheel Encoder: calculating distance per pulse
#define IPSTOMPH 0.0568f
#define INCHESPERMILE 63360.0f
#define INCHESPERPULSE (CIRCUMFERENCE/PULSESPERROTATION)

// Throttle Input: ADC mapping constants
#define ADCMAX 1023
#define ADCMIN 0
#define ADCCONVERSIONCONSTANT 0.029

//SPI ADC Defines
#define SPI_ADC_CHANNEL_0 0
#define SPI_ADC_CHANNEL_1 1
#define SPI_ADC_CHANNEL_2 2
#define SPI_ADC_CHANNEL_3 3
#define SPI_ADC_CHANNEL_4 4
#define SPI_ADC_CHANNEL_5 5
#define SPI_ADC_CHANNEL_6 6
#define SPI_ADC_CHANNEL_7 7

OneWire oneWire(PIN_1WIRETEMP);
DallasTemperature tempSensors(&oneWire);
DeviceAddress motorTemp, batteryTemp;


// Count of wheel pulses, update by ISR
volatile int wheelPulses = 0;
volatile unsigned long totalWheelPulses = 0;

volatile uint8_t motor_temperature = 0;
volatile uint8_t battery_temperature = 0;

double currentSpeedMPH = 0;
double total_distance = 0;
unsigned long pulse_time[10] = {0,0,0,0,0,0,0,0,0,0};
volatile unsigned long last_time = 0;
volatile unsigned long current_time = 0;
uint8_t time_count = 0;
volatile unsigned long last_temp_time = 0;

/* Configure inputs */
void setup_inputs() {

  // Set pins to input
  pinMode(PIN_KILL1, INPUT); 
  pinMode(PIN_KILL2, INPUT); 
  pinMode(PIN_KILL3, INPUT); 
  pinMode(PIN_PITOT, INPUT); 
  pinMode(PIN_VBATT1, INPUT); 
  pinMode(PIN_VBATT2, INPUT); 
  pinMode(PIN_ARMSTATUS, INPUT); 
  pinMode(PIN_AUTOMANUALSW, INPUT); 
  pinMode(PIN_THROTTLE, INPUT); 
  
  digitalWrite(PIN_KILL1, HIGH);
  digitalWrite(PIN_KILL2, HIGH);
  digitalWrite(PIN_KILL3, HIGH);

  pinMode(PIN_WHEELSPEED, INPUT); 
  digitalWrite(PIN_WHEELSPEED, HIGH); 
  attachInterrupt(ISR_WHEELSPEED,wheel_speed_ISR, EVENT_WHEELSPEED); 
  
  // Temperature sensors
  tempSensors.begin();
  tempSensors.setWaitForConversion(false);
  
  #ifdef DEBUGMODE
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.print("ms\t Inputs: Found ");
    DEBUGSERIAL_PORT.print(tempSensors.getDeviceCount());
    DEBUGSERIAL_PORT.println(" temperature sensors");
  #endif
  
  // Find temp sensors by index. Swap these variables depending on the order they show up on the bus
  #ifdef TEMPERATUREDEBUG
  if (!tempSensors.getAddress(motorTemp, 0)) {
    
    DEBUGSERIAL_PORT.print(millis());
      DEBUGSERIAL_PORT.println("ms\t Inputs: Unable to find motor temperature sensor, check connections!"); 
    
  }
  else {
    
      DEBUGSERIAL_PORT.print(millis());
      DEBUGSERIAL_PORT.println("ms\t Inputs: Found motor temperature sensor 1!"); 
    
  } 
  if (!tempSensors.getAddress(batteryTemp, 1)) {
    
      DEBUGSERIAL_PORT.print(millis());
      DEBUGSERIAL_PORT.println("ms\t Inputs: Unable to find battery temperature sensor, check connections!"); 
    
  }
  else {
    
      DEBUGSERIAL_PORT.print(millis());
      DEBUGSERIAL_PORT.println("ms\t Inputs: Found motor temperature sensor 2!"); 
    
  }
  #endif
  
  
  tempSensors.setResolution(motorTemp, TEMPERATURE_PRECISION);
  tempSensors.setResolution(batteryTemp, TEMPERATURE_PRECISION);


  #ifdef DEBUGMODE
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.println("ms\t Inputs: Initialization Complete");
  #endif

  for(int i=0; i<10; i++)
    pulse_time[i] = 0;

}

/* Get status of killswitch */
boolean get_kills(int killID) {
  switch(killID) {
    case 0: // LEFT
      return digitalRead(PIN_KILL1);
    case 1: // RIGHT
      return digitalRead(PIN_KILL2);
    case 2: // MASTER
      return digitalRead(PIN_KILL3);
    default:
      return true;
  }
}

/* Get state of throttle, scale to 0-100 percent? */
int get_throttle() {
  #ifdef VARIANT_STING
    int throtAdc = analogRead(PIN_THROTTLE);
    // TODO: Implement cutoff based on engine RPM
    //  double throttle = ((analogRead(PIN_THROTTLE) - THROTTLEMIN) * ADCMAX * ADCCONVERSIONCONSTANT);
    
    // This does not have fractional granularity (e.g., returns int). Can multiply by constant to do fixed-point math.
    int res = map(throtAdc,THROTTLEMIN,THROTTLEMAX, 0, 100);
    res = constrain(res , 0, 100);
    #ifdef INPUTDEBUG
      DEBUGSERIAL_PORT.print(millis());
      DEBUGSERIAL_PORT.print("ms\t Input: Read throttle ADC of ");  
      DEBUGSERIAL_PORT.print(analogRead(PIN_THROTTLE));  
      DEBUGSERIAL_PORT.print(" counts, converted to ");  
      DEBUGSERIAL_PORT.print(res);
      DEBUGSERIAL_PORT.println("%");  
    #endif 
    return res;
  #endif
  
  
  #ifdef VARIANT_GLII
  
  
    //-18 to 983
    int tps = get_current_megasquirt_data().tps();
    tps = map(tps,-18,983,0,100);
    tps = constrain(tps,0,100);
    
    //#ifdef INPUTDEBUG
      //DEBUGSERIAL_PORT.print(millis());
      //DEBUGSERIAL_PORT.print("ms\t ***** Input: Read throttle from MegaSquirt of ");  
      //DEBUGSERIAL_PORT.println(tps);  
      return tps;
    //#endif
  #endif
}

//TODO
/* Get pitot tube value */
double get_rel_windspeed() {
  int val = analogRead(PIN_PITOT);
  
  double voltage = (double) val * (5.000/1024.00);
  double relWindSpeed = (1.2592 * (voltage)*(voltage)*(voltage))-(9.9568*(voltage)*(voltage))+(32.123*(voltage))+1.950;
  
  #ifdef DEBUGMODE
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.print("ms/t Input: Windspeed of: ");
    DEBUGSERIAL_PORT.println(relWindSpeed);
  #endif
  
  if(relWindSpeed < 5.0){
    //not accurate under 5 mph
    return 0.0;
  }
}

// Get battery voltage. Will need voltage divider circuit. Output as 8-bit.
uint8_t get_batt1_voltage() {
  int val = analogRead(PIN_VBATT1);
  double voltage = 5*12/1024*val;
  
  return voltage;
}

double get_batt2_voltage() {  //REDO FOR OUR DIVIDER
  double val = analogRead(PIN_VBATT2);
  // 1.8 counts per volt
  //DEBUGSERIAL_PORT.print("Counts: ");
  //DEBUGSERIAL_PORT.println(val);
  return val / 19.01683;
}

uint8_t get_batt_temperature() {
  return motor_temperature;
}

// Get battery temperature over 1-wire bus, return in units of some sort. Output as 8-bit integer.
void request_temperatures() {
  
  tempSensors.requestTemperatures();
  
  // Conversions should be ready simultaneouslyish
  while(!tempSensors.isConversionAvailable(batteryTemp)) {
   motor_temperature = tempSensors.getTempF(batteryTemp);
   battery_temperature = tempSensors.getTempF(motorTemp);
  }
  /*
  DEBUGSERIAL_PORT.print(millis());
  DEBUGSERIAL_PORT.print("ms\t Input: Read battery temperature of ");  
  DEBUGSERIAL_PORT.print(val);  
  DEBUGSERIAL_PORT.println(" F");  
  */
}

// Get motor temperature over 1-wire bus
uint8_t get_motor_temperature() {
  return battery_temperature;
}

// Read arm button
boolean get_armstatus() {
  return digitalRead(PIN_ARMSTATUS);
}

// Get auto/manual switch status
boolean get_ismanual() {
  return digitalRead(PIN_AUTOMANUALSW);  
}

void check_stopped() {
  current_time = micros();
  if(current_time - last_time > 1000000) {
    for(int i=0;i<10;i++){
      pulse_time[i] = 0;
    }
  }
}

/* ISR for wheel encoder */
void wheel_speed_ISR() {
  ++wheelPulses;
  
  // 10-13-2012 MDS: Changed the speed calculation to be based
  //                 on micro seconds instead of miliseconds
  current_time = micros();
  pulse_time[time_count] = current_time - last_time;
  
  #ifdef DEBUGMODE
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.print("ms\t Interupt! ");  
    DEBUGSERIAL_PORT.println(pulse_time[time_count]);
  #endif
  
  #ifdef CAPTUREDATA
    Serial.print((unsigned long)current_time);
    Serial.print(',');
    Serial.println((unsigned long)(pulse_time[time_count]));
    Serial.flush();
  #endif
  
  time_count = ((time_count + 1) % 10);
  last_time = current_time;

}

/* Accessor method for wheel pulse data */
int get_wheel_pulses() {
  return wheelPulses;
}

/* MUST be called every 1 seconds to calculate speed */
/*
void calculate_wheelspeed_1s() {
  //detachInterrupt(ISR_WHEELSPEED);
  currentSpeedMPH = wheelPulses * DISTPERPULSE;
  total_distance = wheelPulses * DISTPERPULSE;
  totalWheelPulses += wheelPulses;
  wheelPulses = 0;
  //attachInterrupt(ISR_WHEELSPEED,wheel_speed_ISR, EVENT_WHEELSPEED); 
  #ifdef DEBUGMODE
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.print("ms\t Input: Current speed of ");  
    DEBUGSERIAL_PORT.println(currentSpeedMPH);
  #endif
}
*/

// Get current speed in MPH
double get_speed_MPH() {
  
  unsigned long average_time_acc = 0;
  for(int i = 0; i < 10; i++) {
    average_time_acc += pulse_time[i];
  }
  double average_time = ((double)average_time_acc)/10;
  
  // 10-13-2012 MDS: Updated the calculation of MPH to be based on
  //                 micro-seconds, not miliseconds.
  if(average_time_acc ==0){
    currentSpeedMPH = 0.0;
  }else{
    currentSpeedMPH = ((INCHESPERPULSE / average_time) * 1000000 * IPSTOMPH);
  }
    
  #ifdef DEBUGMODE
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.print("ms\t Input: Current speed of ");  
    DEBUGSERIAL_PORT.print(currentSpeedMPH);
    DEBUGSERIAL_PORT.print(" Average time: ");  
    DEBUGSERIAL_PORT.println(average_time);
  #endif
  
  return currentSpeedMPH;
}

//TODO: reset distance
double get_total_distance() {
  total_distance = ((totalWheelPulses * (CIRCUMFERENCE/PULSESPERROTATION)) / INCHESPERMILE);
  return total_distance;
}

//External SPI ADC Module
void spi_adc_config()
{
  //use a 2 MHz clock
  SPI.setClockDivider(SPI_CLOCK_DIV32);
  
  //shift MSB first
  SPI.setBitOrder(MSBFIRST);
  
  //set the SPI mode
  //clock low when idle
  //data sampled on rising edge
  //Data clocked out on falling edge
  SPI.setDataMode(SPI_MODE0);
  
  // Use PB0 (#17) for the CS line Spare-2
  SPI.begin(); 
 
 pinMode(SS, OUTPUT); 
}

// Read the specified address from the ADC
int spi_adc_read(byte address, int chipSelect)
{
  #ifdef DEBUGMODE
    DEBUGSERIAL_PORT.println("Reading ADC");
    DEBUGSERIAL_PORT.println(address);
  #endif
  // Bring the CS line low
  digitalWrite(SS, LOW);
  
  // transfer the first byte, ignore the return value
  SPI.transfer(0x01);
  
  // Transfer the second, command byte
  byte response = SPI.transfer(0x80 | (address << 4));
  
  // formulate the 10 bit result
  int ret = (response & 0x03);
  ret <<= 8;
  
  response = SPI.transfer(0x00);
  ret |= response;
  
  // Set the CS line high
  digitalWrite(SS, HIGH);
  
  // return the ADC result
  return ret;
  
}

//Get the reading from channel one of the adc
  double get_generator_current() {
  
  int adc_result = spi_adc_read(SPI_ADC_CHANNEL_0, 2);
  double measured_voltage = (adc_result*5)/1023;
  
  #ifdef SPIDEBUG
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.print("ms\t SPI VALUE OF: ");  
    DEBUGSERIAL_PORT.print(adc_result);
    DEBUGSERIAL_PORT.print(".   measuredVoltage of: ");  
    DEBUGSERIAL_PORT.println(measured_voltage);
  #endif
  //PUT IN EQUATION FOR CONVERTING FROM THE VOLTAGE TO THE CURRENT VALUE'
  //FOR NOW WE'RE JUST RETURNING THE MEASURED VOLTAGE:
  
  return measured_voltage;
}

//Get the reading from channel one of the adc
double get_drive_current() {
  
  int adc_result = spi_adc_read(SPI_ADC_CHANNEL_1, 2);
  double measured_voltage = adc_result * 5.0 / 1023.0;
  
  #ifdef SPIDEBUG
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.print("ms\t SPI DRIVE VALUE OF: ");  
    DEBUGSERIAL_PORT.println(adc_result);
    DEBUGSERIAL_PORT.print(".   measuredVoltage of: ");  
    DEBUGSERIAL_PORT.println(measured_voltage);
  #endif
  //PUT IN EQUATION FOR CONVERTING FROM THE VOLTAGE TO THE CURRENT VALUE'
  //FOR NOW WE'RE JUST RETURNING THE MEASURED VOLTAGE:
  
  return measured_voltage;
}

double get_cap_voltage() {
  
  int adc_result = spi_adc_read(SPI_ADC_CHANNEL_0, 1);
  double measured_voltage = adc_result * 5.0 / 1023.0;
  
  return measured_voltage;
}

double get_gen_voltage() {
  
  int adc_result = spi_adc_read(SPI_ADC_CHANNEL_1, 1);
  double measured_voltage = adc_result * 5.0 / 1023.0;
  
  return measured_voltage;
}

double get_drive_voltage() {
  
  int adc_result = spi_adc_read(SPI_ADC_CHANNEL_2, 1);
  double measured_voltage = adc_result * 5.0 / 1023.0;
  
  return measured_voltage;
}
