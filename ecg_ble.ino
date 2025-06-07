/*
 * ecg_ble.ino – Stream ADS1292R ECG (Lead II) + impedance via BLE UART
 * Hardware: Arduino Pro Micro 3.3 V/8 MHz + ADS1292R breakout + HM‑10
 * Author: ChatGPT 2025‑06‑07
 * License: MIT
 */

#include <SPI.h>

// ---------- Pin definitions ----------
constexpr uint8_t PIN_CS    = 10;
constexpr uint8_t PIN_DRDY  = 7;
constexpr uint8_t PIN_START = 8;
constexpr uint8_t PIN_RESET = 9;
constexpr uint8_t PIN_HM_KEY = 6; // optional (AT mode control)

// ---------- ADS1292R command macros ----------
#define CMD_WAKEUP   0x02
#define CMD_SDATAC   0x11
#define CMD_RDATAC   0x10
#define CMD_RDATA    0x12
#define CMD_WREG     0x40

// ---------- Helper: write single register ----------
void adsWriteReg(uint8_t reg, uint8_t val) {
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(CMD_WREG | reg);
  SPI.transfer(0x00);      // single register
  SPI.transfer(val);
  digitalWrite(PIN_CS, HIGH);
}

void adsCommand(uint8_t cmd) {
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(cmd);
  digitalWrite(PIN_CS, HIGH);
}

void setup() {
  // --- GPIO ---
  pinMode(PIN_CS,    OUTPUT);
  pinMode(PIN_START, OUTPUT);
  pinMode(PIN_RESET, OUTPUT);
  pinMode(PIN_DRDY,  INPUT_PULLUP);
  pinMode(PIN_HM_KEY, OUTPUT);
  digitalWrite(PIN_CS, HIGH);
  digitalWrite(PIN_HM_KEY, LOW); // normal mode

  // --- Serial ports ---
  Serial.begin(115200);   // USB debug
  Serial1.begin(9600);    // HM‑10 default baud

  // --- SPI bus ---
  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));

  // --- Reset ADS1292R ---
  digitalWrite(PIN_RESET, HIGH); delay(1);
  digitalWrite(PIN_RESET, LOW);  delay(5);
  digitalWrite(PIN_RESET, HIGH); delay(50);

  adsCommand(CMD_SDATAC); // stop read‑datacontinuous
  delay(5);

  // CONFIG1: internal osc, 500 SPS, HR = 0
  adsWriteReg(0x01, 0b10010110);
  // CONFIG2: enable PDB_LOFF_COMP, test signals off
  adsWriteReg(0x02, 0b11000000);
  // LOFF: lead‑off defaults
  adsWriteReg(0x03, 0x10);
  // CH1SET: gain 6 ×, ECG normal electrode on, power on
  adsWriteReg(0x04, 0b00010000);
  // CH2SET: gain 6 ×, used for impedance, so normal electrode off
  adsWriteReg(0x05, 0b00010000);
  // RLD_SENS / RLD_SENSP/N: defaults enable RL drive

  adsCommand(CMD_RDATAC); // start continuous read
  digitalWrite(PIN_START, HIGH);

  Serial.println(F("ADS1292R initialised – streaming to BLE"));
}

void loop() {
  if (digitalRead(PIN_DRDY) == LOW) {
    // each sample frame = 9 bytes: status(3) + CH1(3) + CH2(3)
    uint8_t buf[9];
    digitalWrite(PIN_CS, LOW);
    for (uint8_t i = 0; i < 9; ++i) buf[i] = SPI.transfer(0x00);
    digitalWrite(PIN_CS, HIGH);

    // combine 24‑bit signed values
    int32_t ecgRaw = ((int32_t)buf[3] << 16) | (buf[4] << 8) | buf[5];
    if (ecgRaw & 0x800000) ecgRaw |= 0xFF000000; // sign‑extend
    int32_t impRaw = ((int32_t)buf[6] << 16) | (buf[7] << 8) | buf[8];
    if (impRaw & 0x800000) impRaw |= 0xFF000000;

    // convert to mV assuming ±4.5 mV full‑scale at gain 6
    const float LSB = 4.5f / 8388607.0f; // mV per bit
    float ecg_mV = ecgRaw * LSB;
    float imp_uV = impRaw * LSB * 1000.0f; // microvolts for impedance

    // format CSV "mV,µV\n"
    char out[32];
    dtostrf(ecg_mV, 1, 4, out);
    Serial1.print(out);   Serial1.print(',');
    dtostrf(imp_uV, 1, 2, out);
    Serial1.println(out);

    // mirror to USB for debugging
    Serial.println(out);
  }
}

// AT+NAMEECG_STREAMER     # Set device name AT+BAUD4               # 9600 baud AT+RESET
