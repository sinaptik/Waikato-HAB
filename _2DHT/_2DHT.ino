#include "DHT.h"

#define DHTL 2
#define DHTR 3

#define DHTTYPE DHT22   // DHT 22  (AM2302)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dhtL(DHTL, DHTTYPE);
DHT dhtR(DHTR, DHTTYPE);

void setup() 
{
  Serial.begin(9600); 
  Serial.println("DHTxx test!");
 
  dhtL.begin();
  dhtR.begin();
}

void loop() 
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float hl = dhtL.readHumidity();
  float tl = dhtL.readTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(tl) || isnan(hl)) 
  {
    Serial.println("Failed to read from DHT L");
  } 
  else 
  {
    Serial.print("Left: ");
    Serial.print(hl);
    Serial.print(" %\t");
    Serial.print(tl);
    Serial.println(" *C");
  }
  
    // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float hr = dhtR.readHumidity();
  float tr = dhtR.readTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(tr) || isnan(hr)) 
  {
    Serial.println("Failed to read from DHT R");
  } 
  else 
  {
    Serial.print("Right: ");
    Serial.print(hr);
    Serial.print(" %\t");
    Serial.print(tr);
    Serial.println(" *C");
  }
  
  delay(1000);
}
