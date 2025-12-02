/**
 * @file Servo-Stepper.c
 * @brief Linux kernel driver for servo and stepper motor control
 * @author Yahya
 * 
 * This driver provides character device interfaces for controlling:
 * - Servo motor (PWM control for tilt adjustment)
 * - Stepper motor (4-phase control for rotation)
 * 
 * Device files created:
 * /dev/plat_drv0 - Servo motor control
 * /dev/plat_drv1-4 - Stepper motor phase control
 */

#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>

#define MAX_DEVICES 5
#define DEVICE_NAME "plat_drv"

// GPIO Pin Definitions
#define SERVO_GPIO 18
#define STEPPER_GPIO_BASE 22

// Servo Configuration
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
#define SERVO_MIN_DUTY 500
#define SERVO_MAX_DUTY 2500
#define SERVO_PERIOD 20000

// Stepper Configuration
#define STEPPER_STEP_DELAY 2

static int servo_gpio = SERVO_GPIO;
static int stepper_gpio_base = STEPPER_GPIO_BASE;

static dev_t devno;
static struct class *gpio_class;
static struct cdev gpio_cdev[MAX_DEVICES];

static int servo_angle = 0;

// Stepper motor 4-phase sequence for smooth rotation
static const int step_sequence[4][4] = {
    {1, 0, 0, 1},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 1}
};

/**
 * @brief Write handler for device files
 * @param filep File pointer
 * @param ubuf User buffer with command data
 * @param count Number of bytes to write
 * @param f_pos File position (unused)
 * @return Number of bytes written, or negative error code
 */
