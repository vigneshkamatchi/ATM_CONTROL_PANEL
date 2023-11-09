#include <CAN.h>
#include <Arduino.h>

#define SLAVE1_ID 0x101
#define SLAVE2_ID 0x102

const int MAX_PACKET_SIZE = 101; // Maximum expected packet size

// Define passwords for the slaves
const char* SLAVE1_PASSWORD = "4567";
const char* SLAVE2_PASSWORD = "1234";

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("CAN Master");

  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
}

// Function to send a password message to the specified slave
void sendPasswordMessage(uint16_t slaveID, const char* password) {
  // Serial.print("Sending message to slave: ");
  // Serial.print(slaveID, HEX);
  // Serial.print(" ... ");

  CAN.beginPacket(slaveID);
  CAN.print(password); // Sending the password as a message
  CAN.endPacket();

  // Serial.println("done");
  //delay(1000);
}

void loop() {
  char receivedMessage[MAX_PACKET_SIZE]; // Buffer to store received message

  // Send password to Slave 1 and handle reply
  sendPasswordMessage(SLAVE1_ID, SLAVE1_PASSWORD);
  if (CAN.parsePacket()) {
    int packetSize = CAN.readBytes(receivedMessage, MAX_PACKET_SIZE);
    receivedMessage[packetSize] = '\0';
    if (strcmp(receivedMessage, "4567") == 0) {
      Serial.println("Access Granted for Slave 1");
    } else {
      Serial.println("Access Denied for Slave 1");
    }
  }

  //delay(500);

  //Send password to Slave 2 and handle reply
  sendPasswordMessage(SLAVE2_ID, SLAVE2_PASSWORD);
  if (CAN.parsePacket()) {
    int packetSize = CAN.readBytes(receivedMessage, MAX_PACKET_SIZE);
    receivedMessage[packetSize] = '\0';
    if (strcmp(receivedMessage, "1234") == 0) {
      Serial.println("Access Granted for Slave 2");
    } else {
      Serial.println("Access Denied for Slave 2");
    }
  }

  //delay(500);
}