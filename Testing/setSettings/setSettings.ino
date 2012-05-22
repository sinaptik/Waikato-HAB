#include <SoftwareSerial.h>

SoftwareSerial nss(3, 4, false, true);

byte term[255];
int index;
int length;

byte UcReset[] = {0xFB, 0x05, 0x04, 0x00, 0xFF, 0x01, 0x00};
byte SwReset[] = {0xFB, 0x02, 0x02, 0x00};
byte SaveEEPROM[] = {0xFB, 0x05, 0x04, 0xFF, 0xFF, 0x01, 0x02};  //save and reset

byte lockToOneMW[] = {0xFB, 0x05, 0x04, 0x18, 0x00, 0x01, 0x10};
byte setDataRate[] = {0xFB, 0x05, 0x04, 0x01, 0x00, 0x01, 0x03};  //set to 38.4kbps

byte getXmitPower[] = {0xFB, 0x04, 0x03, 0x18, 0x00, 0x01};

byte getDeviceMode[] = {0xFB, 0x04, 0x03, 0x00, 0x00, 0x01};

byte setBaseSlotSize[] = {0xFB, 0x05, 0x04, 0x02, 0x01, 0x01, 0xC8};  //loc 2 bank 1 span 1 val 200
byte setMaxPropDelay[] = {0xFB, 0x05, 0x04, 0x09, 0x01, 0x01, 0xFF};  //loc 9 bank 1 span 1 val 255

byte getRegister[] = {0xFB, 0x04, 0x03, 0x1C, 0x00, 0x10};
//                                      reg   bank  span

void setup() {  
  Serial.begin(115200);
  
  Serial.println("Arduino started");
  
  nss.begin(9600);
}

void loop() {
  nss.listen();
  readRadio();
  
  //if we have something to read
  if (length == index && length > 0)
    printTerm();

  sendCommand();
}

void readRadio () {
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
      //Serial.println(b, HEX);
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
    
    for (int i = 0; i < length; i++) {
      Serial.print((char)term[i]);
    }
    
    Serial.println();
    
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

void sendCommand () {
  if (Serial.available())
  {
    char input = Serial.read();
    
    switch (input)
    {
      //get register
      case (char)'a':
      sendCmd(getRegister);
      break;

      //tx data
      case (char)'t':
      nss.write(0xFB);
      nss.write(0x08);
      nss.write(0x05);
      nss.write(0x02);
      nss.write(0x10);
      nss.write(0x00, HEX);
      nss.write(0x70);
      nss.write(0x72);
      nss.write(0x6F);
      nss.write(0x2E);
      break;

      //set protocol mode
      case (char)'p':
      nss.write(0xFB);
      nss.write(0x05);
      nss.write(0x04);
      nss.write(0x00, HEX);
      nss.write(0x04);
      nss.write(0x01);
      nss.write(0x01);
      break;

      //make remote
      case (char)'R':
      nss.write(0xFB);
      nss.write(0x05);
      nss.write(0x04);
      nss.write(0x00, HEX);
      nss.write(0x00, HEX);
      nss.write(0x01);
      nss.write(0x00, HEX);
      break;

      //make base
      case (char)'B':
      nss.write(0xFB);
      nss.write(0x05);
      nss.write(0x04);
      nss.write(0x00, HEX);
      nss.write(0x00, HEX);
      nss.write(0x01);
      nss.write(0x01);
      break;
      
      case (char)'L':
      sendCmd(lockToOneMW);
      break;
      
      case (char)'l':
      sendCmd(getXmitPower);
      break;
      
      case (char)'D':
      sendCmd(setDataRate);
      break;

      //save EEPROM
      case (char)'S':
      sendCmd(SaveEEPROM);
      break;

      //Am I base or remote?
      case (char)'w':
      sendCmd(getDeviceMode);
      break;
            
      case (char)'r':
      sendCmd(UcReset);
      break;
      
      default:
      Serial.print("Length: "); Serial.println(length);
      Serial.print("Index: "); Serial.println(index);
    }
  }
  
  Serial.flush();
}
