#define BLYNK_TEMPLATE_ID "TMPL3sfWbJiMO"
#define BLYNK_TEMPLATE_NAME "Accident"
#define BLYNK_AUTH_TOKEN "wTCH99LvY3JyZAIcCqax2KA1yGWfmKf9"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Wi-Fi Credentials
const char* ssid = "Shivansh";
const char* password = "12345678";

// Virtual Pin for sending data to Blynk
#define VIRTUAL_PIN V0

void setup() {
    Serial.begin(115200);      // Initialize Serial Monitor
    Serial2.begin(115200, SERIAL_8N1, 16, 17); // RX=16, TX=17 (modify as per ESP32 GPIO connections)
   
    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to Wi-Fi...");
    }
    Serial.println("Connected to Wi-Fi");
   
    // Connect to Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

void loop() {
    Blynk.run(); // Run Blynk
   
    if (Serial2.available()) { // Check if data is available on UART2
        String receivedData = "";
        while (Serial2.available()) {
            char c = Serial2.read();
            receivedData += c;
        }
        Serial.println("Received from STM32: " + receivedData);
        Blynk.virtualWrite(VIRTUAL_PIN, receivedData); // Send data to Blynk Cloud
        
        // Send log event only when data is received
        String logMessage = "ESP32 Alert: Data Received - " + receivedData;
        Blynk.logEvent("log_alert", logMessage.c_str());
    }
}
