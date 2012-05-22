#include <TinyGPS.h>
#include <SoftwareSerial.h>

SoftwareSerial nss(3, 4, false, true);

byte term[255];
int index;
int length;

long previousMillis = 0;

const int ledPin =  13;

const int resetPin = 5;
const int HOST_RTS = 7;

byte UcReset[] = {0xFB, 0x05, 0x04, 0x00, 0xFF, 0x01, 0x00};
byte SwReset[] = {0xFB, 0x02, 0x02, 0x00};
byte SaveEEPROM[] = {0xFB, 0x05, 0x04, 0xFF, 0xFF, 0x01, 0x01};

byte lockToOneMW[] = {0xFB, 0x05, 0x04, 0x18, 0x00, 0x01, 0x10};
byte setDataRate[] = {0xFB, 0x05, 0x04, 0x01, 0x00, 0x01, 0x03};

byte getXmitPower[] = {0xFB, 0x04, 0x03, 0x18, 0x00, 0x01};

byte getRegister[] = {0xFB, 0x04, 0x03, 0x1C, 0x00, 0x10};

int count;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  pinMode(HOST_RTS, OUTPUT);
  digitalWrite(HOST_RTS, LOW);
  
  digitalWrite(resetPin, LOW);
  digitalWrite(resetPin, HIGH);
  
  Serial.begin(115200);
  
  Serial.println("Arduino started");
  
  nss.begin(9600);
}

void loop() {
    
  digitalWrite(ledPin, HIGH);

  nss.listen();
  readRadio();
  
  //if we have something to read
  if (length == index && length > 0)
    crackTerm();
  
  digitalWrite(ledPin, LOW);
  
  sendCommand();
  
  //rawRead();
  //printTerm();
    
  //sendEvery(5000);
}

void rawRead() {
  while (nss.available())
  {
    Serial.println(nss.read(), HEX);
  }
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
    
    index = length = 0;
  }
}

void crackTerm () {
  
  switch ((byte)term[0]) {

    //Replies
    case (byte)0x10:
    Serial.println("EnterProtocolModeReply");
    //no other data
    break;
    
    case (byte)0x11:
    Serial.println("ExitProtocolModeReply");
    //no other data
    break;
    
    case (byte)0x12:
    Serial.println("SoftwareResetReply");
    //no other data
    break;
    
    case (byte)0x13:
    GetRegisterReply();
    break;
    
    case (byte)0x14:
    Serial.println("SetRegisterReply");
    break;
    
    case (byte)0x15:
    Serial.println("TxDataReply");
    TxDataReply();
    break;
    
    case (byte)0x1A:
    Serial.println("GetRemoteRegisterReply");
    break;
    
    case (byte)0x1B:
    Serial.println("SetRemoteRegisterReply");
    break;

    //Events
    case (byte)0x26:
    Serial.println("RxData");
    RxData();
    break;
    
    case (byte)0x27:
    Serial.println("Announce");
    Announce();
    break;
    
    case (byte)0x28:
    Serial.println("RxEvent");
    break;

    default:
    Serial.print("UNRECKOGNISED MESSAGE: "); Serial.println(term[0], HEX);
  }
  
  nss.flush();

  length = index = 0;
}

void sendCmd(byte array[])
{
  for (int i = 0; i < array[1] + 2; i++)
  {
    nss.write(array[i]);
  }
}

void RxData () {
  Serial.print("  Address: "); Serial.print(term[3], HEX); Serial.print(term[2], HEX); Serial.println(term[1], HEX);
  Serial.print("  RSSI: "); Serial.println((char)term[4], DEC);

  Serial.print("  Data: \"");
  for (int i = 5; i < length; i++)
  {
    Serial.print((char)term[i]);
  }
  Serial.println("\"");
  
  Serial.print("  Data (HEX): \"");
  for (int i = 5; i < length; i++)
  {
    Serial.print(term[i], HEX);
  }
  Serial.println("\"");
  
  if(length == 16) {
    Serial.print(term[5], DEC); Serial.print(":"); Serial.print(term[6], DEC); Serial.print(":"); Serial.println(term[7], DEC);
  
    long lat = 0;//(long)(term[8] << 24) | (term[9] << 16) | (term[10] << 8) | term[11];
    long lon = 0;//(long)(term[12] << 24) | (term[13] << 16) | (term[14] << 8) | term[15];
    
    lat = ((unsigned long) term[8]) << 24;
    lat |= ((unsigned long) term[9]) << 16;
    lat |= ((unsigned long) term[10]) << 8;
    lat |= ((unsigned long) term[11]);
    
    lon = ((unsigned long) term[12]) << 24;
    lon |= ((unsigned long) term[13]) << 16;
    lon |= ((unsigned long) term[14]) << 8;
    lon |= ((unsigned long) term[15]);
    
    Serial.print("lat: "); Serial.println(lat);
    Serial.print("lon: "); Serial.println(lon);
  }
}

void TxDataReply () {
  Serial.print("  TxStatus: ");
  switch ((byte)term[1]) {
  
    case (byte)0x00:
    Serial.println("ACK received");
    break;

    case (byte)0x01:
    Serial.println("NO ACK received");
    break;

    case (byte)0x02:
    Serial.println("Not linked (remote)");
    break;

    case (byte)0x03:
    Serial.println("NO ACK due to recipient holding for flow control");
    break;

    default:
    Serial.print("UNRECKOGNISED TX STATUS: "); Serial.println(term[1], HEX);
  }
  
  Serial.print("  MAC: "); Serial.print(term[4], HEX); Serial.print(term[3], HEX); Serial.println(term[2], HEX);
  
  Serial.print("  RSSI: "); Serial.println((char)term[5], DEC);
}

