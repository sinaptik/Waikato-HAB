#include "FileLogger.h"

// variable used when reading from serial
byte inSerByte = 0;

#define MESSAGE "Hello, this is my message. Just testing the FileLogger library.\r\n"
unsigned long length = sizeof(MESSAGE)-1;
byte buffer[] = MESSAGE;

byte millisBuffer[20];

byte linefeed[] = "\r\n";

void setup(void) {
  Serial.begin(9600);
  
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);
  
  Serial.println("done booting");
}

void loop(void) {

  // Arduino expects one of a series of one-byte commands
  if (Serial.available() > 0) {
    int result;
    inSerByte = Serial.read();
    switch (inSerByte) {
      case 'W':
        result = FileLogger::append("data.log", buffer, length);
        Serial.print(" Result: ");
        if( result == 0) {
          Serial.println("OK");
        } else if( result == 1) {
          Serial.println("Fail initializing");
        } else if( result == 2) {
          Serial.println("Fail appending");
        }
      break;
    case 'T':
	  for(int i=0; i<10; i++) {
	      //result = FileLogger::append("data.log", buffer, length);

              ltoa(millis(), (char*)millisBuffer, 10);

              FileLogger::append("data.log", millisBuffer, 20);
              result = FileLogger::append("data.log", linefeed, sizeof(linefeed)-1);
              
              Serial.print(" Result: ");
              if( result == 0) {
                Serial.println("OK");
              } else if( result == 1) {
                Serial.println("Fail initializing");
              } else if( result == 2) {
                Serial.println("Fail appending");
              }
	  }
          Serial.print("Done");
      break;
    }
  }
}

