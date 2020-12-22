

// SimpleRx - the slave or the receiver

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(6, 2, 7, 4, 3, 8);

#define CE_PIN   9
#define CSN_PIN 10

const byte thisSlaveAddress[5] = {'R','x','A','A','A'};

RF24 radio(CE_PIN, CSN_PIN);

char dataReceived[50]; // this must match dataToSend in the TX
bool newData = false;

float cellVoltages[3];
float avgVoltage;


bool firstContact = false;
bool firstContact2 = false;
unsigned long lastPing = 0;
unsigned long timeSinceLastPing = 0;
unsigned long lastPrint = 0;
unsigned long lastBattWarn = 0;
bool warningPrinted = false;

unsigned int numCells = 0;
bool cellsChecked = false;

void initRadio() {
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.openReadingPipe(1, thisSlaveAddress);
  radio.startListening();
}

void setup() {

  lcd.begin(20, 4);
  lcd.clear();
  lcd.print("init radio...");

  Serial.begin(9600);

  Serial.println("SimpleRx Starting");
  initRadio();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Radio up!");
  delay(1000);
  lcd.clear();
}

void loop() {
    getData();
    showData();

    if (!firstContact && lastPrint > 1000) {
      lcd.setCursor(0,0);
      lcd.print("Waiting for remote..");
      lcd.setCursor(0,1);
      lcd.print(millis()/1000);
      lastPrint = 0;
    } else if (firstContact) {
      if (!firstContact2) {
        lcd.clear();
        firstContact2 = true;
      }
      printLinkStatus();
      lcd.setCursor(0,0);
      lcd.print("Cells: ");
      lcd.print(numCells);
      printCellStatus();
    }
    
    lastPrint = millis();
}

void printLinkStatus() {
  timeSinceLastPing = millis() - lastPing;

  if (timeSinceLastPing < 2000) {
    lcd.setCursor(12,0);
    lcd.print("Link UP!");
    lcd.setCursor(17,1);
    lcd.print("   ");
  } else {
    lcd.setCursor(12,0);
    lcd.print("Link DN!");
    lcd.setCursor(17,1);
    lcd.print(timeSinceLastPing/1000);
    delay(200);
  }
}

void calculateAverageVoltage() {
  if (numCells == 3) {
    avgVoltage = (cellVoltages[0] + cellVoltages[1] + cellVoltages[2])/3;
  } else {
    avgVoltage = (cellVoltages[0] + cellVoltages[1])/2;
  }
}

void printCellStatus() {
  lcd.setCursor(0,2);
  lcd.print(" C1 | C2 | C3 | AVG");
  lcd.setCursor(0,3);
  lcd.print(cellVoltages[0]);
  lcd.print("|");
  lcd.print(cellVoltages[1]);
  lcd.print("|");
  if (numCells == 3) {
    lcd.print(cellVoltages[2]);
  } else {
    lcd.print("----");
  }
  lcd.print("|");
  lcd.print(avgVoltage);
  lcd.print("|");

  lcd.setCursor(0,1);
  lcd.print("Tot: ");
  lcd.print(cellVoltages[0] + cellVoltages[1] + cellVoltages[2]);
  lcd.print("V");
}

//==============

void getData() {
    if ( radio.available() ) {
        radio.read( &dataReceived, sizeof(dataReceived) );
        newData = true;

        char* delim;

        delim = strtok (dataReceived,":");
        int i = 0;
        while (delim != NULL) {
          cellVoltages[i] = atof(delim);
          delim = strtok (NULL, ":");
          i++;
        }

        if (!cellsChecked) {
          if (cellVoltages[2] <= 0.0) {
            Serial.println("NUM CELLS 2");
            numCells = 2;
          } else {
            numCells = 3;
          }
          cellsChecked = true;
        }
        calculateAverageVoltage();
        lastPing = millis();
        firstContact = true;
    }
}

void showData() {
    if (newData == true) {
        Serial.print("Data received ");
        Serial.println(dataReceived);
        Serial.print(cellVoltages[0]);
        Serial.print(" V, ");
        Serial.print(cellVoltages[1]);
        Serial.print(" V, ");
        Serial.print(cellVoltages[2]);
        Serial.println(" V, ");
        newData = false;
    }
}
