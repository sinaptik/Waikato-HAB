#include <SoftwareSerial.h>


//SoftwareSerial mySerial(ReceivePin, TransmitPin, reverse, isdnt);
SoftwareSerial nss(3, 4, false, true);

const int switchPin = 12;
const int ledPin = 13;

byte term[255];
int index;
int length;

byte setToOneMW[]   = {0xFB, 0x05, 0x04, 0x18, 0x00, 0x01, 0x10};
byte setToOneWatt[] = {0xFB, 0x05, 0x04, 0x18, 0x00, 0x01, 0x05};

byte SaveEEPROM[] = {0xFB, 0x05, 0x04, 0xFF, 0xFF, 0x01, 0x02};  //save and reset

void setup() 
{                
  pinMode(ledPin, OUTPUT);     
  pinMode(switchPin, INPUT);
  
  Serial.begin(115200);
  Serial.println("Arduino started");
  nss.begin(9600);
  
  delay(1000);
  
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
    
    sendCmd(SaveEEPROM);
  }
}

void loop() 
{

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
