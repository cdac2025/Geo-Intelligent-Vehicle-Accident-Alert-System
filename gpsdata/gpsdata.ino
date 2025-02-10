#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// GPS module on pins 4 (RX) and 3 (TX)
SoftwareSerial gpsSerial(4, 3);
TinyGPSPlus gps;

void setup() {
  // Initialize serial communication at 9600 baud rate for debugging (on pins 0 and 1)
  Serial.begin(9600); // Serial Monitor and STM32 communication on pins 0 and 1
  
  // Wait for serial to be ready (necessary for some boards)
  while (!Serial) {
    ; // Wait for serial to be ready
  }
  
  Serial.println("GPS Data Logger Ready!");

  // Start communication with the GPS module at 9600 baud
  gpsSerial.begin(9600);
}

void loop() {
  // Continuously read GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
    
    if (gps.location.isUpdated() && gps.altitude.isUpdated()) {
      // Get the latitude, longitude, and altitude data from GPS
      double latitude = gps.location.lat();
      double longitude = gps.location.lng();
      double altitude = gps.altitude.meters();
      
      // Print the data for debugging purposes (on Serial Monitor)
      Serial.print("Latitude: ");
      Serial.print(latitude, 6);
      Serial.print(" Longitude: ");
      Serial.print(longitude, 6);
      Serial.print(" Altitude: ");
      Serial.print(altitude, 2);
      Serial.println(" meters");

      // Send the parsed data to STM32 via Serial (pins 0 and 1)
      sendDataToSTM32(latitude, longitude, altitude);
    }
  }
}

void sendDataToSTM32(double latitude, double longitude, double altitude) {
  // Prepare data to send in CSV format (Latitude,Longitude,Altitude)
  String data = String(latitude, 6) + "," + String(longitude, 6) + "," + String(altitude, 2);
  
  // Send data over Serial (pins 0 and 1) to STM32
  Serial.println(data);
}
