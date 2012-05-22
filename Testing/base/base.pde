#include <SoftwareSerial.h>

SoftwareSerial nss(3, 4, false, true);

const int switchPin = 12;
const int ledPin = 13;
int switchState = 0;

byte term[255];
int index;
int length;

byte setToOneMW[]   = {0xFB, 0x05, 0x04, 0x18, 0x00, 0x01, 0x00};
byte setToOneWatt[] = {0xFB, 0x05, 0x04, 0x18, 0x00, 0x01, 0x05};

void setup() 
{                
  pinMode(ledPin, OUTPUT);     
  pinMode(switchPin, INPUT);
  
  Serial.begin(115200);
  Serial.println("Arduino started");
  nss.begin(9600);
  
  delay(1000);
  
  //by default set to one mw
  sendCmd(setToOneMW);
  delay(10);
  readRadio();
  
  if (length == 1 && term[0] == 0x14)
  {
    Serial.println("set to one milliwatt");
    
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    
    switchState = digitalRead(switchPin);
    if (switchState == HIGH)
    {  
      sendCmd(setToOneWatt);
      delay(10);
      readRadio();
      
      if (length == 1 && term[0] == 0x14)
      {
        Serial.println("set to one watt");
        
        delay(500);
        digitalWrite(ledPin, HIGH);
        delay(1000);
        digitalWrite(ledPin, LOW);
      }
    }
  }
  
  digitalWrite(ledPin, LOW);
}

void loop() 
{
  nss.write(0xFB);
  nss.write(0x05);
  nss.write(0x05);
  
  nss.write(0x02);
  nss.write(0x10);
  nss.write(0x00);
  
  nss.write(0xAA);
  
  delay(1000);
  
  readRadio();
  
  if (term[0] == 0x15 && term[1] == 0x00)
  {
    //we got data
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    
    index = length = 0;
  }
}

void readRadio() {
  while (nss.available()) {
    
    byte b = nss.read();
    
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

void printTerm () {
  if (length == index && length > 0) {
    Serial.print("Length: "); Serial.println(length);
    for (int i = 0; i < length; i++) {
      Serial.print("  "); Serial.println(term[i], HEX);
    }
    
    index = length = 0;
  }
}

void sendCmd(byte array[])
{
  for (int i = 0; i < array[1] + 2; i++)
  {
    nss.write(array[i]);
  }
}
