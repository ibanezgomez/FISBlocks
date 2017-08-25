#include "KWP.h"
#include <Arduino.h>

#define DEBUG_LEVEL 1

KWP::KWP(uint8_t receivePin, uint8_t transmitPin){
  _OBD_RX_PIN = receivePin;
  _OBD_TX_PIN = transmitPin;

  pinMode(transmitPin, OUTPUT);
  digitalWrite(transmitPin, HIGH);

  obd = new NewSoftwareSerial(receivePin, transmitPin, false); // RX, TX, inverse logic

  #ifdef DEBUG_LEVEL
    Serial.println(F("KWP created"));
  #endif
}

KWP::~KWP(){
  delete obd;
  obd = NULL;
}

bool KWP::connect(uint8_t addr, int baudrate) {
  Serial.print(F("------connect addr="));
  Serial.print(addr);
  Serial.print(F(" baud="));
  Serial.println(baudrate);
  blockCounter = 0;
  obd->begin(baudrate);
  KWP5BaudInit(addr);
  char s[3];
  int size = 3;
  if (!KWPReceiveBlock(s, 3, size)) return false;
  if (    (((uint8_t)s[0]) != 0x55)
     ||   (((uint8_t)s[1]) != 0x01)
     ||   (((uint8_t)s[2]) != 0x8A)   ){
    Serial.println(F("ERROR: invalid magic"));
    disconnect();
    errorData++;
    return false;
  }
  connected = true;
  if (!readConnectBlocks()) return false;
  return true;
}

void KWP::disconnect() {
  connected = false;
}

int KWP::readBlock(uint8_t addr, int group, int maxSensorsPerBlock, SENSOR resGroupSensor[]) {
  SENSOR sensor;
  Serial.print(F("------readBlock "));
  Serial.println(group);
  char s[64];
  sprintf(s, "\x04%c\x29%c\x03", blockCounter, group);
  if (!KWPSendBlock(s, 5)) return false;
  int size = 0;
  KWPReceiveBlock(s, 64, size);
  if (s[2] != '\xe7') {
    Serial.println(F("ERROR: invalid answer"));
    disconnect();
    errorData++;
    return 0;
  }
  int count = (size-4) / 3;
  if (count > maxSensorsPerBlock) {
    Serial.println(F("ERROR: max sensors exceded"));
    disconnect();
    errorData++;
    return 0;
  }
  String blockDescs= getBlockDesc(addr, group);
  int len=blockDescs.length();
  char buf[len+1];
  blockDescs.toCharArray(buf, len+1);
  char* command = strtok(buf, ",");
  Serial.print(F("count="));
  Serial.println(count);
  int j=0;
  for (int idx=0; idx < count; idx++){
    byte k=s[3 + idx*3];
    byte a=s[3 + idx*3+1];
    byte b=s[3 + idx*3+2];
    String desc=String(command);
    SENSOR sensor = getSensorData(k, a, b);
    if(desc != "" && sensor.value != ""){
      resGroupSensor[j].type = sensor.type;
      resGroupSensor[j].a = sensor.a;
      resGroupSensor[j].b = sensor.b;
      resGroupSensor[j].desc = desc;
      resGroupSensor[j].value = sensor.value;
      resGroupSensor[j].units = sensor.units;
      j++;
    }
    command = strtok(0, ",");
  }
  return j;
}

