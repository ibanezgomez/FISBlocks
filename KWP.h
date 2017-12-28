#ifndef KWP_h
#define KWP_h

#include <inttypes.h>
#include <Arduino.h>

#include "NewSoftwareSerial.h"

#define ADR_Engine 0x01 // Engine
#define ADR_Gears  0x02 // Auto Trans
#define ADR_ABS_Brakes 0x03
#define ADR_Airbag 0x15
#define ADR_Dashboard 0x17 // Instruments
#define ADR_Immobilizer 0x25
#define ADR_Central_locking 0x35
#define ADR_Navigation 0x37

struct KWP_MODULE{
  String name;
  uint8_t addr;
  int baudrate;
  int *groups;
  int ngroups;
};

struct SENSOR {
  int type;
  int a;
  int b;
  String desc;
  String value;
  String units;
};

class KWP {
  public:
    KWP(uint8_t receivePin, uint8_t transmitPin);
    ~KWP();
    bool connect(uint8_t addr, int baudrate);
    void disconnect();
    int readBlock(uint8_t addr, int group, int maxSensorsPerGroup, SENSOR resGroupSensor[]);
    SENSOR getSensorData(byte k, byte a, byte b);
    String getBlockDesc(uint8_t addr, int block);
    bool isConnected();
    uint8_t getCurrAddr();
  private:
    uint8_t _OBD_RX_PIN;
    uint8_t _OBD_TX_PIN;

    bool connected = false;
    uint8_t blockCounter = 0;
    uint8_t errorTimeout = 0;
    uint8_t errorData = 0;

    NewSoftwareSerial *obd;

    void obdWrite(uint8_t data);
    uint8_t obdRead();
    void send5baud(uint8_t data);
    bool KWP5BaudInit(uint8_t addr);
    bool KWPSendBlock(char *s, int size);
    bool KWPReceiveBlock(char s[], int maxsize, int &size, bool init_delay = false);
    bool KWPSendAckBlock();
    bool readConnectBlocks();

};

#endif

