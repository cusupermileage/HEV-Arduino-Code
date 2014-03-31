/*
 * Output Functions
 *
 * - Write control voltage through DAC for DC/DC converter
 * - Send disarm signal to relay control box
 * - Write PWM signal to drive throttle servo on gas vehicles
 *
 */
double synq_voltage = 0.0;

// LDAC pin is LED for dummy pin
MCP4822 dacOut = MCP4822(PIN_DAC_CS,PIN_SERVO);

// Servo configuration
#ifdef VARIANT_GLII
  Servo throttleServo;
#endif

void setup_outputs() {
  pinMode(PIN_ARMDISARMSET, OUTPUT);
  pinMode(PIN_SERVO, OUTPUT);
  
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
  DDRE  |= 0b00000100;  // Set up HWB (EXP led) as output

  // Initialize servo
  #ifdef VARIANT_GLII
    throttleServo.attach(PIN_SERVO);
  #endif

  // Initialize DAC
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  SPI.begin();
  dacOut.setGain2X_AB();
  
  // Zero outputs
  #ifdef VARIANT_STING
    set_motor_voltage(0);
    delay(10);
    set_motor_voltage(0);
  #endif
  
  #ifdef DEBUGMODE
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.println("ms\t Outputs: Initialization Complete");
  #endif
  
  // Arm delay for Sting so DCDC doesn't freak out
  #ifdef VARIANT_STING
    delay(100);
  #endif
  
  arm_vehicle();
}

void xLedOn() {
  PORTE |= 0b00000100;
}
void xLedOff() {
  PORTE &= ~(0b00000100);
}

// Output to synqor through DAC over SPI
void set_motor_voltage(int percent) {
  // Check this multiplier
  // Possibly use the map function. See documentation.
  // TODO: Check against defined maximum cutoff voltage to avoid destroying the motor!
  
  unsigned int outvolts = percent * MOTOR_MAX_VOLTAGE;  
  
  // Set up the map function to do the whole range of values for the dcdc. The constrain above will handle limiting.
  // Multiplication by 1000 gets this into the integer range needed by the setValue_A method. Note that map() is int-only.
  unsigned int dacvolts = map(outvolts,0,MOTOR_MAX_VOLTAGE * 100,SYNQOR_MIN_TRIM*1000,SYNQOR_MAX_TRIM*1000); 

  dacOut.setValue_A(dacvolts); // Double-check this function call and its physical output

  synq_voltage = outvolts/100;

  #ifdef OUTPUTDEBUG
  #ifdef VARIANT_STING
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.print("ms\t Output: DAC Channel A set to ");  
    DEBUGSERIAL_PORT.print(dacvolts);
    DEBUGSERIAL_PORT.print(" mV mapping to DCDC output of ");
    DEBUGSERIAL_PORT.print(synq_voltage);
    DEBUGSERIAL_PORT.println(" V");
  #endif
  #endif
}

// Output to the servo for throttle position. Might need to have the option for
// offsets or something of that sort. Should argument be a percentage?
void set_throttle_position(double throttlePercent) {
  double throttleDegrees = map(throttlePercent, 0, 100, SERVO_MIN_DEGREES, SERVO_MAX_DEGREES);     // scale it to use it with the servo (value between 0 and 180) 
#ifdef VARIANT_GLII
  throttleServo.write(throttleDegrees);                        // sets the servo position according to the scaled value 
#endif
  #ifdef OUTPUTDEBUG
  #ifdef VARIANT_GLII
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.print("ms\t Output: Set throttle position to ");  
    DEBUGSERIAL_PORT.print(throttlePercent);  
    DEBUGSERIAL_PORT.print("% (");
    DEBUGSERIAL_PORT.print(throttleDegrees);
    DEBUGSERIAL_PORT.println(" Degrees)");  
  #endif
  #endif
}

double get_last_motor_voltage() {
  return synq_voltage;
}

// Output to the arm/disarm relay (thru NPN)
void arm_disarm(boolean isArmed) {
  // High triggers the transistor which disarms, right? Double-check this.
  digitalWrite(PIN_ARMDISARMSET, !isArmed);
  #ifdef OUTPUTDEBUG
    DEBUGSERIAL_PORT.print(millis());
    DEBUGSERIAL_PORT.print("ms\t Output: Arm Output set to ");  
    DEBUGSERIAL_PORT.println(isArmed);  
  #endif
}

void process_throttle() {
  #ifdef VARIANT_GLII
    // Set servo position to match hand throttle value
    set_throttle_position(get_throttle()); 
  #endif
  #ifdef VARIANT_STING
    // Set DAC output to match hand throttle value
    set_motor_voltage(get_throttle());
  #endif
}

void arm_vehicle() {
  // Arm the vehicle
  digitalWrite(PIN_ARMDISARMSET, HIGH);
}
void disarm_vehicle() {
  // Disarm the vehicle
  digitalWrite(PIN_ARMDISARMSET, LOW); 
}