SENSOR KWP::getSensorData(byte k, byte a, byte b) {
    SENSOR res;
    Serial.print(F("type="));
    Serial.print(k);
    Serial.print(F("  a="));
    Serial.print(a);
    Serial.print(F("  b="));
    Serial.print(b);
    Serial.print(F("  text="));
    String t = "";
    float v = 0;
    String units = "";
    char buf[32];
    switch (k){
      case 1:  v=0.2*a*b;             units=F("rpm"); break;
      case 2:  v=a*0.002*b;           units=F("%"); break; // case 2:  v=a*0.002*b;            units=F("%%");break;
      case 3:  v=0.002*a*b;           units=F("Deg"); break;
      case 4:  v=abs(b-127)*0.01*a;   units=F("ATDC"); break;
      case 5:  v=a*(b-100)*0.1;       units=F("c");break;
      case 6:  v=0.001*a*b;           units=F("v");break;
      case 7:  v=0.01*a*b;            units=F("km/h");break;
      case 8:  v=0.1*a*b;             units=F(" ");break;
      case 9:  v=(b-127)*0.02*a;      units=F("Deg");break;
      case 10: if (b == 0) t=F("COLD"); else t=F("WARM");break;
      case 11: v=0.0001*a*(b-128)+1;  units = F(" ");break;
      case 12: v=0.001*a*b;           units =F("Ohm");break;
      case 13: v=(b-127)*0.001*a;     units =F("mm");break;
      case 14: v=0.005*a*b;           units=F("bar");break;
      case 15: v=0.01*a*b;            units=F("ms");break;
      case 18: v=0.04*a*b;            units=F("mbar");break;
      case 19: v=a*b*0.01;            units=F("l");break;
      case 20: v=a*(b-128)/128;       units=F("%");break; // case 20: v=a*(b-128)/128;       units=F("%%");break;
      case 21: v=0.001*a*b;           units=F("V");break;
      case 22: v=0.001*a*b;           units=F("ms");break;
      case 23: v=b/256*a;             units=F("%");break; //case 23: v=b/256*a;             units=F("%%");break;
      case 24: v=0.001*a*b;           units=F("A");break;
      case 25: v=(b*1.421)+(a/182);   units=F("g/s");break;
      case 26: v=float(b-a);          units=F("C");break;
      case 27: v=abs(b-128)*0.01*a;   units=F("Deg");break; //case 27: v=abs(b-128)*0.01*a;   units=F("°");break;
      case 28: v=float(b-a);          units=F(" ");break;
      case 30: v=b/12*a;              units=F("Deg k/w");break;
      case 31: v=b/2560*a;            units=F("°C");break;
      case 33: v=100*b/a;             units=F("%");break; //case 33: v=100*b/a;             units=F("%%");break;
      case 34: v=(b-128)*0.01*a;      units=F("kW");break;
      case 35: v=0.01*a*b;            units=F("l/h");break;
      case 36: v=((unsigned long)a)*2560+((unsigned long)b)*10;  units=F("km");break;
      case 37: v=b; break; // oil pressure ?!
      // ADP: FIXME!
      /*case 37: switch(b){
             case 0: sprintf(buf, F("ADP OK (%d,%d)"), a,b); t=String(buf); break;
             case 1: sprintf(buf, F("ADP RUN (%d,%d)"), a,b); t=String(buf); break;
             case 0x10: sprintf(buf, F("ADP ERR (%d,%d)"), a,b); t=String(buf); break;
             default: sprintf(buf, F("ADP (%d,%d)"), a,b); t=String(buf); break;
          }*/
      case 38: v=(b-128)*0.001*a;        units=F("Deg k/w"); break;
      case 39: v=b/256*a;                units=F("mg/h"); break;
      case 40: v=b*0.1+(25.5*a)-400;     units=F("A"); break;
      case 41: v=b+a*255;                units=F("Ah"); break;
      case 42: v=b*0.1+(25.5*a)-400;     units=F("Kw"); break;
      case 43: v=b*0.1+(25.5*a);         units=F("V"); break;
      case 44: sprintf(buf, "%2d:%2d", a,b); t=String(buf); break;
      case 45: v=0.1*a*b/100;            units=F(" "); break;
      case 46: v=(a*b-3200)*0.0027;      units=F("Deg k/w"); break;
      case 47: v=(b-128)*a;              units=F("ms"); break;
      case 48: v=b+a*255;                units=F(" "); break;
      case 49: v=(b/4)*a*0.1;            units=F("mg/h"); break;
      case 50: v=(b-128)/(0.01*a);       units=F("mbar"); break;
      case 51: v=((b-128)/255)*a;        units=F("mg/h"); break;
      case 52: v=b*0.02*a-a;             units=F("Nm"); break;
      case 53: v=(b-128)*1.4222+0.006*a;  units=F("g/s"); break;
      case 54: v=a*256+b;                units=F("count"); break;
      case 55: v=a*b/200;                units=F("s"); break;
      case 56: v=a*256+b;                units=F("WSC"); break;
      case 57: v=a*256+b+65536;          units=F("WSC"); break;
      case 59: v=(a*256+b)/32768;        units=F("g/s"); break;
      case 60: v=(a*256+b)*0.01;         units=F("sec"); break;
      case 62: v=0.256*a*b;              units=F("S"); break;
      case 64: v=float(a+b);             units=F("Ohm"); break;
      case 65: v=0.01*a*(b-127);         units=F("mm"); break;
      case 66: v=(a*b)/511.12;          units=F("V"); break;
      case 67: v=(640*a)+b*2.5;         units=F("Deg"); break;
      case 68: v=(256*a+b)/7.365;       units=F("deg/s");break;
      case 69: v=(256*a +b)*0.3254;     units=F("Bar");break;
      case 70: v=(256*a +b)*0.192;      units=F("m/s^2");break;
      default: sprintf(buf, "%2x, %2x      ", a, b); break;
    }

    if (units.length() != 0){
      dtostrf(v,4, 2, buf);
      t=String(buf) + " " + units;
    }
    Serial.println(t);

    res.type = k;
    res.a = a;
    res.b = b;
    res.value = String(buf);
    res.units = units;
    return res;
}

