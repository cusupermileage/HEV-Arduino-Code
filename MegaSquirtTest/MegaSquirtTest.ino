#include <LiquidCrystal.h>

#include <MegaSquirt.h>
LiquidCrystal lcd(A5, A4, A3, A2, A1, A0);

byte MSTable[MS_TABLE_LENGTH];

void setup()  
{
  lcd.begin(16,2);
  lcd.print("Meh?");
  
  MegaSquirt::begin();
  
  Serial.begin(57600);
  Serial.println("Test starting");
}

int t;

void loop() // run over and over
{
  t = millis();
  byte status = MegaSquirt::getData(MSTable);
  if (status != MS_COMM_SUCCESS) {
    Serial.println("MS Comm Unsuccessful");
  }
  else {
    Serial.println("MS comm successful!");
  }
  
  MegaSquirtData data;
  data.loadData(MSTable);
  lcd.setCursor(0,0);
  lcd.print("TPS:");
  lcd.print(data.tps());
  lcd.setCursor(0,1);
  lcd.print("RPM:");
  lcd.print(data.rpm());
  lcd.print(" ");
  lcd.print(data.batteryVoltage());
  lcd.print("V");
  Serial.println("Loaded data");
  Serial.print("RPM: ");
  Serial.println(data.rpm());
  Serial.print("Duration: ");
  Serial.println(millis() - t);
  delay(1000);
}
