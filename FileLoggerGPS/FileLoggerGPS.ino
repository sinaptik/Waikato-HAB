#include <PString.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include "FileLogger.h"

TinyGPS gps;
SoftwareSerial nss(3, 4);
SoftwareSerial radio(5, 6, false, true);

int ledPin = 9;

static void gpsdump(TinyGPS &gps);
static bool feedgps();

byte buffer[256];
byte term[256];

PString bufferString((char*)buffer, sizeof(buffer));

int index, length;

void setup()
{
  Serial.begin(115200);
  
  nss.begin(4800);
  radio.begin(9600);
  
  //set up SD SS pin
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  
  //led pin
  pinMode(ledPin, OUTPUT);
}

void loop()
{
  bool newdata = false;
  unsigned long start = millis();
  
  // Every second we print an update
  while (millis() - start < 1000)
  {
    nss.listen();
    if (feedgps())
      newdata = true;
  }
  
  gpsdump(gps);
    
  //do other things here
}

static void gpsdump(TinyGPS &gps)
{ 
  long latitude, longditude;
  unsigned long age, date, time, chars;
  unsigned short sentences, failed;
  byte month, day, hour, minute, second, hundredths;
  int year;
  
  long timeStamp = millis();
  
  gps.get_position(&latitude, &longditude, &age);
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  gps.stats(&chars, &sentences, &failed);
  
  signed long gps_altitude = gps.altitude();
  unsigned long gps_speed = gps.speed();
  unsigned long gps_hdop = gps.hdop();

  bufferString.begin();
  bufferString.print(timeStamp); bufferString.print(",");
  bufferString.print(hour); bufferString.print(":"); bufferString.print(minute); bufferString.print(":"); bufferString.print(second); bufferString.print(",");
  bufferString.print(latitude); bufferString.print(",");
  bufferString.print(longditude); bufferString.print(",");
  bufferString.print(gps_altitude); bufferString.print(",");
  bufferString.print(gps_speed); bufferString.print(",");
  bufferString.print(gps_hdop); bufferString.print(",");
  bufferString.println(age);
  //send lipo votage
  //send last message strength?
    
  sendBufferOverRadio(bufferString.length());
  
  radio.listen();
  delay(200);
  readTxReply();
  int rssi = (int)term[5];
  if (rssi >= 128)
    rssi = rssi - 256;
  Serial.println(rssi);
  if (rssi == 127)
  {
    analogWrite(ledPin, 0);
  }
  else
  {
    if ((rssi + 128) >= 0 && (rssi + 128) <= 255)
      analogWrite(ledPin, rssi + 128);
  }
  
  if (FileLogger::append("data.log", buffer, bufferString.length()) == 0)
  {
    /*analogWrite(ledPin, 255);
    delay(50);
    analogWrite(ledPin, 0);*/
  }
  
  /*if (result == 0)
    Serial.println("SD WRITE OK!");
  
  Serial.print("mil "); Serial.println(timeStamp);
  Serial.print("lat "); Serial.println(latitude);
  Serial.print("lon "); Serial.println(longditude);
  Serial.print("alt "); Serial.println(gps_altitude);
  Serial.print("spd "); Serial.println(gps_speed);
  Serial.print("hdp "); Serial.println(gps_hdop);
  Serial.print("fai "); Serial.println(failed);
  Serial.println();*/

}

static void sendBufferOverRadio(int bufferLength)
{
  radio.write(0xFB);
  radio.write((byte)(bufferLength + 4));
  radio.write(0x05);
  radio.write((byte)0x00);
  radio.write((byte)0x00);
  radio.write((byte)0x00);
  
  for (int i = 0; i < bufferLength; i++)
    radio.write((byte)buffer[i]);
}

static bool feedgps()
{
  while (nss.available())
  {
    if (gps.encode(nss.read()))
      return true;
  }
  return false;
}

//
//can remove stuff under here
//

void rawRead() {
  while (radio.available())
  {
    Serial.println(radio.read(), HEX);
  }
}

void readTxReply()
{
  while (radio.available())
  {
    byte b = radio.read();
    
    if (b == (byte)251) {
      index = 0;
      length = 0;
    }
    else if (length == 0) {
        length = (int)b;
    }
    else {
      term[index++] = b;
    }
  }
}
