#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>
#include <IRRemoteESP32.h>  // Keep this for later use
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

// ==== Pin Definitions ====
#define SDA_PIN 21
#define SCL_PIN 22
#define RST_PIN 9
#define SS_PIN 10
#define BUTTON_UP_PIN 12
#define BUTTON_DOWN_PIN 13
#define BUTTON_SELECT_PIN 14
#define SD_CS_PIN 5

// ==== OLED Settings ====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==== RFID Settings ====
MFRC522 mfrc522(SS_PIN, RST_PIN);

// ==== SD Card ====
File dataFile;

// ==== Menu Variables ====
int currentMenu = 0;

// ==== Setup Display ====
void setupDisplay() {
    if (!display.begin(OLED_I2C_ADDR, OLED_RESET)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);  // Halt execution
    }
    display.display();
    delay(2000);
    display.clearDisplay();
}

// ==== Setup RFID ====
void initRFID() {
    SPI.begin();  // Init SPI bus
    mfrc522.PCD_Init();  // Init RFID reader
    Serial.println(F("Scan RFID tag"));
}

// ==== Setup SD Card ====
void initSD() {
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD Card initialization failed!");
        return;
    }
    Serial.println("SD Card is ready.");
}

// ==== Placeholder for WiFi tools ====
void initWiFiTools() {
    Serial.println("Initializing WiFi Tools...");
    // WiFi tools go here later
}

// ==== Display Menu ====
void displayMenu() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    display.println("== Flipper Menu ==");
    display.println(currentMenu == 0 ? "> WiFi Tools"     : "  WiFi Tools");
    display.println(currentMenu == 1 ? "> RFID Scanner"   : "  RFID Scanner");
    display.println(currentMenu == 2 ? "> IR Blaster"     : "  IR Blaster");
    display.println(currentMenu == 3 ? "> SD Logs"        : "  SD Logs");

    display.display();
}

// ==== Setup ====
void setup() {
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);
    pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
    pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);

    setupDisplay();
    initRFID();
    initSD();
    initWiFiTools();
}

// ==== Main Loop ====
void loop() {
    displayMenu();

    // Menu navigation
    if (digitalRead(BUTTON_UP_PIN) == LOW) {
        currentMenu--;
        if (currentMenu < 0) currentMenu = 3;
        delay(300);
    }
    if (digitalRead(BUTTON_DOWN_PIN) == LOW) {
        currentMenu++;
        if (currentMenu > 3) currentMenu = 0;
        delay(300);
    }
    if (digitalRead(BUTTON_SELECT_PIN) == LOW) {
        Serial.print("Selected menu: ");
        Serial.println(currentMenu);
        if (currentMenu == 1) {
            // RFID Scanner Mode
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.println("Scanning for RFID...");
            display.display();

            // Look for new RFID cards
            if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
                Serial.println("No card detected.");
                delay(1000);
                return;
            }

            // Get UID
            String uid = "";
            for (byte i = 0; i < mfrc522.uid.size; i++) {
                uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
                uid += String(mfrc522.uid.uidByte[i], HEX);
            }
            uid.toUpperCase();

            Serial.print("RFID UID: ");
            Serial.println(uid);

            // Show on OLED
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("RFID TAG Found!");
            display.println("UID: ");
            display.println(uid);
            display.display();

            // Optional: Save to SD Card
            dataFile = SD.open("/rfid_log.txt", FILE_APPEND);
            if (dataFile) {
                dataFile.println(uid);
                dataFile.close();
                Serial.println("Saved to SD");
            } else {
                Serial.println("Failed to open log file.");
            }

            delay(2000);  // Delay to avoid reading the same card again
        }

        delay(300);
    }

    delay(100);
}
