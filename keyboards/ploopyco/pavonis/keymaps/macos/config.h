// Copyright 2026 Marvin
// Built on George Norton's Pavonis/maXTouch work and Vuk Vukovic's
// qmk-trackpad-macos engine.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// macOS tuning for the Ploopy Pavonis Trackpad.
//
// macOS never asks for the Microsoft Precision Touchpad protocol, so the
// pad always runs in mouse-fallback mode there. The stock fallback sends
// raw, unfiltered sensor deltas: that is the "way too sensitive and
// jittery" behaviour. This keymap enables the filtered pipeline from the
// qmk-trackpad-macos firmware fork and tunes it for this board.
//
// All distances below are sensor units: 0.01 mm on every maXTouch board.
//
// The board-level config.h is processed first, so anything it defines is
// #undef'd here before being overridden.

#pragma once

/* ---------------------------------------------------------------------
 * Safety net: hold five fingers on the pad for 1.5 s to reboot into the
 * bootloader (patches/0005; stock reboots instantly, which a resting
 * hand triggered by accident). Lets you reflash without opening the
 * bootloader cap.
 * ------------------------------------------------------------------- */
#define MAXTOUCH_BOOTLOADER_GESTURE

/* ---------------------------------------------------------------------
 * Sensor (maXTouch) settings
 * ------------------------------------------------------------------- */

/* The chip's own movement-smoothing filter. The stock driver hard-codes
 * it off. It smooths slow motion hard and tails off at speed, which
 * removes most of the at-rest jitter before the firmware sees it. */
#define MXT_MOVE_SMOOTHING 255

/* Finer motion steps: the default hysteresis pre-quantises slow drags
 * into visible steps. The firmware's 1-euro filter absorbs the extra
 * noise this lets through. */
#undef MXT_MOVE_HYSTERESIS_INITIAL
#define MXT_MOVE_HYSTERESIS_INITIAL 6
#undef MXT_MOVE_HYSTERESIS_NEXT
#define MXT_MOVE_HYSTERESIS_NEXT 4

/* Longer acquisition charge time: on the Procyon this halved phantom
 * contacts caused by USB-C charger noise without lowering amplitudes. */
#undef MXT_CHARGE_TIME
#define MXT_CHARGE_TIME 2

/* Gain, touch threshold and the low-pass filter stay at Ploopy's values
 * from the board config.h. They are tuned for this glass/tape stack.
 * If dry fingers miss taps or two-finger scrolls merge into one finger,
 * raising MXT_GAIN by 1-2 is the first thing to try. */

/* ---------------------------------------------------------------------
 * Pointer
 * ------------------------------------------------------------------- */

/* 1-euro filter. Tuning order:
 *   1) finger resting: LOWER mincutoff until the cursor stops wiggling;
 *   2) fast flicks:    RAISE beta until the cursor stops lagging. */
#define ONE_EURO_MINCUTOFF 1.0f
#define ONE_EURO_BETA 0.01f

/* Teleport guard: a per-report jump larger than this is treated as a
 * sensor glitch and dropped. The engine default (100) was set on a pad
 * reporting every ~25 ms. The Pavonis reports every 3-7 ms over a much
 * larger surface, and a gesture log showed ordinary strokes crossing
 * 100 constantly (p99 342 units/report): 332 real movements dropped in
 * one session, each one stalling the cursor. Single-report spikes are
 * still removed by the engine's 3-sample median filter. */
#define DIGITIZER_MOUSE_TELEPORT_UNITS 1000

/* Output speed as a percentage of filtered finger travel. macOS still
 * applies its own acceleration on top (System Settings > Mouse >
 * Tracking speed), so this is mostly a coarse range setting. */
#define DIGITIZER_MOUSE_POINTER_SCALE_PCT 55

/* ---------------------------------------------------------------------
 * Tapping
 * ------------------------------------------------------------------- */

/* How long a motionless finger may rest on the pad and still count as
 * a tap (needs patches/0001-tap-hold-window.patch). The engine's 150 ms
 * rejected normal taps. Logged one-finger taps on the Pavonis held the
 * pad for 120-210 ms in one session and 380-640 ms in another, partly
 * because the sensor reports a 40-60 ms fade on lift. Any movement
 * beyond the tap distance still turns the touch into pointing at once,
 * so this only decides what a still finger means when it lifts.
 * Click latency after lift stays at the engine's 150 ms. */
#define DIGITIZER_MOUSE_TAP_HOLD_MS 500

/* Two-finger tap window, judged at lift from when the pair formed
 * (engine default 400). A logged 515 ms two-finger tap was missed. */
#define DIGITIZER_MOUSE_TAP_SALVAGE_MS 550

