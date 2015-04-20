/*
  SNAKECHARMER R002
*/

#include <Wire.h> // Arduino I2C library
#include "tsi.h"  // Touch Sense Interface

// GALICIAN BAGPIPE FINGERING (ALPHA) - according to OpenPIPE :)
struct {
  word fpos; 
  word mask;
  byte note;
} table[] = {
/*
    FINGER   = finger position (1: on, 0: off)
    RELEVANT = ignore (1: relevant, 0: ignore)
    PWM      = pwm rate

    FINGER          IGNORE         PWM
*/
  { 0b000110111111, 0b000111111111, 249 },
  { 0b000110011111, 0b000111111111, 235 },
  { 0b000110101111, 0b000111111111, 222 },
  { 0b000110001111, 0b000111111111, 209 },
  { 0b000110010111, 0b000111011111, 198 },
  { 0b000110000111, 0b000111001111, 186 },
  { 0b000110000110, 0b000111000111, 176 },
  { 0b000110000101, 0b000111000111, 166 },
  { 0b000110000100, 0b000111000110, 157 },
  { 0b000110000010, 0b000111000110, 148 },
  { 0b000110000000, 0b000111000110, 140 },
  { 0b000010000100, 0b000111000110, 132 },
  { 0b000010000000, 0b000111000110, 124 },
  { 0b000100111111, 0b000111111111, 124 },
  { 0b000000000000, 0b000111000110, 117 },
  { 0b000100011111, 0b000111111111, 117 },
  { 0b000010011111, 0b000111111111, 117 },
  { 0b000100001111, 0b000111111111, 105 },
  { 0b000100000111, 0b000111111111,  93 }
};

byte scanTable(word fingers) {
  byte n;
  // Loop through table
  byte tableCount = sizeof(table) / sizeof(table[0]);
  for(n = 0; n < tableCount; n++) {
    // Check for match
    if((fingers & table[n].mask) == table[n].fpos) {
      // Match found, return note
      return table[n].note;
    }
  }
  return 0; // None found
}

void setup() {
  Serial.begin(57600);
  Serial.println("SnakeCharmer R002 breakout example sketch");

  // Configure PWM timer (warning, low-level code be here)
  pinMode(11, OUTPUT);
  TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS22);
  OCR2B = 10;
  
  // Configure the TSI
  Wire.begin();
  Serial.print("Configuring... ");
  tsiConfig();
  Serial.println("done!");
}

void loop() {
  byte pwm;
  word fingers;
  static word fingersPrev = 0xFFFF;
  static byte pwmPrev = 0;
  byte buffer[18];
  
  // Read all registers from TSI (though in this case, we only use the first two)
  tsiReadAll(buffer);
  
  // Construct word from two bytes (big-endian word)
  fingers = (buffer[1] << 8) | buffer[0];
  
  // Give info on serial port
  if(fingers != fingersPrev) {
    Serial.print("Fingers: ");
    Serial.println(fingers | 0x8000, BIN); // 0x8000 only to force nice alignment
    fingersPrev = fingers; // To detect a change
  }

  // Do fingers to note conversion
  if(fingers & 0b000000000100) {
    // Thumb is in position: table lookup to decode finger position
    pwm = scanTable(fingers);
  } else {
    // Thumb not in position - no sound
    pwm = 0;
  }

  // Audio output
  if(pwm != pwmPrev) {
    // Update PWM register with new note
    Serial.print("PWM: ");
    Serial.println(pwm);
    OCR2A = pwm;
    pwmPrev = pwm; // To detect a change
  }
}

void tsiReadAll(byte buffer[]) {
  Wire.beginTransmission(0x5A); // Begin communication with the MPR121 (address 0x5A)
  Wire.write(0);                // Send address to be read
  Wire.requestFrom(0x5A, 18);   // Request a read out from the MPR121 (18 bytes)
  for(byte i = 0; Wire.available() && i < 18; i++) {
    buffer[i] = Wire.read();    // Receive data
  }
  Wire.endTransmission();       // End communication
}

// Read TSI
char tsiRead(unsigned char address) {
  char data;
  Wire.beginTransmission(0x5A);  // Begin communication with the MPR121 (address 0x5A)
  Wire.write(address);           // Send address to be read
  Wire.requestFrom(0x5A, 1);     // Request a read out from the MPR121
  data = Wire.read();            // Receive data
  Wire.endTransmission();        // End communication
  return data;                   // Return the received data
}

// Write TSI
void tsiWrite(unsigned char address, unsigned char data) {
  Wire.beginTransmission(0x5A);  // Begin communication with the MPR121 (address 0x5A)
  Wire.write(address);           // Send address to be written
  Wire.write(data);              // Send data to be written
  Wire.endTransmission();        // End communication
}

// Configure TSI
void tsiConfig(void) {
  tsiWrite(MHD_R, 0x01);         // Filtering
  tsiWrite(NHD_R, 0x01);
  tsiWrite(NCL_R, 0x00);
  tsiWrite(FDL_R, 0x00);
  tsiWrite(MHD_F, 0x01);
  tsiWrite(NHD_F, 0x01);
  tsiWrite(NCL_F, 0xFF);
  tsiWrite(FDL_F, 0x02);
  tsiWrite(ELE0_T, TOU_THRESH);  // Thresholds (see tsi.h)
  tsiWrite(ELE0_R, REL_THRESH);
  tsiWrite(ELE1_T, TOU_THRESH);
  tsiWrite(ELE1_R, REL_THRESH);
  tsiWrite(ELE2_T, TOU_THRESH);
  tsiWrite(ELE2_R, REL_THRESH);
  tsiWrite(ELE3_T, TOU_THRESH);
  tsiWrite(ELE3_R, REL_THRESH);
  tsiWrite(ELE4_T, TOU_THRESH);
  tsiWrite(ELE4_R, REL_THRESH);
  tsiWrite(ELE5_T, TOU_THRESH);
  tsiWrite(ELE5_R, REL_THRESH);
  tsiWrite(ELE6_T, TOU_THRESH);
  tsiWrite(ELE6_R, REL_THRESH);
  tsiWrite(ELE7_T, TOU_THRESH);
  tsiWrite(ELE7_R, REL_THRESH);
  tsiWrite(ELE8_T, TOU_THRESH);
  tsiWrite(ELE8_R, REL_THRESH);
  tsiWrite(ELE9_T, TOU_THRESH);
  tsiWrite(ELE9_R, REL_THRESH);
  tsiWrite(ELE10_T, TOU_THRESH);
  tsiWrite(ELE10_R, REL_THRESH);
  tsiWrite(ELE11_T, TOU_THRESH);
  tsiWrite(ELE11_R, REL_THRESH);
  tsiWrite(FIL_CFG, 0x00);
  tsiWrite(ELE_CFG, 0x0C);        // Enable all electrodes
  tsiWrite(ATO_CFG0, 0x0B);
  tsiWrite(ATO_CFGU, 0xC9);       // USL = (VDD - 0.7) / VDD * 256 = 0xC9 (for 3.3V)
  tsiWrite(ATO_CFGL, 0x82);       // LSL = 0.65 * USL = 0x82              (for 3.3V)
  tsiWrite(ATO_CFGT, 0xB5);       // TGT = 0.90 * USL = 0xB5              (for 3.3V)
}