String KWP::getBlockDesc(uint8_t addr, int block){
  String blockDescs;
  if(addr == ADR_Dashboard){
    switch (block){
      case 1: blockDescs=F("Speed,Engine Speed,Oil pressure,Time"); break;
      case 2: blockDescs=F("Odometer,Fuel lvl,FuelSend,TAmbient"); break; // case 2: blockDescs=F("Odometer,Fuel level (l),Fuel Sender,Ambient"); break;
      case 3: blockDescs=F("Coolant Temp,Oil level,Oil,[N/A]"); break;
      case 22: blockDescs=F("Starting,Engine (ECM),Key condition,Number of"); break;
      case 23: blockDescs=F("Variable code,Key status,Fixed code,Immobilizer"); break;
      case 24: blockDescs=F("Instrument,Engine control,Emergency,Transponder"); break;
      case 25: blockDescs=F("Immobilizer,[N/A],[N/A],[N/A]"); break;
      case 50: blockDescs=F("Odometer,Engine Speed,Oil,Coolant"); break;
      case 125: blockDescs=F("Engine,Transmission,ABS,[N/A]"); break;
      case 126: blockDescs=F("Steering,Airbag,[N/A],[N/A]"); break;
    default: blockDescs=F(""); break;
    }

  }
  //Label 06A-906-032-AUM.lbl
  else if(addr == ADR_Engine){
    switch (block){
      case 1: blockDescs=F("Engine Speed,Coolant,Lambda Control,Basic Setting"); break;
      case 2: blockDescs=F("Engine  speed,Load,Inj Time,MAF"); break; // case 2: blockDescs=F("Engine Speed,Engine Load,Injection Timing,Mass Air Flow"); break;
      case 3: blockDescs=F("Engine  speed,MAF,Throttle,Ignition"); break; // case 3: blockDescs=F("Engine Speed,Mass Air Flow,Throttle Valve Angle,Ignition"); break;
      case 4: blockDescs=F("Engine Speed,Battery Voltage,Coolant,Intake Air"); break;
      case 5: blockDescs=F("Engine Speed,Engine Load,Vehicle Speed,Load Status"); break;
      case 6: blockDescs=F("Engine Speed,Engine Load,Intake Air,Altitude"); break;
      case 10: blockDescs=F("Engine Speed,Engine Load,Throttle Valve Angle,Ignition"); break;
      case 11: blockDescs=F("Engine Speed,Coolant,Intake Air,Ignition"); break;
      case 14: blockDescs=F("Engine Speed,Engine Load,Misfire,Misfire"); break;
      case 15: blockDescs=F("Misfire1,Misfire2,Misfire3,Misfire4"); break; // case 15: blockDescs=F("Misfire Counter,Misfire Counter,Misfire Counter,Misfire"); break;
      case 16: blockDescs=F("Misfire Counter,Misfire"); break;
      case 18: blockDescs=F("Lower,Upper,Lower,Upper"); break;
      case 20: blockDescs=F("TimReta1,TimReta2,TimReta3,TimReta4"); break; //case 20: blockDescs=F("Timing Retardation,Timing Retardation,Timing Retardation,Timing Retardation"); break;
      case 22: blockDescs=F("Engine Speed,Engine Load,Timing Retardation,Timing Retardation"); break;
      case 23: blockDescs=F("Engine Speed,Engine Load,Timing Retardation,Timing Retardation"); break;
      case 26: blockDescs=F("Voltage,Voltage,Voltage,Voltage"); break;
      case 28: blockDescs=F("Engine Speed,Engine Load,Coolant,Result"); break;
      case 30: blockDescs=F("Bank 1,Bank 1"); break;
      case 31: blockDescs=F("LambdDes,LambdAct"); break; // case 31: blockDescs=F("Lambda Control,Lambda Control"); break;
      case 32: blockDescs=F("Adaptation (Idle),Adaptation (Partial)"); break;
      case 33: blockDescs=F("Lambda Control,Sensor Voltage"); break;
      case 34: blockDescs=F("Engine Speed,Catalytic Converter,Dynamic Factor,Result"); break;
      case 36: blockDescs=F("Sensor Voltage,Result"); break;
      case 37: blockDescs=F("Engine Load,Sensor Voltage"); break;
      case 41: blockDescs=F("Resistance,Heater Condition,Resistance,Heater Condition"); break;
      case 43: blockDescs=F("Engine Speed,Catalytic Converter,Lambda Voltage,Result"); break;
      case 46: blockDescs=F("Engine Speed,Catalytic Converter"); break;
      case 50: blockDescs=F("Engine Speed,Engine Speed,A/C Readiness,A/C Compressor"); break;
      case 51: blockDescs=F("Engine Speed,Engine Speed,Selected Gear,Battery Voltage"); break;
      case 53: blockDescs=F("Engine Speed,Engine Speed,Battery Voltage,Generator"); break;
      case 54: blockDescs=F("Engine Speed,Load Status,Accel. Pedal Pos.,Throttle Valve Angle"); break;
      case 55: blockDescs=F("Engine Speed,Idle Regulator,Idle Stabilization,Operating"); break;
      case 56: blockDescs=F("Engine Speed,Engine Speed,Idle Regulator,Operating"); break;
      case 60: blockDescs=F("Throttle Valve,Throttle Valve,Throttle Adaptation,Result"); break;
      case 61: blockDescs=F("Engine Speed,Battery Voltage,Throttle Valve Angle,Operating"); break;
      case 62: blockDescs=F("Throttle Valve,Throttle Valve,Accel. Pedal Pos.,Accel. Pedal Pos."); break;
      case 64: blockDescs=F("Lower Adaptation,Lower Adaptation,Emergency Air Gap,Emergency Air Gap"); break;
      case 66: blockDescs=F("Vehicle Speed,Switch Positions I,Vehicle Speed,Switch Positions II"); break;
      case 70: blockDescs=F("Evap. Emissions,Lambda Control,Result"); break;
      case 77: blockDescs=F("Engine Speed,Mass Air Flow,Air Mass from,Result"); break;
      case 81: blockDescs=F("Vehicle Ident.,Immobilizer"); break;
      case 86: blockDescs=F("Readiness Bits,Cycle Flags I,Cycle Flags II,Cycle Flags II"); break;
      case 87: blockDescs=F("Readiness Bits,Error Flags I,Error Flags II,Error Flags II"); break;
      case 90: blockDescs=F("Engine Speed,Camshaft Adjustm.,Camshaft Adjustm."); break;
      case 91: blockDescs=F("Engine Speed,Engine Load,Camshaft Adjustm.,Camshaft"); break;
      case 94: blockDescs=F("Engine Speed,Camshaft Adjustm.,Result"); break;
      case 99: blockDescs=F("Engine Speed,Coolant,Lambda Control,Lambda Control"); break;
      case 100: blockDescs=F("Readiness Bits,Coolant,Time since,OBD-Status"); break;
      case 101: blockDescs=F("Engine Speed,Engine Load,Injection Timing,Mass Air Flow"); break;
      case 102: blockDescs=F("Engine Speed,Coolant,Intake Air,Injection Timing"); break;
      case 107: blockDescs=F("Engine Speed,Lambda Control,Result"); break;
      case 110: blockDescs=F("Engine Speed,Coolant,Injection Timing,Throttle Valve Angle"); break;
      case 111: blockDescs=F("RPM Range 1,RPM Range 2,RPM Range 3,RPM Range 4"); break;
      case 113: blockDescs=F("Engine Speed,Engine Load,Throttle Valve Angle,Athmospheric"); break;
      case 114: blockDescs=F("Engine Load,Engine Load,Engine Load,Wastegate (N75)"); break;
      case 115: blockDescs=F("Engine  speed,Load,BoostDes,BoostAct"); break; //case 115: blockDescs=F("Engine Speed,Engine Load,Boost Pressure,Boost Pressure"); break;
      case 116: blockDescs=F("Engine Speed,Fuel Temp.,Coolant Temp.,Intake Air Temp."); break;
      case 117: blockDescs=F("Engine Speed,Accel. Pedal Pos.,Throttle Valve Angle,Boost Pressure"); break;
      case 118: blockDescs=F("Engine  speed,IAT,WGTE N75,Boost"); break; //case 118: blockDescs=F("Engine Speed,Intake Air,Wastegate (N75),Boost Pressure"); break;
      case 119: blockDescs=F("Engine Speed,Charge Limit,Wastegate (N75),Boost Pressure"); break;
      case 120: blockDescs=F("Engine Speed,Torque specified,Engine Torque,Traction Control"); break;
      case 122: blockDescs=F("Engine Speed,Engine Load,Engine Load,Status"); break;
      case 125: blockDescs=F("Transmission,Brake Electronics,Instrument Cluster,Heating/Air"); break;
      default: blockDescs=F(""); break;
    }
  }
  else{
    Serial.println("Not description found for that address");
  }
  return blockDescs;
}

