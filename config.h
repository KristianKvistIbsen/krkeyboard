#pragma once

#define EE_HANDS // Store which side I am in EEPROM

/* Keyboard matrix assignments */
#define MATRIX_ROW_PINS { GP29, GP28, GP27, GP26, GP22 }
#define MATRIX_COL_PINS { GP2, GP3, GP4, GP5, GP6}

/* Trackball */
#define SPI_DRIVER SPID0
#define SPI_SCK_PIN GP18
#define SPI_MISO_PIN GP20
#define SPI_MOSI_PIN GP19
#define POINTING_DEVICE_CS_PIN GP10
/* #define ROTATIONAL_TRANSFORM_ANGLE -30 // Optional: Rotates the trackball
/* #define POINTING_DEVICE_INVERT_X // Optional: Inverts trackball X

/* Reset */
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED GP17
// This LED blinks when entering bootloader