// WARNING!
// THIS SCRIPT IMMEDIATELY ERASES ANY CONNECTED SPI FLASH ROMS AFTER STARTUP!
// WARNING!

// Wiring:
// ESP32 CS0  (GPIO5)  - Flash /CS   (Pin 1)
// ESP32 MISO (GPIO19) - Flash DO    (Pin 2)
// ESP32 WP   (GPIO22) - Flash /WP   (Pin 3)
// ESP32 GND           - Flash GND   (Pin 4)
// ESP32 MOSI (GPIO23) - Flash DI    (Pin 5)
// ESP32 CLK  (GPIO18) - Flash CLK   (Pin 6)
// ESP32 HD   (GPIO21) - Flash /HOLD (Pin 7)
// ESP32 3V3           - Flash VCC   (Pin 8)

#include <Arduino.h>
#include <SPIMemory.h>

#define BLOCK_SIZE 4096

SPIFlash flash;

bool getID() {
    uint32_t JEDEC = flash.getJEDECID();
    if (!JEDEC) {
        Serial.println("No comms. Check wiring. Is chip supported?");
        return false;
    } else {
        Serial.print("JEDEC ID: 0x");
        Serial.println(JEDEC, HEX);
        Serial.print("Man ID: 0x");
        Serial.println(uint8_t(JEDEC >> 16), HEX);
        Serial.print("Memory ID: 0x");
        Serial.println(uint8_t(JEDEC >> 8), HEX);
        Serial.print("Capacity: ");
        Serial.println(flash.getCapacity());
        Serial.print("Max Pages: ");
        Serial.println(flash.getMaxPage());
    }
    return true;
}

uint8_t buffer[BLOCK_SIZE];
uint8_t buffer2[BLOCK_SIZE];

void setup() {
    Serial.begin(115200);
    while (!Serial)
        ;  // Wait for Serial monitor to open

    delay(50);  // Time to terminal get connected

    Serial.print(F("Initialising"));
    for (uint8_t i = 0; i < 10; ++i) {
        Serial.print(F("."));
    }
    Serial.println();

    // H_SPI_WP (write protect) pin
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);

    // V_SPI_HD (hold) pin
    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);

    bool ret = flash.begin();

    if (flash.error()) {
        Serial.println(flash.error(VERBOSE));
    }

    ret = getID();
    if (!ret) {
        while (true)
            ;
    }
    Serial.println("Erasing...");
    ret = flash.eraseChip();
    if (ret) {
        Serial.println("Erased!");
    } else {
        Serial.println("Error!");
    }

    uint32_t address = 0;
    int cmp = 0;
    int errors = 0;

    // 8 MiB, change this to your flash size
    while (address < (8 * 1024 * 1024)) {
        Serial.println("W");

        for (uint32_t i = 0; i < BLOCK_SIZE; i++) {
            while (!Serial.available())
                ;
            buffer[i] = Serial.read();
        }
        do {
            Serial.println("Writing");
            flash.writeByteArray(address, buffer, BLOCK_SIZE);
            Serial.println("Verifying");
            flash.readByteArray(address, buffer2, BLOCK_SIZE);
            cmp = memcmp(buffer, buffer2, BLOCK_SIZE);
            if (cmp != 0) {
                Serial.println("Verify failed. Retrying.");
                errors++;
            }
            if (errors > 3) {
                Serial.println("Fatal error");
                while (true)
                    ;
            }

        } while (cmp != 0);
        address += BLOCK_SIZE;
    }
}

void loop() {
    Serial.println("Done!");
    delay(1000);
}