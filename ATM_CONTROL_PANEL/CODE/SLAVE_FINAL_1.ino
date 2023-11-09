#include <CAN.h>
#include <Arduino.h>
#include <Keypad.h>
#include <Adafruit_LiquidCrystal.h>
#define RECEIVER_ID 0x101
const int MAX_PACKET_SIZE = 101; // Maximum expected packet size
Adafruit_LiquidCrystal lcd(A0, A1, 5, 4, 3, 2);
char receivedPassword[MAX_PACKET_SIZE]; // Buffer to store the received password

// Define the keypad layout and pins
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; // Connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String inputPassword; // Variable to store the entered password

void setup() {
  lcd.begin(16, 2);
  lcd.print("Enter Your Pin");
  Serial.begin(9600);
  while (!Serial);
  Serial.println("CAN Receiver");

  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
  CAN.filter(RECEIVER_ID, 101); // Set filter to only receive packets with RECEIVER_ID
}

void loop() {
  // Check if there is any CAN message received
  if (CAN.parsePacket()) {
    // Limit buffer size to avoid overflows
    int bufferSize = min(CAN.packetDlc(), sizeof(receivedPassword) - 1);
    int bytesRead = CAN.readBytes(receivedPassword, bufferSize);
    receivedPassword[bytesRead] = '\0';

    // Process the received password (optional)
    // Here, you can add any custom processing logic for the received password.

    // For demonstration purposes, let's print the received password.
    //Serial.print("Received Password: ");
    //Serial.println(receivedPassword);
  }

  if (keypad.getKeys()) {
    for (int i = 0; i < LIST_MAX; i++) {
      if (keypad.key[i].stateChanged && keypad.key[i].kstate == PRESSED) {
        char keyChar = keypad.key[i].kchar;
        Serial.print("Entered Character: ");
        Serial.println(keyChar);

        if (keyChar == '*') {
          // Clear the entered password when '*' is pressed
          inputPassword = ""; // Clear the entered password
          lcd.clear();
          lcd.print("Enter Your Pin");
          lcd.print(getAsterisks(inputPassword.length()));
        } else if (keyChar == '#') {
          // Check if the entered password matches the predefined password when '#' is pressed
          if (inputPassword.equals(receivedPassword)) {
            lcd.clear();
            lcd.print("Access granted!");
            delay(3000);
            lcd.clear();
            lcd.print("Thank You");
            delay(2000);
            lcd.clear();
            lcd.print("Enter Your Pin");
            // Convert the String to a uint8_t buffer and send it as a reply (Access Granted)
            size_t passwordLength = inputPassword.length();
            const uint8_t* passwordBuffer = reinterpret_cast<const uint8_t*>(inputPassword.c_str());
            CAN.beginPacket(RECEIVER_ID);
            CAN.write(passwordBuffer, passwordLength);
            CAN.endPacket();
            Serial.println("Access Granted");
          } else {
            lcd.clear();
            lcd.print("Access denied!");
            delay(3000);
            lcd.clear();
            lcd.print("Try Again!");
            delay(2000);
            lcd.clear();
            lcd.print("Enter Your Pin");
            // Send the "Access Denied" message as a reply
            const char* deniedMessage = "DENIED";
            CAN.beginPacket(RECEIVER_ID);
            CAN.write(reinterpret_cast<const uint8_t*>(deniedMessage), strlen(deniedMessage));
            CAN.endPacket();
            Serial.println("Access Denied");
          }
          // Clear the entered password after checking
          inputPassword = "";
          // Clear the asterisks from the second line
          lcd.setCursor(0, 1);
          lcd.print(getAsterisks(inputPassword.length()));
        } else {
          // Append other characters to the entered password
          inputPassword += keyChar;
          // Display asterisk for each entered character on the second line
          lcd.setCursor(inputPassword.length() - 1, 1);
          lcd.print("*");
          delay(1000);
        }
      }
    }
  }
}

String getAsterisks(int length) {
  String asterisks = "";
  for (int i = 0; i < length; i++) {
    asterisks += "*";
  }
  return asterisks;
}