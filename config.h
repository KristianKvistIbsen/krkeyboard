#pragma once

/* Trackball */
#define SPI_DRIVER SPID0
#define SPI_SCK_PIN GP18
#define SPI_MISO_PIN GP20
#define SPI_MOSI_PIN GP19
#define POINTING_DEVICE_CS_PIN GP10
#define ROTATIONAL_TRANSFORM_ANGLE -100 // Optional: Rotates the trackball
// #define POINTING_DEVICE_INVERT_X // Optional: Inverts trackball X
#define POINTING_DEVICE_INVERT_Y // Optional: Inverts trackball X

/* Reset */
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED GP17