bool KWP::isConnected() {
  return connected;
}

void KWP::obdWrite(uint8_t data) {
  obd->write(data);
}

uint8_t KWP::obdRead() {
  unsigned long timeout = millis() + 1000;
  while (!obd->available()){
    if (millis() >= timeout) {
      Serial.println(F("ERROR: obdRead timeout"));
      disconnect();
      errorTimeout++;
      return 0;
    }
  }
  uint8_t data = obd->read();
  return data;
}

bool KWP::KWP5BaudInit(uint8_t addr){
  Serial.println(F("---KWP 5 baud init"));
  //delay(3000);
  send5baud(addr);
  return true;
}

void KWP::send5baud(uint8_t data) {
  #define bitcount 10
  byte bits[bitcount];
  byte even=1;
  byte bit;
  for (int i=0; i < bitcount; i++){
    bit=0;
    if (i == 0)  bit = 0;
      else if (i == 8) bit = even; // computes parity bit
      else if (i == 9) bit = 1;
      else {
        bit = (byte) ((data & (1 << (i-1))) != 0);
        even = even ^ bit;
      }
    Serial.print(F("bit"));
    Serial.print(i);
    Serial.print(F("="));
    Serial.print(bit);
    if (i == 0) Serial.print(F(" startbit"));
      else if (i == 8) Serial.print(F(" parity"));
      else if (i == 9) Serial.print(F(" stopbit"));
    Serial.println();
    bits[i]=bit;
  }
  for (int i=0; i < bitcount+1; i++){
    if (i != 0){
      delay(200);
      if (i == bitcount) break;
    }
    if (bits[i] == 1){
      digitalWrite(_OBD_TX_PIN, HIGH);
    } else {
      digitalWrite(_OBD_TX_PIN, LOW);
    }
  }
  obd->flush();
}

