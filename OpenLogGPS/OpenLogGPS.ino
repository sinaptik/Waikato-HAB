#include <PString.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <DHT22.h>

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
  nss.begin(4800);
  radio.begin(9600);
    
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
  
  digitalWrite(ledPin, HIGH);
  gpsdump(gps);
  digitalWrite(ledPin, LOW);
    
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

  bufferString.begin();
  bufferString.print(timeStamp); bufferString.print(",");
  bufferString.print(hour); bufferString.print(":"); bufferString.print(minute); bufferString.print(":"); bufferString.print(second); bufferString.print(",");
  bufferString.print(latitude); bufferString.print(",");
  bufferString.print(longditude); bufferString.print(",");
  bufferString.print(gps.altitude()); bufferString.print(",");
  bufferString.print(gps.speed()); bufferString.print(",");
  bufferString.print(gps.hdop()); bufferString.print(",");
  bufferString.println(age);
  //send lipo votage
  //send last message strength?
    
  sendBufferOverRadio(bufferString.length());
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
