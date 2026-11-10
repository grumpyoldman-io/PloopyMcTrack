// Copyright 2026 Marvin
// SPDX-License-Identifier: GPL-2.0-or-later
//
// macOS keymap for the Ploopy Pavonis Trackpad. The pad has no keys, so
// the only real code here decides what a three-finger swipe sends.

#include QMK_KEYBOARD_H
#include "digitizer_mouse_fallback.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {{{KC_NO}}};

#if defined(DIGITIZER_FORCE_CLICK) && defined(PAVONIS_HAPTIC_MOTOR)
// Haptic click for force presses, for Pavonis builds fitted with the
// optional vibration motor on HAPTICS_PIN. Pulses are timed from the housekeeping loop rather than
// with wait_ms(), so the pointer never pauses while the motor runs.
static uint32_t haptic_off_at = 0;
static bool     haptic_on     = false;

static void haptic_pulse(uint16_t ms) {
    gpio_write_pin_high(HAPTICS_PIN);
    haptic_on     = true;
    haptic_off_at = timer_read32() + ms;
}

void keyboard_post_init_user(void) {
    gpio_set_pin_output_push_pull(HAPTICS_PIN);
    gpio_write_pin_low(HAPTICS_PIN);
}

void housekeeping_task_user(void) {
    if (haptic_on && (int32_t)(timer_read32() - haptic_off_at) >= 0) {
        gpio_write_pin_low(HAPTICS_PIN);
        haptic_on = false;
    }
}

void digitizer_force_click_changed(bool pressed, uint8_t buttons) {
    haptic_pulse(pressed ? HAPTIC_PRESS_MS : HAPTIC_RELEASE_MS);
}
#endif

// The engine's swipe gestures fire macOS keyboard shortcuts. On Windows
// and Linux the pad switches to the native Precision Touchpad protocol
// and the OS does its own gestures; if it ever falls back to mouse mode
// there, stay quiet, because Ctrl+arrow means "jump by word" on those
// systems rather than "switch desktop".
void digitizer_swipe_action(digitizer_swipe_dir_t dir) {
    switch (detected_host_os()) {
        case OS_WINDOWS:
        case OS_LINUX:
            return;
        default:
            break;
    }
    switch (dir) {
        case DIGITIZER_SWIPE_DIR_RIGHT:
            tap_code16(DIGITIZER_SWIPE_RIGHT_KC);
            break;
        case DIGITIZER_SWIPE_DIR_LEFT:
            tap_code16(DIGITIZER_SWIPE_LEFT_KC);
            break;
        case DIGITIZER_SWIPE_DIR_DOWN:
            tap_code16(DIGITIZER_SWIPE_DOWN_KC);
            break;
        case DIGITIZER_SWIPE_DIR_UP:
            tap_code16(DIGITIZER_SWIPE_UP_KC);
            break;
    }
}
