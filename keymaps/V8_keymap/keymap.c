#include QMK_KEYBOARD_H

enum custom_keycodes {
    MS_SCRL = SAFE_RANGE,  // L: hold -> trackball moves become scroll (vertical & horizontal)
    MS_ZOOM,               // M: hold -> trackball moves become ctrl+scroll (zoom)
    MS_SLOW,               // X: tap to toggle trackball movement between normal and ~5% speed
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_WWW_BACK,    KC_WWW_FORWARD, KC_F5,          KC_HOME,      KC_END,
        MS_BTN1,        MS_BTN2,        MS_BTN3,        LGUI(KC_D),   LGUI(KC_TAB),
        KC_MPLY,        MS_ZOOM,        MS_SCRL,        KC_BSPC,      KC_UP,      KC_RGHT,
                                                        KC_LEFT,      KC_DOWN,
        LALT(KC_TAB),   LCTL(KC_V),     LCTL(KC_C),     KC_ENT,       KC_DEL,     MS_SLOW
    )
};

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {
        if (clockwise) {
            tap_code(KC_VOLD);
        } else {
            tap_code(KC_VOLU);
        }
    }
    return false;
}

static bool scroll_mode = false;
static bool zoom_mode   = false;
static bool slow_mode   = false;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case MS_SCRL:
            scroll_mode = record->event.pressed;
            return false;
        case MS_ZOOM:
            zoom_mode = record->event.pressed;
            if (record->event.pressed) {
                register_code(KC_LCTL);
            } else {
                unregister_code(KC_LCTL);
            }
            return false;
        case MS_SLOW:
            if (record->event.pressed) {
                slow_mode = !slow_mode;
            }
            return false;
    }
    return true;
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    // Accumulators store sub-pixel/sub-scroll movements between polling cycles
    static int16_t scroll_accum_v = 0;
    static int16_t scroll_accum_h = 0;
    static int16_t slow_accum_x = 0;
    static int16_t slow_accum_y = 0;

    if (scroll_mode || zoom_mode) {
        scroll_accum_v += mouse_report.y;
        scroll_accum_h += mouse_report.x;

        // Adjust the '8' here to change scroll sensitivity (higher = slower scroll)
        int16_t scroll_steps_v = scroll_accum_v / 8;
        int16_t scroll_steps_h = scroll_accum_h / 8;

        mouse_report.v = -scroll_steps_v;
        mouse_report.h = scroll_steps_h;

        // Subtract the applied steps to keep the fractional remainder for next time
        scroll_accum_v -= scroll_steps_v * 8;
        scroll_accum_h -= scroll_steps_h * 8;

        // Zero out standard cursor movement
        mouse_report.x = 0;
        mouse_report.y = 0;

    } else if (slow_mode) {
        slow_accum_x += mouse_report.x;
        slow_accum_y += mouse_report.y;

        // 5% speed is equal to dividing by 20
        int16_t new_x = slow_accum_x / 20;
        int16_t new_y = slow_accum_y / 20;

        mouse_report.x = new_x;
        mouse_report.y = new_y;

        // Subtract the applied steps to keep the fractional remainder for next time
        slow_accum_x -= new_x * 20;
        slow_accum_y -= new_y * 20;

    } else {
        // Reset accumulators when not using these modes so residual
        // movements don't cause sudden cursor/scroll jumps later.
        scroll_accum_v = 0;
        scroll_accum_h = 0;
        slow_accum_x = 0;
        slow_accum_y = 0;
    }

    return mouse_report;
}
