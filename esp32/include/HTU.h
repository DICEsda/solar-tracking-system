/**
 * @file HTU.h
 * @brief HTU21D Temperature and Humidity sensor interface
 * @author Yahya
 * 
 * Provides interface for reading temperature and humidity data
 * from HTU21D sensor and exposing it via web API
 */

#pragma once

#include <Wire.h>
#include <Arduino.h>
#include <HTU21D.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// I2C Pin Configuration
#define SDA_PIN 21
#define SCL_PIN 22

/**
 * @brief HTU21D Sensor wrapper class
 */
class HTU21D_Sensor {
private:
    HTU21D htu21d;
    bool sensorFound;

public:
    /**
     * @brief Constructor - Initialize I2C and HTU21D sensor
     */
    HTU21D_Sensor() {
        Wire.begin(SDA_PIN, SCL_PIN);
        
        if (!htu21d.begin()) {
            Serial.println("ERROR: HTU21D sensor not detected!");
            sensorFound = false;
        } else {
            Serial.println("HTU21D sensor initialized successfully");
            sensorFound = true;
        }
    }

    /**
     * @brief Read temperature from sensor
     * @return Temperature in Celsius, NAN if error
     */
    float readTemperature() {
        if (sensorFound) {
            float temp = htu21d.getTemperature();
            return temp;
        } else {
            Serial.println("ERROR: HTU21D sensor not available");
            return NAN;
        }
    }

    /**
     * @brief Read humidity from sensor
     * @return Humidity as percentage, NAN if error
     */
    float readHumidity() {
        if (sensorFound) {
            float humidity = htu21d.getHumidity();
            return humidity;
        } else {
            Serial.println("ERROR: HTU21D sensor not available");
            return NAN;
        }
    }

    /**
     * @brief Check if sensor is available
     * @return true if sensor is initialized and responding
     */
    bool isAvailable() {
        return sensorFound;
    }

    /**
     * @brief Get both temperature and humidity in one call
     * @param temp Reference to store temperature value
     * @param humid Reference to store humidity value
     * @return true if successful, false otherwise
     */
    bool readBoth(float& temp, float& humid) {
        if (!sensorFound) {
            return false;
        }
        
        temp = readTemperature();
        humid = readHumidity();
        
        return (!isnan(temp) && !isnan(humid));
    }
};

// Global sensor instance
HTU21D_Sensor sensor;

/**
 * @brief Web handler for temperature endpoint
 */
void handleTemperature(AsyncWebServerRequest *request) {
    float temp = sensor.readTemperature();
    
    if (isnan(temp)) {
        request->send(500, "text/plain", "Sensor Error");
    } else {
        request->send(200, "text/plain", String(temp, 2));
    }
}

/**
 * @brief Web handler for humidity endpoint
 */
void handleHumidity(AsyncWebServerRequest *request) {
    float humidity = sensor.readHumidity();
    
    if (isnan(humidity)) {
        request->send(500, "text/plain", "Sensor Error");
    } else {
        request->send(200, "text/plain", String(humidity, 2));
    }
}

/**
 * @brief Web handler for temperature graph data
 */
void handleGraph_Temp(AsyncWebServerRequest *request) {
    handleTemperature(request);
}

/**
 * @brief Web handler for humidity graph data
 */
void handleGraph_Humidity(AsyncWebServerRequest *request) {
    handleHumidity(request);
}