static ssize_t gpio_write(struct file *filep, const char __user *ubuf, size_t count, loff_t *f_pos) {
    char kbuf[32];
    int minor = MINOR(filep->f_inode->i_rdev);
    int value;

    if (count >= sizeof(kbuf)) {
        return -EINVAL;
    }

    if (copy_from_user(kbuf, ubuf, count)) {
        return -EFAULT;
    }
    kbuf[count] = '\0';

    if (minor == 0) {
        // Servo motor control
        if (kstrtoint(kbuf, 10, &value)) {
            pr_err("Invalid servo angle format\n");
            return -EINVAL;
        }

        if (value < SERVO_MIN_ANGLE || value > SERVO_MAX_ANGLE) {
            pr_err("Servo angle out of range (%d-%d)\n", SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
            return -EINVAL;
        }

        // Calculate PWM duty cycle for servo angle
        int duty_cycle = SERVO_MIN_DUTY + 
                        ((value * (SERVO_MAX_DUTY - SERVO_MIN_DUTY)) / SERVO_MAX_ANGLE);

        // Generate PWM pulse
        gpio_set_value(servo_gpio, 1);
        udelay(duty_cycle);
        gpio_set_value(servo_gpio, 0);
        udelay(SERVO_PERIOD - duty_cycle);

        servo_angle = value;
        pr_info("Servo moved to %d degrees\n", value);

    } else if (minor >= 1 && minor <= 4) {
        // Stepper motor individual pin control
        if (kstrtoint(kbuf, 10, &value)) {
            pr_err("Invalid stepper value format\n");
            return -EINVAL;
        }

        if (value != 0 && value != 1) {
            pr_err("Stepper value must be 0 or 1\n");
            return -EINVAL;
        }

        gpio_set_value(stepper_gpio_base + (minor - 1), value);
    } else {
        return -EINVAL;
    }

    return count;
}

/**
 * @brief Read handler for device files
 * @param filep File pointer
 * @param buf User buffer to write data to
 * @param count Number of bytes to read
 * @param f_pos File position
 * @return Number of bytes read, or negative error code
 */
static ssize_t gpio_read(struct file *filep, char __user *buf, size_t count, loff_t *f_pos) {
    char kbuf[64];
    int len;
    int minor = MINOR(filep->f_inode->i_rdev);

    if (*f_pos > 0) {
        return 0;  // EOF
    }

    if (minor == 0) {
        len = snprintf(kbuf, sizeof(kbuf), "Servo angle: %d degrees\n", servo_angle);
    } else if (minor >= 1 && minor <= 4) {
        int value = gpio_get_value(stepper_gpio_base + (minor - 1));
        len = snprintf(kbuf, sizeof(kbuf), "Stepper pin %d: %d\n", minor, value);
    } else {
        return -EINVAL;
    }

    if (len > count) {
        len = count;
    }

    if (copy_to_user(buf, kbuf, len)) {
        return -EFAULT;
    }

    *f_pos += len;
    return len;
}

static const struct file_operations gpio_fops = {
    .owner = THIS_MODULE,
    .write = gpio_write,
    .read = gpio_read,
};

/**
 * @brief Platform driver probe function
 * @param pdev Platform device pointer
 * @return 0 on success, negative error code on failure
 */
static int plat_drv_probe(struct platform_device *pdev) {
    dev_t curr_devno;
    int err;
    int i;

    pr_info("Probing GPIO Driver for Solar Tracking System\n");

    // Allocate character device region
    err = alloc_chrdev_region(&devno, 0, MAX_DEVICES, DEVICE_NAME);
    if (err) {
        pr_err("Failed to allocate chrdev region\n");
        return err;
    }

    // Create device class
    gpio_class = class_create(THIS_MODULE, DEVICE_NAME "_class");
    if (IS_ERR(gpio_class)) {
        unregister_chrdev_region(devno, MAX_DEVICES);
        return PTR_ERR(gpio_class);
    }

    // Create device files
    for (i = 0; i < MAX_DEVICES; i++) {
        curr_devno = MKDEV(MAJOR(devno), i);
        cdev_init(&gpio_cdev[i], &gpio_fops);
        gpio_cdev[i].owner = THIS_MODULE;
        
        err = cdev_add(&gpio_cdev[i], curr_devno, 1);
        if (err) {
            pr_err("Failed to add cdev %d\n", i);
            goto cleanup_cdev;
        }
        
        device_create(gpio_class, NULL, curr_devno, NULL, DEVICE_NAME "%d", i);
        pr_info("Created /dev/" DEVICE_NAME "%d\n", i);
    }

    // Request and configure servo GPIO
    err = gpio_request_one(servo_gpio, GPIOF_OUT_INIT_LOW, "Servo GPIO");
    if (err) {
        pr_err("Failed to request Servo GPIO %d\n", servo_gpio);
        goto cleanup_cdev;
    }

    // Request and configure stepper motor GPIOs
    for (i = 0; i < 4; i++) {
        err = gpio_request_one(stepper_gpio_base + i, GPIOF_OUT_INIT_LOW, 
                              "Stepper GPIO");
        if (err) {
            pr_err("Failed to request Stepper GPIO %d\n", stepper_gpio_base + i);
            goto cleanup_gpio;
        }
    }

    pr_info("GPIO Driver successfully probed\n");
    return 0;

cleanup_gpio:
    // Free already allocated GPIOs
    for (i = i - 1; i >= 0; i--) {
        gpio_free(stepper_gpio_base + i);
    }
    gpio_free(servo_gpio);

cleanup_cdev:
    for (i = i - 1; i >= 0; i--) {
        device_destroy(gpio_class, MKDEV(MAJOR(devno), i));
        cdev_del(&gpio_cdev[i]);
    }
    class_destroy(gpio_class);
    unregister_chrdev_region(devno, MAX_DEVICES);
    return err;
}

/**
 * @brief Platform driver remove function
 * @param pdev Platform device pointer
 * @return 0 on success
 */
static int plat_drv_remove(struct platform_device *pdev) {
    int i;

    pr_info("Removing GPIO Driver\n");

    // Free GPIOs
    gpio_free(servo_gpio);
    for (i = 0; i < 4; i++) {
        gpio_free(stepper_gpio_base + i);
    }

    // Destroy devices
    for (i = 0; i < MAX_DEVICES; i++) {
        device_destroy(gpio_class, MKDEV(MAJOR(devno), i));
        cdev_del(&gpio_cdev[i]);
    }

    class_destroy(gpio_class);
    unregister_chrdev_region(devno, MAX_DEVICES);

    pr_info("GPIO Driver removed successfully\n");
    return 0;
}

// Device tree matching table
static const struct of_device_id plat_drv_of_match[] = {
    { .compatible = "mygpio,plat_drv", },
    { }
};
MODULE_DEVICE_TABLE(of, plat_drv_of_match);

// Platform driver structure
static struct platform_driver plat_drv_driver = {
    .probe = plat_drv_probe,
    .remove = plat_drv_remove,
    .driver = {
        .name = DEVICE_NAME,
        .of_match_table = plat_drv_of_match,
    },
};

module_platform_driver(plat_drv_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yahya");
MODULE_DESCRIPTION("GPIO Driver for Servo and Stepper Motor Control in Solar Tracking System");
MODULE_VERSION("1.0");
