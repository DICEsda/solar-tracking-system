/**
 * @file main.cpp
 * @brief Main application for IoT-Based Dual-Axis Solar Tracking System
 * @author Yahya
 * 
 * This file contains the main logic for the ESP32-based solar tracking system.
 * It manages sensor readings, web server, display output, and UART communication.
 */

#include <Arduino.h>
#include <Wire.h>
#include <esp_task_wdt.h>
#include <esp_adc_cal.h>
#include "DisplayHandler.h"
#include "Endpoints.h"
#include "HTU.h"
#include "Lys.h"
#include "Wifi_Config.h"

// I2C Configuration
#define SDA_PIN 21
#define SCL_PIN 22

// Light Sensor Pins
#define LIGHT_LEFT_PIN   32
#define LIGHT_RIGHT_PIN  33
#define LIGHT_UP_PIN     39
#define LIGHT_DOWN_PIN   36

// UART Configuration
#define RX_PIN 27
#define TX_PIN 26
#define UART_BAUD 115200

// WiFi Configuration
const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

// Web Server Port
#define WEB_SERVER_PORT 80

// Task Configuration
#define SENSOR_READ_INTERVAL 1000  // milliseconds
#define LIGHT_READ_INTERVAL  1000  // milliseconds

// Global Objects
HTU21D humidity_temperature;
DisplayHandler display;
HardwareSerial RP(1);  // UART1 for Raspberry Pi communication
LightSensor leftSensor(LIGHT_LEFT_PIN);
LightSensor rightSensor(LIGHT_RIGHT_PIN);
LightSensor upSensor(LIGHT_UP_PIN);
LightSensor downSensor(LIGHT_DOWN_PIN);
AsyncWebServer server(WEB_SERVER_PORT);

/**
 * @brief Web server root handler
 */
void handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
}

/**
 * @brief Task for reading temperature and humidity sensors
 * @param pvParameters Task parameters (unused)
 */
void readSensorsTask(void *pvParameters) {
    for (;;) {
        float temperature = humidity_temperature.getTemperature();
        float humidity = humidity_temperature.getHumidity();

        Serial.printf("Temperature: %.2f °C | Humidity: %.2f %%\n", temperature, humidity);

        display.showTempAndHumidity(temperature, humidity, 0, 90);
        
        vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_INTERVAL));
    }
}

/**
 * @brief Initialize all hardware components
 */
void setupHardware() {
    // Initialize Serial
    Serial.begin(115200);
    Serial.println("\n\n=== Solar Tracking System Starting ===");
    
    // Initialize I2C
    Wire.setClock(100000);
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.println("I2C initialized");
    
    // Initialize UART for Raspberry Pi communication
    RP.begin(UART_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
    Serial.println("UART initialized");
    
    // Initialize Light Sensors
    leftSensor.initLight();
    rightSensor.initLight();
    upSensor.initLight();
    downSensor.initLight();
    Serial.println("Light sensors initialized");
}

/**
 * @brief Initialize web server endpoints
 */
void setupWebServer() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/temperature", HTTP_GET, handleTemperature);
    server.on("/humidity", HTTP_GET, handleHumidity);
    server.on("/graph_Temp", HTTP_GET, handleTemperature);
    server.on("/graph_Humidity", HTTP_GET, handleHumidity);
    
    server.begin();
    Serial.println("Web server started");
}

/**
 * @brief Arduino setup function - runs once at startup
 */
void setup() {
    // Initialize Watchdog Timer
    esp_task_wdt_init(5, true);
    
    // Initialize hardware
    setupHardware();
    
    // Initialize WiFi and display
    HandleWiFi_init(WIFI_SSID, WIFI_PASSWORD);
    
    // Create sensor reading task on Core 1
    xTaskCreatePinnedToCore(
        readSensorsTask,
        "SensorReadTask",
        4096,           // Stack size
        NULL,           // Parameters
        1,              // Priority
        NULL,           // Task handle
        1               // Core ID
    );
    
    // Initialize web server
    setupWebServer();
    
    Serial.println("=== Setup Complete ===");
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
}

/**
 * @brief Arduino main loop - runs continuously
 */
void loop() {
    // Read light sensor values
    int leftValue = analogRead(LIGHT_LEFT_PIN);
    int rightValue = analogRead(LIGHT_RIGHT_PIN);
    int upValue = analogRead(LIGHT_UP_PIN);
    int downValue = analogRead(LIGHT_DOWN_PIN);
    
    // Display light intensities on TFT
    leftSensor.logLightIntensity(display, 0, 30);
    rightSensor.logLightIntensity(display, 0, 40);
    upSensor.logLightIntensity(display, 0, 50);
    downSensor.logLightIntensity(display, 0, 60);
    
    // Determine sun direction and send to Raspberry Pi
    String direction = leftSensor.getSunDirection(leftValue, rightValue, upValue, downValue);
    
    // Send direction to Raspberry Pi via UART
    if (RP.availableForWrite()) {
        RP.printf("SUN_DIR:%s\n", direction.c_str());
    }
    
    // Display on local TFT
    int maxValue = max(max(leftValue, rightValue), max(upValue, downValue));
    display.showDirection(direction, maxValue, 10, 100);
    
    // Reset watchdog timer
    esp_task_wdt_reset();
    
    // Delay before next reading
    delay(LIGHT_READ_INTERVAL);
}
