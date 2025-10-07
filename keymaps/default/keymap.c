#include QMK_KEYBOARD_H

enum layers {
    _BASE,
    _FN,
};

enum custom_keycodes {
    MS_TOG = SAFE_RANGE,  // toggle mouse CPI (precision mode)
    ZOOM_MODE,            // convert vertical movement to Ctrl+wheel
    SCROLL_MODE,          // convert movement to vertical/horizontal wheel
};

static bool mouse_slow = false;
static bool zoom_mode = false;
static bool scroll_mode = false;

// Wheel accumulators so small sensor deltas still produce scroll steps
static int16_t acc_x = 0;
static int16_t acc_y = 0;

// Tunables
static const uint16_t CPI_FAST = 800;  // adjust to taste
static const uint16_t CPI_SLOW = 160;   // ~20% of fast

// How much motion makes one wheel "tick"
static const uint8_t WHEEL_STEP = 8;

// Convert motion into discrete wheel steps with accumulation
static inline int8_t to_wheel_steps(int16_t* acc, int16_t delta) {
    *acc += delta;
    int8_t out = 0;
    while (*acc >= WHEEL_STEP) { out++;  *acc -= WHEEL_STEP; }
    while (*acc <= -WHEEL_STEP){ out--;  *acc += WHEEL_STEP; }
    return out;
}

void keyboard_post_init_user(void) {
    debug_enable = true;
    debug_matrix = true;
}

void pointing_device_init_user(void) {
    // Set initial CPI for the PMW3389 via QMK pointing device API
    pointing_device_set_cpi(CPI_FAST);
}

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {
        case MS_TOG:
            if (record->event.pressed) {
                mouse_slow = !mouse_slow;
                // Use CPI change only — no extra divide so it won't "freeze"
                pointing_device_set_cpi(mouse_slow ? CPI_SLOW : CPI_FAST);
            }
            return false;

        case ZOOM_MODE:
            zoom_mode = record->event.pressed;
            if (!zoom_mode) {
                unregister_code(KC_LCTL);  // ensure released
            }
            return false;

        case SCROLL_MODE:
            scroll_mode = record->event.pressed;
            return false;
    }
    return true;
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
        KC_WBAK, KC_WFWD, KC_F5,   LCTL(KC_A), KC_END,       // 00-04
        MS_BTN1, MS_BTN2, MS_BTN3, KC_DOT,     KC_HOME,      // 10-14
        ZOOM_MODE, SCROLL_MODE,    KC_BSPC,    KC_UP, KC_RGHT, // 20-24
                             KC_LEFT,          KC_DOWN,      // 32-33
        TG(_FN),   KC_ENT,         KC_DEL,     MS_TOG        // 40-43
    ),

    [_FN] = LAYOUT(
        KC_7, KC_8, KC_9, KC_0, KC_END,        // 00-04
        KC_4, KC_5, KC_6, KC_COMM, KC_HOME,    // 10-14
        KC_1, KC_2, KC_3, KC_UP,   KC_RGHT,    // 20-24
                KC_LEFT, KC_DOWN,              // 32-33
        TG(_FN), KC_ENT, KC_DEL, MS_TOG        // 40-43
    )
};

report_mouse_t pointing_device_task_user(report_mouse_t mr) {
    // Default pointer movement when no scroll mode is active
    if (!zoom_mode && !scroll_mode) {
        unregister_code(KC_LCTL);
        acc_x = acc_y = 0;  // reset when not scrolling
        // Note: We do NOT divide x/y here anymore. CPI handles precision.
        return mr;
    }

    // Convert motion to wheel steps using accumulators
    // If your scroll direction feels wrong, flip the signs below.
    int8_t wheel_v = -to_wheel_steps(&acc_y, mr.y); // vertical wheel from Y
    int8_t wheel_h = -to_wheel_steps(&acc_x, mr.x); // horizontal wheel from X

    if (zoom_mode) {
        // Ctrl + vertical wheel = zoom in most apps
        if (wheel_v != 0) {
            register_code(KC_LCTL);
            mr.v = wheel_v;
        } else {
            unregister_code(KC_LCTL);
            mr.v = 0;
        }
        mr.h = 0;
    } else { // scroll_mode
        // Horizontal + vertical scroll
        mr.v = wheel_v;
        mr.h = -wheel_h;
    }

    // Suppress pointer movement while scrolling
    mr.x = 0;
    mr.y = 0;
    return mr;
}
