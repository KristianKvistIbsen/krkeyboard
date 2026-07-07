#include QMK_KEYBOARD_H

// Allow using MS_BTN1/2/3 tokens in the keymap
#ifndef MS_BTN1
#    define MS_BTN1 KC_BTN1
#    define MS_BTN2 KC_BTN2
#    define MS_BTN3 KC_BTN3
#endif

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

// Accumulators so small sensor deltas still produce steps
static int16_t acc_x = 0;
static int16_t acc_y = 0;

// Tunables
static const uint16_t CPI_FAST = 800;   // adjust to taste
static const uint16_t CPI_SLOW = 160;   // ~20% of fast

// How much motion makes one step (scroll/zoom)
static const uint8_t WHEEL_STEP = 8;    // scroll sensitivity

// Convert motion into discrete steps with accumulation
static inline int8_t to_steps(int16_t* acc, int16_t delta, uint8_t step) {
    *acc += delta;
    int8_t out = 0;
    while (*acc >= step)   { out++;  *acc -= step; }
    while (*acc <= -step)  { out--;  *acc += step; }
    return out;
}

static inline void reset_accumulators(void) {
    acc_x = acc_y = 0;
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
            if (zoom_mode) {
                reset_accumulators();
            } else {
                // make sure Ctrl is released when leaving zoom mode
                unregister_code(KC_LCTL);
            }
            return false;

        case SCROLL_MODE:
            if (record->event.pressed) {
                scroll_mode = true;
                reset_accumulators();
            } else {
                scroll_mode = false;
            }
            return false;
    }
    return true;
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
        KC_WBAK, KC_WFWD, KC_F5,   KC_BSPC, KC_ESC,              // 00-04
        MS_BTN1, MS_BTN2, MS_BTN3, KC_DOT,  KC_TAB,              // 10-14
        ZOOM_MODE, SCROLL_MODE,    LGUI(KC_DOT), KC_UP, KC_RGHT, // 20-24
                             KC_LEFT,          KC_DOWN,          // 32-33
        TG(_FN),   KC_ENT,         KC_DEL,     MS_TOG            // 40-43
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
    // Default pointer movement when no scroll/zoom mode is active
    if (!zoom_mode && !scroll_mode) {
        unregister_code(KC_LCTL);  // ensure Ctrl is not stuck
        reset_accumulators();      // keep things tidy between modes
        return mr;                 // CPI handles precision; no extra divide
    }

    // Convert motion to wheel steps using accumulators
    int8_t wheel_v = -to_steps(&acc_y, mr.y, WHEEL_STEP); // vertical wheel from Y
    int8_t wheel_h = -to_steps(&acc_x, mr.x, WHEEL_STEP); // horizontal wheel from X

    if (zoom_mode) {
        // Ctrl + vertical wheel = zoom in most apps
        if (wheel_v != 0) {
            register_code(KC_LCTL);
            mr.v = wheel_v;  // send vertical wheel
        } else {
            // no vertical scroll this frame; release Ctrl so it doesn't get "stuck"
            unregister_code(KC_LCTL);
            mr.v = 0;
        }
        mr.h = 0;    // no horizontal scroll while zooming
    } else { // scroll_mode
        // Normal scrolling, no modifier
        mr.v = wheel_v;
        mr.h = -wheel_h;  // flip if needed
    }

    // Suppress pointer movement while scrolling/zooming
    mr.x = 0;
    mr.y = 0;
    return mr;
}