void Announce () {
  switch ((byte)term[1]) {
    
    case (byte)0xA0:
    Serial.println("  Radio has completed startup initialization");
    break;
    
    case (byte)0xA2:
    Serial.println("  Base: a remote has joined the network");
    Serial.print("  Remote MAC: "); Serial.print(term[4], HEX); Serial.print(term[3], HEX); Serial.println(term[2], HEX);
    Serial.print("  Range: "); Serial.println((unsigned int)term[6], DEC);
    break;
    
    case (byte)0xA3:
    Serial.println("  Remote: joined a network, ready for data");
    Serial.print("  Network ID: "); Serial.println(term[2], HEX);
    Serial.print("  Base MAC: "); Serial.print(term[5], HEX); Serial.print(term[4], HEX); Serial.println(term[3], HEX);
    Serial.print("  Range: "); Serial.println((unsigned int)term[6], DEC);
    break;
    
    case (byte)0xA4:
    Serial.println("  Remote: exited network (base is out of range)");
    Serial.print("  Network ID: "); Serial.println(term[2], HEX);
    break;
    
    case (byte)0xA5:
    Serial.println("  Remote: the base has been rebooted");
    break;
    
    case (byte)0xA7:
    Serial.println("  Base: remote has left the network");
    Serial.print("  Remote MAC: "); Serial.print(term[4], HEX); Serial.print(term[3], HEX); Serial.println(term[2], HEX);
    break;
    
    case (byte)0xA8:
    Serial.println("  Base: heartbeat received from router or remote");
    break;

    //Errors
    case (byte)0xE0:
    Serial.println("  Protocol error - invalid message type");
    break;
    
    case (byte)0xE1:
    Serial.println("  Protocol error - invalid argument");
    break;
    
    case (byte)0xE2:
    Serial.println("  Protocol error - general error");
    break;
    
    case (byte)0xE3:
    Serial.println("  Protocol error - parser timeout");
    break;
    
    case (byte)0xE4:
    Serial.println("  Protocol error - register is read-only");
    break;
    
    case (byte)0xE8:
    Serial.println("  UART receive buffer overflow");
    break;
    
    case (byte)0xE9:
    Serial.println("  UART receive overrun");
    break;
    
    case (byte)0xEA:
    Serial.println("  UART framing error");
    break;
    
    case (byte)0xEE:
    Serial.println("  hardware error");
    break;
    
    default:
    Serial.print("UNRECKOGNISED ANNOUNCE MESSAGE: "); Serial.println(term[1], HEX);
  }
}

void GetRegisterReply () {
  Serial.println("GetRegisterReply");
  
  Serial.print("  Reg: "); Serial.println(term[1], HEX);
  Serial.print("  Bank: "); Serial.println(term[2], HEX);
  Serial.print("  Span: "); Serial.println(term[3], HEX);
  Serial.print("  Val: ");
  for (int i = 4; i < term[3] + 4; i++)
  {
    Serial.print(term[i], HEX);
  }
  Serial.println();
  
  Serial.print("  Val (ASCII): \"");
  for (int j = 4; j < term[3] + 4; j++)
  {
    Serial.print((char)term[j]);
  }
  Serial.println("\"");
}

void sendEvery (long interval) {
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;   

    /*nss.write(0xFB);
    nss.write(0x0C);
    nss.write(0x05);
    nss.write(0x02);
    nss.write(0x10);
    nss.write(0x00, HEX);

    nss.write(0x48);
    nss.write(0x69);
    nss.write(0x01);
    nss.write(0x45);
    nss.write(0x6C);
    nss.write(0x6C);
    nss.write(0x65);
    nss.write(0x21);*/
    
    //sendDiagnostics();
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
      
      case (char)'d':
      sendDiagnostics();
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

      //Enter protocol from transparent
      case (char)'E':
      nss.write(0xFB);
      nss.write(0x07);
      nss.write(0x00, HEX);
      nss.write(0x44);
      nss.write(0x4E);
      nss.write(0x54);
      nss.write(0x43);
      nss.write(0x46);
      nss.write(0x47);
      break;

      //Am I base or remote?
      case (char)'w':
      nss.write(0xFB);
      nss.write(0x04);
      nss.write(0x03);
      nss.write(0x00, HEX);
      nss.write(0x00, HEX);
      nss.write(0x01);
      break;
      
      case (char)'r':
      //softRadioReset();
      /*nss.write(0xFB);
      nss.write(0x05);
      nss.write(0x04);
      nss.write(0x00, HEX);
      nss.write(0xFF);
      nss.write(0x01);
      nss.write(0x00, HEX);
      
      nss.write(0xFB);
      nss.write(0x05);
      nss.write(0x04);
      nss.write(0xFF);
      nss.write(0xFF);
      nss.write(0x01);
      nss.write(0x01);*/
      sendCmd(UcReset);
      break;
      
      default:
      Serial.print("Length: "); Serial.println(length);
      Serial.print("Index: "); Serial.println(index);
    }
  }
  
  Serial.flush();
}

void sendDiagnostics()
{
    //do every x seconds
  
    nss.write(0xFB);
    nss.write(0x05);
    nss.write(0x05);
    nss.write(0x02);
    nss.write(0x10);
    nss.write(0x00, HEX);
    nss.write((byte)count++);
    
    Serial.print("sent: "); Serial.println(count - 1);
}