/* ---------------------------------------------------------------------
 * Force click (needs patches/0002-force-click.patch)
 *
 * Like a Mac trackpad: a light touch only points; pressing harder than
 * one threshold clicks, and the click then stays held through a drag or
 * selection, even at lighter pressure, until you ease back to a light
 * touch or lift. One finger only (two-finger TAP = right click). With the optional
 * haptic motor (PAVONIS_HAPTIC_MOTOR below) you also feel a buzz.
 *
 * There is no force sensor: the firmware compares the touch signal,
 * which rises as a pressed fingertip flattens, against your light-touch
 * level, which it learns from ordinary pointing.
 * ------------------------------------------------------------------- */
#define DIGITIZER_FORCE_CLICK

/* THE click force, like Apple's Light / Medium / Firm setting: how far
 * above your light touch (in %) a press must go to click. Lower = lighter
 * click. Chosen by replaying five gesture logs (~130 touches) through a
 * simulation of the detector: 21/21 deliberate presses at 28 and 30, 0
 * false clicks; at 26, light touches started to click. */
#define DIGITIZER_FORCE_CLICK_PCT 28

/* Floor for the learned light-touch level. In use it settles a little
 * below the true light touch (lift-off fades count as light samples):
 * measured 33-35 in a session that felt right. The floor keeps the
 * click threshold from ever sinking into light-touch territory
 * (32 x 1.28 = 41). */
#define DIGITIZER_FORCE_LIGHT_MIN 32.0f

/* Optional vibration motor (not fitted on every Pavonis). Uncomment to
 * get a buzz on force press and release. The motor is a small ERM that
 * needs ~20 ms to spin up, so shorter pulses are barely felt. */
// #define PAVONIS_HAPTIC_MOTOR
#define HAPTIC_PRESS_MS 30
#define HAPTIC_RELEASE_MS 15

/* ---------------------------------------------------------------------
 * Scrolling: smooth, pixel-precise, with momentum
 * ------------------------------------------------------------------- */

/* Declares the wheel's resolution in the HID descriptor. Without this,
 * macOS assumes 9 counts per inch and treats every count as a chunky
 * three-line wheel notch. With it, macOS posts continuous (pixel)
 * scroll events, the same kind an Apple trackpad produces. */
#define WHEEL_RESOLUTION_CPI 200

/* Scroll speed is travel / divisor, paired with the resolution above.
 * Set from logs, as scroll clicks per 100 units of finger travel: the
 * wanted speed is ~3.7. Before patches/0004 a timer bug kept resetting
 * the scroll accumulator mid-gesture and threw most of the travel away,
 * so the divisors tried then (3, 4, 8) were judged against a crippled
 * scroll; with the fix, 8 measured 12.4, so 8 x 12.4 / 3.7 = 27. */
#define DIGITIZER_SCROLL_DIVISOR 27

/* Scroll acceleration (patches/0006), like an Apple trackpad: slow
 * scrolling stays at the rate above, faster finger movement scrolls up
 * to ACCEL_MAX times as far, and since momentum launches from that
 * rate, a fast flick also glides longer. Speeds in sensor units/s:
 * logged scrolling ran at a median of 3500-7300, flicks peaked at
 * 14000-29000. */
#define DIGITIZER_SCROLL_ACCEL_MAX 4.0f
#define DIGITIZER_SCROLL_ACCEL_LO 5000.0f
#define DIGITIZER_SCROLL_ACCEL_HI 30000.0f
#define DIGITIZER_SCROLL_INTERVAL_MS 8

/* Content follows the fingers, like an Apple trackpad. */
#define DIGITIZER_NATURAL_SCROLL

/* ---------------------------------------------------------------------
 * Three-finger swipes, sent as macOS's own keyboard shortcuts.
 * Fingers left shows the Space on the right, as on an Apple trackpad.
 *   left/right: switch Space (Ctrl+Right / Ctrl+Left)
 *   up:         Mission Control (Ctrl+Up)
 *   down:       App Exposé (Ctrl+Down)
 * ------------------------------------------------------------------- */
#define DIGITIZER_SWIPE_LEFT_KC C(KC_RGHT)
#define DIGITIZER_SWIPE_RIGHT_KC C(KC_LEFT)
#define DIGITIZER_SWIPE_UP_KC C(KC_UP)
#define DIGITIZER_SWIPE_DOWN_KC C(KC_DOWN)
/* Swipe length needed to fire (engine default 500). Same value the
 * engine author settled on; units are per-mm identical across maXTouch
 * boards. Raise it if swipes fire by accident. */
#define DIGITIZER_MOUSE_SWIPE_DISTANCE 300

/* Pinch: macOS accepts no synthetic pinch from third-party devices, so
 * zoom is sent as Cmd+= / Cmd+- steps (the engine's defaults). */

/* ---------------------------------------------------------------------
 * Diagnostics. Compiled in but off at boot. See README "Reporting a bug".
 * ------------------------------------------------------------------- */
#define MAXTOUCH_EVENT_TRACE
#define DIGITIZER_MOUSE_GESTURE_TRACE_OFF
