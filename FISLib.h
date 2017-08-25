#ifndef FISLib_h
#define FISLib_h

#include "Arduino.h"

#define WRITE_PULSEW 50
#define WRITE_STARTPULSEW 100
#define WRITE_START 15

class FISLib{
  public:
    FISLib(int ENABLE, int CLOCK, int DATA);
    void showText(String l0, String l1);
    void sendByte(int Byte);
    void startENA();
    void stopENA();
  private:
    int _PIN_WRITE_ENA;
    int _PIN_WRITE_CLK;
    int _PIN_WRITE_DATA;
};

#endif
