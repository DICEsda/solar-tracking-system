/**
 * @file Wifi_Config.h
 * @brief WiFi configuration and connection management
 * @author Yahya
 * 
 * Handles WiFi initialization, connection, and status display
 */

#pragma once

#include <WiFi.h>
#include <esp_task_wdt.h>
#include "DisplayHandler.h"

// WiFi Connection Timeout (seconds)
#define WIFI_CONNECT_TIMEOUT 30

// Create display handler instance
DisplayHandler display;

/**
 * @brief Initialize WiFi connection with visual feedback
 * @param ssid WiFi network SSID
 * @param password WiFi network password
 * @return true if connection successful, false otherwise
 */
bool HandleWiFi_init(const char* ssid, const char* password) {
    // Add current task to watchdog
    esp_task_wdt_add(NULL);
    
    // Initialize display
    display.initDisplay();
    
    // Start WiFi connection
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    Serial.printf("Connecting to WiFi: %s\n", ssid);
    display.showMessage("Connecting to WiFi...", 10, 20);
    
    int dots = 0;
    int attempts = 0;
    const int maxAttempts = WIFI_CONNECT_TIMEOUT;
    
    // Wait for connection with animated dots
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        String statusMsg = "Status: ";
        
        // Add cycling dots for visual feedback
        for (int i = 0; i < dots; i++) {
            statusMsg += ".";
        }
        
        display.showMessage(statusMsg.c_str(), 10, 50);
        dots = (dots + 1) % 4;
        
        delay(1000);
        attempts++;
        
        // Reset watchdog to prevent timeout
        esp_task_wdt_reset();
        yield();
    }
    
    // Check connection status
    if (WiFi.status() == WL_CONNECTED) {
        display.clear();
        
        // Display success message with IP address
        String ipMessage = "WiFi Connected!\nSSID: " + String(ssid) + 
                          "\nIP: " + WiFi.localIP().toString();
        display.showMessage(ipMessage.c_str(), 10, 10);
        
        Serial.println("\n=== WiFi Connected ===");
        Serial.printf("SSID: %s\n", ssid);
        Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Signal Strength: %d dBm\n", WiFi.RSSI());
        
        delay(3000);  // Show connection info for 3 seconds
        display.clear();
        
        return true;
    } else {
        display.clear();
        display.showMessage("WiFi Failed!\nCheck credentials", 10, 10);
        
        Serial.println("\n=== WiFi Connection Failed ===");
        Serial.printf("SSID: %s\n", ssid);
        Serial.println("Check SSID and password");
        
        return false;
    }
}

/**
 * @brief Check WiFi connection status and reconnect if needed
 * @return true if connected, false otherwise
 */
bool checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi connection lost. Attempting to reconnect...");
        return false;
    }
    return true;
}

/**
 * @brief Get WiFi signal strength description
 * @return String describing signal quality
 */
String getSignalQuality() {
    int rssi = WiFi.RSSI();
    
    if (rssi > -50) return "Excellent";
    else if (rssi > -60) return "Good";
    else if (rssi > -70) return "Fair";
    else if (rssi > -80) return "Weak";
    else return "Very Weak";
}
