
#include <Renard.h>
#include <SoftwareSerial.h>

#define IR_W 0xF7E01F

//Blue
#define RX 5

//Orange
#define TX 6

SoftwareSerial softSerial(RX, TX);

#define IR_MARK LOW
#define IR_SPACE HIGH

#define OUTPUT_PIN 2
#define R_DIM 9
#define G_DIM 10
#define B_DIM 11

#define USE_RENARD False

const uint8_t PROGMEM gamma[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
  10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
  17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
  25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
  37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
  51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
  69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
  90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
  115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
  144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
  177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
  215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};

#if USE_RENARD
Renard renard(Serial, 3);
#endif

void sendHeader(int pin)
{
  digitalWrite(pin, IR_SPACE);
  delayMicroseconds(20000);
  digitalWrite(pin, IR_MARK);
  delayMicroseconds(9530);
  digitalWrite(pin, IR_SPACE);
  delayMicroseconds(4610);
}

void sendFooter(int pin)
{
  digitalWrite(pin, IR_MARK);
  delayMicroseconds(545);
  digitalWrite(pin, IR_SPACE);
  delayMicroseconds(10000);
}

void sendBit(int pin, int value)
{
  if (value == 0) {
    digitalWrite(pin, IR_MARK);
    delayMicroseconds(545);
    digitalWrite(pin, IR_SPACE);
    delayMicroseconds(625);
  }
  else {
    digitalWrite(pin, IR_MARK);
    delayMicroseconds(545);
    digitalWrite(pin, IR_SPACE);
    delayMicroseconds(1725);
  }
}


void sendNECData(int pin, long value)
{
  sendHeader(pin);

  for (int i = 31; i >= 0; --i) {
    int bitValue = ((value & (1L << i)) != 0);
    //Serial.println(bitValue);
    sendBit(pin, bitValue);
  }

  sendFooter(pin);
}

void writeGammaCorrectedAnalog(int pin, int value)
{
  int gammaCorrected = pgm_read_byte(&gamma[value]);
  analogWrite(pin, gammaCorrected);
}

void setup() {
  
  
#if USE_RENARD
  Serial.begin(57600);
  Serial.println(F("Using Renard protocol"));
  Serial.println(F("Ready for action!"));
#else
  softSerial.begin(9600);
#endif



  pinMode(OUTPUT_PIN, OUTPUT);
  digitalWrite(OUTPUT_PIN, HIGH);

  pinMode(R_DIM, OUTPUT);
  pinMode(G_DIM, OUTPUT);
  pinMode(B_DIM, OUTPUT);

  delay(300);
  sendNECData(OUTPUT_PIN, IR_W); //Change the flood light to output all white. This will basically use the controller as a power supply.
  
  delay(100);
  writeGammaCorrectedAnalog(G_DIM, 255);
}

void loop() {

#if USE_RENARD

  while (!renard.processInput())
    ;

  writeGammaCorrectedAnalog(R_DIM, renard.channelValue(1));
  writeGammaCorrectedAnalog(G_DIM, renard.channelValue(2));
  writeGammaCorrectedAnalog(B_DIM, renard.channelValue(3));
#else

  while (softSerial.available() > 0) {
    // look for the next valid integer in the incoming serial stream:
    int red = softSerial.parseInt();
    // do it again:
    int green = softSerial.parseInt();
    // do it again:
    int blue = softSerial.parseInt();

    // look for the newline. That's the end of your sentence:
    if (softSerial.read() == '\n') {
      writeGammaCorrectedAnalog(R_DIM, red);
      writeGammaCorrectedAnalog(G_DIM, green);
      writeGammaCorrectedAnalog(B_DIM, blue);
    }
  }

#endif

}
