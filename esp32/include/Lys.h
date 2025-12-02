/**
 * @file Lys.h
 * @brief Light sensor management for solar tracking
 * @author Yahya
 * 
 * Manages light sensor readings, ADC configuration, and sun direction detection
 * for the dual-axis solar tracking system.
 */

#pragma once

#include <Arduino.h>
#include <driver/adc.h>
#include "DisplayHandler.h"

// ADC Configuration
#define ADC_RESOLUTION ADC_WIDTH_BIT_12
#define ADC_ATTENUATION ADC_ATTEN_DB_12
#define ADC_MAX_VALUE 4095
#define ADC_REFERENCE_VOLTAGE 3.3

class LightSensor {
private:
    int sensorPin;

public:
    /**
     * @brief Constructor
     * @param pin GPIO pin number for the light sensor
     */
    LightSensor(int pin) : sensorPin(pin) {}

    /**
     * @brief Initialize ADC channels for all light sensors
     * Configures 12-bit resolution with 12dB attenuation for 0-3.3V range
     */
    void initLight() {
        adc1_config_width(ADC_RESOLUTION);
        
        // Configure ADC channels with appropriate attenuation
        adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTENUATION);  // GPIO32
        adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTENUATION);  // GPIO33
        adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTENUATION);  // GPIO39
        adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTENUATION);  // GPIO36
        
        Serial.println("ADC channels configured: 12-bit, 12dB attenuation");
    }

    /**
     * @brief Read and display light intensity on TFT
     * @param display DisplayHandler object reference
     * @param x X coordinate on display
     * @param y Y coordinate on display
     */
    void logLightIntensity(DisplayHandler& display, int x, int y) {
        int sensorValue = analogRead(sensorPin);
        float voltage = (sensorValue * ADC_REFERENCE_VOLTAGE) / ADC_MAX_VALUE;

        // Determine sensor position label
        String label;
        if (sensorPin == 32) label = "Left ";
        else if (sensorPin == 33) label = "Right";
        else if (sensorPin == 39) label = "Up   ";
        else if (sensorPin == 36) label = "Down ";

        display.showData(label.c_str(), sensorValue, voltage, x, y);

        // Log to serial for debugging
        if (sensorValue > 3000) {
            Serial.printf("%s sensor: HIGH intensity (%d)\n", label.c_str(), sensorValue);
        } else if (sensorValue < 1000) {
            Serial.printf("%s sensor: LOW intensity (%d)\n", label.c_str(), sensorValue);
        }
    }

    /**
     * @brief Determine sun direction based on sensor values
     * @param left Left sensor value
     * @param right Right sensor value
     * @param up Up sensor value
     * @param down Down sensor value
     * @return Direction string: "Venstre", "Højre", "Op", or "Ned"
     */
    String getSunDirection(int left, int right, int up, int down) {
        int maxIntensity = left;
        String direction = "Venstre";  // Left

        if (right > maxIntensity) {
            maxIntensity = right;
            direction = "Højre";  // Right
        }
        if (up > maxIntensity) {
            maxIntensity = up;
            direction = "Op";  // Up
        }
        if (down > maxIntensity) {
            maxIntensity = down;
            direction = "Ned";  // Down
        }

        Serial.printf("Max intensity direction: %s (%d)\n", direction.c_str(), maxIntensity);
        return direction;
    }

    /**
     * @brief Display sun direction and intensity on TFT (legacy method)
     * @param left Left sensor value
     * @param right Right sensor value
     * @param up Up sensor value
     * @param down Down sensor value
     * @param display DisplayHandler object reference
     */
    void Sunsearch(int left, int right, int up, int down, DisplayHandler& display) {
        String direction = getSunDirection(left, right, up, down);
        int maxIntensity = max(max(left, right), max(up, down));
        display.showDirection(direction, maxIntensity, 10, 100);
    }
};
