/**
 * @file DisplayHandler.h
 * @brief TFT Display management for solar tracking system
 * @author Yahya
 * 
 * Handles all TFT display operations including text rendering,
 * sensor data visualization, and system status display.
 */

#pragma once

#include <TFT_eSPI.h>
#include <Arduino.h>

class DisplayHandler {
private:
    TFT_eSPI tft;

public:
    /**
     * @brief Constructor - initializes TFT object
     */
    DisplayHandler() : tft(TFT_eSPI()) {}

    /**
     * @brief Initialize the display with default settings
     */
    void initDisplay() {
        tft.init();
        tft.setRotation(1);
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(1);
    }

    /**
     * @brief Clear the entire display
     */
    void clear() {
        tft.fillScreen(TFT_BLACK);
    }

    /**
     * @brief Display a text message at specified position
     * @param message Text to display
     * @param x X coordinate
     * @param y Y coordinate
     * @param clearScreen Whether to clear screen before displaying
     */
    void showMessage(const char* message, int x, int y, bool clearScreen = false) {
        if (clearScreen) {
            clear();
        }
        tft.setCursor(x, y);
        tft.println(message);
    }

    /**
     * @brief Display sensor data with label
     * @param label Sensor name/label
     * @param value Raw sensor value
     * @param voltage Converted voltage value
     * @param x X coordinate
     * @param y Y coordinate
     */
    void showData(const char* label, int value, float voltage, int x, int y) {
        String message = String(label) + ": " + String(value) + 
                        " (" + String(voltage, 2) + " V)";
        tft.setCursor(x, y);
        tft.println(message);
    }

    /**
     * @brief Display sun direction and maximum light intensity
     * @param direction Direction string (Left, Right, Up, Down)
     * @param value Maximum light intensity value
     * @param x X coordinate
     * @param y Y coordinate
     */
    void showDirection(const String& direction, int value, int x, int y) {
        tft.setCursor(x, y);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.println("Sun: " + direction);
        
        tft.setCursor(x, y + 10);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.println("Int: " + String(value));
        
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }

    /**
     * @brief Display temperature and humidity readings
     * @param temperature Temperature in Celsius
     * @param humidity Humidity in percentage
     * @param x X coordinate
     * @param y Y coordinate
     */
    void showTempAndHumidity(float temperature, float humidity, int x, int y) {
        tft.setCursor(x, y);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.printf("Temp: %.1f C", temperature);
        
        tft.setCursor(x, y + 10);
        tft.setTextColor(TFT_BLUE, TFT_BLACK);
        tft.printf("Humid: %.1f %%", humidity);
        
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
};
