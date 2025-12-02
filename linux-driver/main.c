/**
 * @file main.c
 * @brief User-space application for solar tracking motor control
 * @author Yahya
 * 
 * This application communicates with the ESP32 via UART to receive
 * sun direction commands and controls servo/stepper motors accordingly
 * through the kernel driver interface.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// Device files for servo and stepper motor control
#define SERVO_DEV "/dev/plat_drv0"
#define STEPPER_DEV1 "/dev/plat_drv1"
#define STEPPER_DEV2 "/dev/plat_drv2"
#define STEPPER_DEV3 "/dev/plat_drv3"
#define STEPPER_DEV4 "/dev/plat_drv4"

// Serial port configuration
#define SERIAL_PORT "/dev/ttyS0"
#define BAUD_RATE B115200

// Motor movement parameters
#define SERVO_UP_ANGLE 90
#define SERVO_DOWN_ANGLE 45
#define STEPPER_STEPS 50
#define STEP_DELAY_US 2000

// Stepper motor 4-phase sequence
const int stepSequence[4][4] = {
    {1, 0, 0, 1},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 1}
};

/**
 * @brief Move servo motor to specified angle
 * @param angle Target angle (0-180 degrees)
 * @return 0 on success, -1 on error
 */
int moveServo(int angle) {
    int fd;
    char buffer[16];

    if (angle < 0 || angle > 180) {
        fprintf(stderr, "Error: Servo angle out of range (0-180)\n");
        return -1;
    }

    fd = open(SERVO_DEV, O_WRONLY);
    if (fd < 0) {
        perror("Error opening servo device");
        return -1;
    }

    snprintf(buffer, sizeof(buffer), "%d", angle);
    if (write(fd, buffer, strlen(buffer)) < 0) {
        perror("Error writing to servo device");
        close(fd);
        return -1;
    }

    close(fd);
    printf("Servo moved to %d degrees\n", angle);
    return 0;
}

/**
 * @brief Write value to specific stepper motor pin
 * @param device Device file path
 * @param value Pin value (0 or 1)
 * @return 0 on success, -1 on error
 */
int writeStepperPin(const char *device, int value) {
    int fd;
    char buffer[2];

    fd = open(device, O_WRONLY);
    if (fd < 0) {
        perror("Error opening stepper device");
        return -1;
    }

    snprintf(buffer, sizeof(buffer), "%d", value);
    if (write(fd, buffer, strlen(buffer)) < 0) {
        perror("Error writing to stepper device");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

/**
 * @brief Reset all stepper motor pins to low
 */
void resetStepper(void) {
    writeStepperPin(STEPPER_DEV1, 0);
    writeStepperPin(STEPPER_DEV2, 0);
    writeStepperPin(STEPPER_DEV3, 0);
    writeStepperPin(STEPPER_DEV4, 0);
}

/**
 * @brief Rotate stepper motor
 * @param steps Number of steps to rotate
 * @param clockwise Direction (1 = clockwise, 0 = counter-clockwise)
 * @return 0 on success, -1 on error
 */
int rotateStepper(int steps, int clockwise) {
    for (int i = 0; i < steps; i++) {
        int stepIndex = clockwise ? (i % 4) : (3 - (i % 4));

        writeStepperPin(STEPPER_DEV1, stepSequence[stepIndex][0]);
        writeStepperPin(STEPPER_DEV2, stepSequence[stepIndex][1]);
        writeStepperPin(STEPPER_DEV3, stepSequence[stepIndex][2]);
        writeStepperPin(STEPPER_DEV4, stepSequence[stepIndex][3]);

        usleep(STEP_DELAY_US);
    }

    resetStepper();
    printf("Stepper rotated %d steps %s\n", steps, 
           clockwise ? "clockwise" : "counter-clockwise");
    return 0;
}

/**
 * @brief Parse sun direction command from ESP32
 * @param line Command line from serial input
 * @return Direction string, or NULL if invalid
 */
const char* parseSunDirection(const char *line) {
    static char direction[32];
    
    if (sscanf(line, "SUN_DIR:%31s", direction) == 1) {
        return direction;
    }
    
    return NULL;
}

/**
 * @brief Main control loop
 */
int main(int argc, char *argv[]) {
    FILE *serialInput;
    char line[256];

    printf("=== Solar Tracking Motor Control ===\n");
    printf("Opening serial port: %s\n", SERIAL_PORT);

    // Open serial port for reading ESP32 commands
    serialInput = fopen(SERIAL_PORT, "r");
    if (!serialInput) {
        fprintf(stderr, "Error: Cannot open serial port %s: %s\n", 
                SERIAL_PORT, strerror(errno));
        return 1;
    }

    printf("Listening for sun direction commands...\n");

    // Main control loop
    while (1) {
        if (fgets(line, sizeof(line), serialInput)) {
            // Remove newline
            line[strcspn(line, "\r\n")] = 0;

            const char *direction = parseSunDirection(line);
            if (!direction) {
                continue;  // Invalid command, skip
            }

            printf("\nReceived direction: %s\n", direction);

            // Control motors based on sun direction
            if (strcmp(direction, "Venstre") == 0) {
                printf("Action: Rotate LEFT\n");
                rotateStepper(STEPPER_STEPS, 0);

            } else if (strcmp(direction, "Højre") == 0 || 
                      strcmp(direction, "Hojre") == 0) {
                printf("Action: Rotate RIGHT\n");
                rotateStepper(STEPPER_STEPS, 1);

            } else if (strcmp(direction, "Op") == 0) {
                printf("Action: Tilt UP\n");
                moveServo(SERVO_UP_ANGLE);

            } else if (strcmp(direction, "Ned") == 0) {
                printf("Action: Tilt DOWN\n");
                moveServo(SERVO_DOWN_ANGLE);

            } else {
                printf("Action: Unknown direction, no movement\n");
            }
        }

        usleep(100000);  // 100ms delay between reads
    }

    fclose(serialInput);
    return 0;
}
