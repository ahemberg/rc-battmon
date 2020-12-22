
// SimpleTx - the master or the transmitter

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN   9
#define CSN_PIN 10

#define CELL_ONE_SCALING 1.47
#define CELL_TWO_SCALING 1.60
#define CELL_THREE_SCALING 1.8526

const byte slaveAddress[5] = {'R','x','A','A','A'};


RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

char dataToSend[50];

unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 1000; // send once per second

unsigned int adc_0_value;
unsigned int adc_1_value;
unsigned int adc_2_value;

float cell_one_voltage;
float cell_two_voltage;
float cell_three_voltage;

void setup() {

    pinMode(A5, INPUT);
    pinMode(A4, INPUT);
    pinMode(A3, INPUT);

    Serial.begin(9600);

    Serial.println("SimpleTx Starting");

    radio.begin();
    radio.setDataRate( RF24_250KBPS );
    radio.setPALevel(RF24_PA_MAX);
    radio.setRetries(3,5); // delay, count
    radio.openWritingPipe(slaveAddress);
}

//====================


void loop() {

    adc_0_value = analogRead(A5);
    adc_1_value = analogRead(A4);
    adc_2_value = analogRead(A3);
    
    cell_one_voltage = adc_0_value * (3.3/1023.0) * CELL_ONE_SCALING;
    cell_two_voltage = adc_1_value * (3.3/1023.0) * CELL_TWO_SCALING;
    cell_three_voltage = adc_2_value * (3.3/1023.0) * CELL_THREE_SCALING;
  
    currentMillis = millis();
    if (currentMillis - prevMillis >= txIntervalMillis) {
        send();
        prevMillis = millis();
    }
}

//====================

void send() {

    bool rslt;
    rslt = radio.write( &dataToSend, sizeof(dataToSend) );
        // Always use sizeof() as it gives the size as the number of bytes.
        // For example if dataToSend was an int sizeof() would correctly return 2

    Serial.print("Data Sent ");
    Serial.print(dataToSend);
    if (rslt) {
        Serial.println("  Acknowledge received");
        updateMessage();
    }
    else {
        Serial.println("  Tx failed");
    }
}

//================

void updateMessage() {

  char buf_one[6];
  char buf_two[6];
  char buf_three[6];

  dtostrf(cell_one_voltage, 4, 2, buf_one);
  dtostrf(cell_two_voltage, 4, 2, buf_two);
  dtostrf(cell_three_voltage, 4, 2, buf_three);
  
  sprintf (dataToSend, "%s:%s:%s", buf_one, buf_two, buf_three);
}
