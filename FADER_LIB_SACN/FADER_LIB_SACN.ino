#include <ResponsiveAnalogRead.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
EthernetUDP Udp;


// SACN OUTPUT SETTINGS
int SACN_ADDRESSES[] = {1, 2, 3, 4, 5, 6, 7, 8};
byte UNIVERSE = 2;


// NETWORK SETTINGS
IPAddress SELF_IP(192, 168, 1, 130);
#define SELF_PORT 58300

IPAddress DESTINATION_IP(239, 255, 0, UNIVERSE);
#define DESTINATION_PORT 5568

byte MAC_ADDRESS[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };


// FADER TRIM SETTINGS
#define TOP 960
#define BOT 70
int faderTrimTop[8] = {TOP, TOP, TOP, TOP, TOP, TOP, TOP, TOP}; // ADJUST THIS IF A SINGLE FADER ISN'T READING 255 AT THE TOP OF ITS TRAVEL
int faderTrimBottom[8] = {BOT, BOT, BOT, BOT, BOT, BOT, BOT, BOT}; // ADJUST THIS IF A SINGLE FADER ISN'T READING 0 AT THE BOTTOM OF ITS TRAVEL

#define TOUCH_THRESHOLD 30




char sequence = 0;
char RootLayer[] = {0x00, 0x10, 0x00, 0x00, 0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00, 0x72, 0x6e, 0x00, 0x00, 0x00, 0x04, 0x48, 0x2d, 0xe2, 0xa9, 0xae, 0x2b, 0x47, 0xf2, 0x9d, 0xb7, 0x2a, 0x00, 0x64, 0xfa, 0x48, 0x32};
char FramingLayer[] = {0x72, 0x58, 0x00, 0x00, 0x00, 0x02, 0x46, 0x41, 0x44, 0x45, 0x52, 0x5F, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x0b, 0x00, 0x00, UNIVERSE};
char DMPLayer[] = {0x72, 0x0b, 0x02, 0xa1, 0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x00};
char Slots[512];
static byte MOTOR_PINS_A[8] = {0, 2, 4, 6, 8, 10, 24, 28};
static byte MOTOR_PINS_B[8] = {1, 3, 5, 7, 9, 12, 25, 29};
ResponsiveAnalogRead faders[8] = {
  ResponsiveAnalogRead(A9, true),
  ResponsiveAnalogRead(A8, true),
  ResponsiveAnalogRead(A7, true),
  ResponsiveAnalogRead(A6, true),
  ResponsiveAnalogRead(A5, true),
  ResponsiveAnalogRead(A4, true),
  ResponsiveAnalogRead(A3, true),
  ResponsiveAnalogRead(A2, true)
};


void setup() {
  Serial.begin(9600);

  for (byte i = 0; i < 8; i++) {
    pinMode(MOTOR_PINS_A[i], OUTPUT);
    pinMode(MOTOR_PINS_B[i], OUTPUT);
    digitalWrite(MOTOR_PINS_A[i], LOW);
    digitalWrite(MOTOR_PINS_B[i], LOW);
    analogWriteFrequency(MOTOR_PINS_A[i], 18000);
    analogWriteFrequency(MOTOR_PINS_B[i], 18000);
    faders[i].setActivityThreshold(TOUCH_THRESHOLD);
  }
  
  Ethernet.begin(MAC_ADDRESS, SELF_IP);
  if (Ethernet.linkStatus() == LinkON) {
    Udp.begin(SELF_PORT);
  } else {
    Serial.println("Ethernet is not connected.");
  }

}

void loop() {
  FramingLayer[73] = sequence;

  for (byte i = 0; i < 8; i++) {
    faders[i].update();
    Slots[SACN_ADDRESSES[i]-1] = getFaderValue(i);
  }

  if (Ethernet.linkStatus() == LinkON) {
    Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
    Udp.write(RootLayer, 38);
    Udp.write(FramingLayer, 77);
    Udp.write(DMPLayer, 11);
    Udp.write(Slots, 512);
    Udp.endPacket();
  }
  sequence++;
  delay(50);
}
int getFaderValue(byte fader) {
  return max(0, min(255, map(faders[fader].getValue(), faderTrimBottom[fader], faderTrimTop[fader], 0, 255)));
}
