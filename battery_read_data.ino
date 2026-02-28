#include <Wire.h>
#include <Adafruit_INA3221.h>

Adafruit_INA3221 inaA;
Adafruit_INA3221 inaB;

// Voltage thresholds (LiPo)
const float CELL_FULL    = 4.20;
const float CELL_EMPTY   = 3.30;
const float CELL_WARNING = 3.50;
const float TAP_PRESENT_THRESHOLD = 2.0;

float cell[6];
int detectedCells = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (!inaA.begin(0x41)) {
    Serial.println("INA3221 @0x41 not found");
    while (1);
  }

  if (!inaB.begin(0x40)) {
    Serial.println("INA3221 @0x40 not found");
    while (1);
  }

  Serial.println("Multi-Cell Battery Monitor Started");
}

void loop() {

  //----------------------------------
  // Read and calculate per-cell voltages
  //----------------------------------

  float ch1 = inaA.getBusVoltage(1);  // reads ~8V (two cells combined)
  cell[0] = ch1 / 2.0;                // Cell 1 estimated
  cell[1] = ch1 / 2.0;                // Cell 2 estimated
  cell[2] = inaA.getBusVoltage(2) - ch1;           // Cell 3
  cell[3] = inaB.getBusVoltage(0) - inaA.getBusVoltage(2);  // Cell 4
  cell[4] = inaB.getBusVoltage(1) - inaB.getBusVoltage(0);  // Cell 5
  cell[5] = inaB.getBusVoltage(2) - inaB.getBusVoltage(1);  // Cell 6

  //----------------------------------
  // Detect how many cells exist
  //----------------------------------

  detectedCells = 0;

  for (int i = 0; i < 6; i++) {
    if (cell[i] > TAP_PRESENT_THRESHOLD) {
      detectedCells++;
    }
  }

  if (detectedCells < 1) {
    Serial.println("No battery detected");
    delay(1000);
    return;
  }

  //----------------------------------
  // Total voltage
  //----------------------------------

  float totalVoltage = 0.0;

  for (int i = 0; i < 6; i++) {
    if (cell[i] > TAP_PRESENT_THRESHOLD) {
      totalVoltage += cell[i];
    }
  }

  //----------------------------------
  // Average cell voltage
  //----------------------------------

  float avgCellVoltage = totalVoltage / detectedCells;

  //----------------------------------
  // Convert voltage → percentage
  //----------------------------------

  float percentFloat =
    (avgCellVoltage - CELL_EMPTY) /
    (CELL_FULL - CELL_EMPTY) * 100.0;

  if (percentFloat > 100) percentFloat = 100;
  if (percentFloat < 1)   percentFloat = 1;

  int batteryPercent = (int)percentFloat;

  //----------------------------------
  // Low-cell warning detection
  //----------------------------------

  bool lowCellDetected = false;

  for (int i = 0; i < 6; i++) {
    if (cell[i] > TAP_PRESENT_THRESHOLD && cell[i] < CELL_WARNING) {
      lowCellDetected = true;
    }
  }

  //----------------------------------
  // Output results
  //----------------------------------

  Serial.print("Detected Cells: ");
  Serial.print(detectedCells);
  Serial.print("   ");

  Serial.print("Battery %: ");
  Serial.print(batteryPercent);
  Serial.print("   ");

  Serial.print("Total Voltage: ");
  Serial.print(totalVoltage, 3);
  Serial.print("V   ");

  if (lowCellDetected) {
    Serial.print("WARNING: LOW CELL!");
  }

  Serial.println();

  //----------------------------------
  // Debug
  //----------------------------------

  Serial.print("Cells: ");
  for (int i = 0; i < 6; i++) {
    if (cell[i] > TAP_PRESENT_THRESHOLD) {
      Serial.print(cell[i], 3);
      Serial.print("  ");
    }
  }
  Serial.println();

  delay(500);
}