bool KWP::KWPSendBlock(char *s, int size) {
  Serial.print(F("---KWPSend sz="));
  Serial.print(size);
  Serial.print(F(" blockCounter="));
  Serial.println(blockCounter);
  Serial.print(F("OUT:"));
  for (int i=0; i < size; i++){
    uint8_t data = s[i];
    Serial.print(data, HEX);
    Serial.print(" ");
  }
  Serial.println();
  for (int i=0; i < size; i++){
    uint8_t data = s[i];
    obdWrite(data);
    if (i < size-1){
      uint8_t complement = obdRead();
      if (complement != (data ^ 0xFF)){
        Serial.println(F("ERROR: invalid complement"));
        disconnect();
        errorData++;
        return false;
      }
    }
  }
  blockCounter++;
  return true;
}

bool KWP::KWPReceiveBlock(char s[], int maxsize, int &size, bool init_delay) {
  bool ackeachbyte = false;
  uint8_t data = 0;
  int recvcount = 0;
  if (size == 0) ackeachbyte = true;
  Serial.print(F("---KWPReceive sz="));
  Serial.print(size);
  Serial.print(F(" blockCounter="));
  Serial.println(blockCounter);
  if (size > maxsize) {
    Serial.println("ERROR: invalid maxsize");
    return false;
  }
  unsigned long timeout = millis() + 2000;  // TODO: This allows connect to different Modules
  //unsigned long timeout = millis() + 1000;
  while ((recvcount == 0) || (recvcount != size)) {
    while (obd->available()){
      data = obdRead();
      s[recvcount] = data;
      recvcount++;
      if ((size == 0) && (recvcount == 1)) {
        size = data + 1;
        if (size > maxsize) {
          Serial.println("ERROR: invalid maxsize");
          return false;
        }
      }
      if ((ackeachbyte) && (recvcount == 2)) {
        if (data != blockCounter){
          Serial.println(F("ERROR: invalid blockCounter"));
          disconnect();
          errorData++;
          return false;
        }
      }
      if ( ((!ackeachbyte) && (recvcount == size)) ||  ((ackeachbyte) && (recvcount < size)) ){
        obdWrite(data ^ 0xFF);
      }
      timeout = millis() + 1000;
    }
    if (millis() >= timeout){
      Serial.println(F("ERROR: timeout"));
      disconnect();
      errorTimeout++;
      return false;
    }
  }
  Serial.print(F("IN: sz="));
  Serial.print(size);
  Serial.print(F(" data="));
  for (int i=0; i < size; i++){
    uint8_t data = s[i];
    Serial.print(data, HEX);
    Serial.print(F(" "));
  }
  Serial.println();
  blockCounter++;
  return true;
}

bool KWP::KWPSendAckBlock() {
  Serial.print(F("---KWPSendAckBlock blockCounter="));
  Serial.println(blockCounter);
  char buf[32];
  sprintf(buf, "\x03%c\x09\x03", blockCounter);
  return (KWPSendBlock(buf, 4));
}

bool KWP::readConnectBlocks() {
  Serial.println(F("------readconnectblocks"));
  String info;
  while (true){
    int size = 0;
    char s[64];
    if (!(KWPReceiveBlock(s, 64, size))) return false;
    if (size == 0) return false;
    if (s[2] == '\x09') break;
    if (s[2] != '\xF6') {
      Serial.println(F("ERROR: unexpected answer"));
      disconnect();
      errorData++;
      return false;
    }
    String text = String(s);
    info += text.substring(3, size-2);
    if (!KWPSendAckBlock()) return false;
  }
  Serial.print("label=");
  Serial.println(info);
  return true;
}
