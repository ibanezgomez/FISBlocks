#include "Arduino.h"
#include "FISLib.h"

FISLib::FISLib(int ENABLE, int CLOCK, int DATA) {
  _PIN_WRITE_ENA = ENABLE;
  _PIN_WRITE_CLK = CLOCK;
  _PIN_WRITE_DATA = DATA;

  pinMode(_PIN_WRITE_ENA, OUTPUT);
  digitalWrite(_PIN_WRITE_ENA, LOW);
  pinMode(_PIN_WRITE_CLK, OUTPUT);
  digitalWrite(_PIN_WRITE_CLK, HIGH);
  pinMode(_PIN_WRITE_DATA, OUTPUT);
  digitalWrite(_PIN_WRITE_DATA, HIGH);
}

void FISLib::showText(String l0, String l1){
  l0.toUpperCase();
  l1.toUpperCase();
  int l0_length=l0.length();
  int l1_length=l1.length();
  uint8_t dataCRC=0;

  if(l0_length<=8) for(int i=0;i<(8-l0_length);i++) l0+=" ";
  if(l1_length<=8) for(int i=0;i<(8-l1_length);i++) l1+=" ";

  dataCRC=(0xFF^WRITE_START);

  startENA();
  sendByte(WRITE_START);

  for(int i = 0; i <= 7; i++){
    sendByte(0xFF^l0[i]);
    dataCRC+=l0[i];
  }

  for(int i = 0; i <= 7; i++){
    sendByte(0xFF^l1[i]);
    dataCRC+=l1[i];
  }

  sendByte(dataCRC%0x100);
  stopENA();
}

void FISLib::sendByte(int Byte){
  static int iResult[8];
  for (int i = 0; i <= 7; i++){
    iResult[i] = Byte % 2;
    Byte = Byte / 2;
  }
  for(int i=7;i>=0;i--){
    switch (iResult[i]) {
      case 1: digitalWrite(_PIN_WRITE_DATA,HIGH);
              break;
      case 0:digitalWrite(_PIN_WRITE_DATA,LOW);
             break;
      }
      digitalWrite(_PIN_WRITE_CLK,LOW);
      delayMicroseconds(WRITE_PULSEW);

      digitalWrite(_PIN_WRITE_CLK,HIGH);
      delayMicroseconds(WRITE_PULSEW);
  }
}

void FISLib::startENA(){
  if (!digitalRead(_PIN_WRITE_ENA)) digitalWrite(_PIN_WRITE_ENA,HIGH);
}

void FISLib::stopENA(){
 digitalWrite(_PIN_WRITE_ENA,LOW);
}
