# Build Options
BOOTMAGIC_ENABLE = yes
EXTRAKEY_ENABLE  = yes
MOUSEKEY_ENABLE  = yes
CONSOLE_ENABLE   = yes

# Trackball
POINTING_DEVICE_ENABLE = yes
POINTING_DEVICE_DRIVER = pmw3389

# Prefer defining MCU/BOARD in keyboard.json to avoid duplicate warnings.
# If you keep them here, ensure they are NOT also in keyboard.json.
# MCU   = RP2040
# BOARD = GENERIC_RP_RP2040

# Disable keymap introspection to avoid unknown keycode errors
KEYMAP_INTROSPECTION_ENABLE = no
