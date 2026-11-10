# Ploopy Trackpad on macOS

Firmware that makes the [Ploopy Pavonis Trackpad](https://ploopy.co) usable
on a Mac: a steady, precise pointer, smooth pixel scrolling with momentum,
press-to-click like a MacBook (light touch points, firm press clicks and
drags), tap-to-click, two-finger right-click and three-finger swipes. There's
nothing to install on the Mac. You flash the firmware once and that's it.

> **Status: in testing.** Built and flashed on one trackpad so far. Feedback welcome.

## Why the stock firmware feels so bad on a Mac

The Pavonis is a real multitouch trackpad. On Windows and Linux it talks
Microsoft's **Precision Touchpad** protocol, and the OS handles pointer
acceleration, scrolling and gestures itself.

macOS doesn't support that protocol. It has no public way for a
third-party device to register as a trackpad, so the Pavonis falls back to
pretending to be a plain mouse. In the stock firmware that fallback sends
the sensor's raw, unfiltered movement, which causes:

- a cursor that jitters while your finger is still, and jumps on fast moves;
- scrolling that arrives as chunky "mouse wheel notches" (macOS assumes
  9 counts per inch when a device doesn't declare its scroll resolution);
- no momentum and no useful gestures.

## What this firmware changes

It builds Ploopy's own firmware (QMK, by George Norton) on top of
[qmk-trackpad-macos](https://github.com/vukvukovich/qmk-trackpad-macos)
by Vuk Vukovic. That project rewrites the mouse fallback for macOS. This
repo adds the Pavonis-specific keymap and tuning, plus two engine patches
in [`patches/`](patches):

- `0001-tap-hold-window`: taps may last up to 500 ms without making
  every click wait longer.
- `0002-force-click`: press-to-click and press-to-drag, described below.
- `0003-double-tap`: relaxed double and triple taps register. The stock
  logic required the whole second tap to finish within 150 ms.
- `0004-timer-wrap-fix`: timers in the gesture engine could wrap to
  ~49 days when checked in the same millisecond they started, so "held
  longer than X" checks fired at once.
- `0005-bootloader-hold`: the five-finger reboot needs a 1.5 s hold, so
  a resting hand can't trigger it by accident.
- `0006-scroll-acceleration`: faster finger movement scrolls further and
  coasts longer, like an Apple trackpad.

Every value in the tuning was set from gesture logs recorded on a real
Pavonis, not guessed. The comments in `config.h` give the measurements.

| | Stock | This firmware |
|---|---|---|
| Pointer at rest | jitters | still (1-euro filter + rest anchor) |
| Pointer in motion | twitchy | smooth; no real movement thrown away |
| Sensor glitches | cursor teleports | rejected (median filter, teleport guard, checked I2C reads) |
| Clicking | tap only | firm press clicks, light touch only points; tap-to-click too |
| Selecting / dragging | tap-then-drag | press and drag, like a MacBook |
| Two-finger scroll | chunky wheel notches | continuous pixel scroll, macOS-native |
| Momentum scrolling | none | yes, proportional to how hard you flick |
| Scroll acceleration | none | slow = precise, fast = up to 4× further, like a MacBook |
| Scroll direction | macOS setting | natural, like a MacBook |
| Two-finger tap right-click | needs both fingers to land together | judged at lift, tolerant of staggered fingers |
| Three-finger swipe left/right | — | switch Space |
| Three-finger swipe up/down | — | Mission Control / App Exposé |
| Pinch | — | zoom in/out (Cmd + / Cmd −) |
| Windows / Linux | Precision Touchpad | unchanged, still Precision Touchpad |

### What's not possible

macOS only renders its gesture animations (fluid Space sliding, real
pinch-zoom, Notification Center swipe, Force Touch) for Apple's own
trackpads, and accepts no synthetic gesture events from other devices.
Swipes and pinch therefore trigger the matching keyboard shortcuts. The
result is the same, but the motion doesn't follow your fingers.

Press-to-click is a normal click, not Force Touch: macOS takes pressure
only from Apple's trackpads, so there is no "deep press" (Look Up,
pressure-sensitive drawing).

## Press-to-click

There is no force sensor in the Pavonis. What it can measure is how
strongly each finger registers: a pressed fingertip flattens against the
glass and the signal rises. The firmware learns your light-touch level
from ordinary pointing, so it adapts to your fingers, dry or damp skin,
and grounding. A press that goes clearly above that level while your
finger is nearly still is a click. It works like a Mac trackpad's click
setting:

- **One threshold** decides light vs firm: `DIGITIZER_FORCE_CLICK_PCT`
  (default 28, meaning 28% above your light touch). Lower it for a lighter click.
- **Once pressed, it holds.** Keep the finger down and move to drag or
  select, even at lighter pressure. It lets go when you ease back to a
  light touch or lift.
- **One finger only.** Two fingers landing for a scroll register as
  stronger than one, so two-finger touches never press-click. Use a
  two-finger tap to right-click.
- Right after you plug it in, point around lightly for a few seconds so
  it can learn your light touch.

The Pavonis can be fitted with an optional vibration motor. If yours has
one, uncomment `PAVONIS_HAPTIC_MOTOR` in `config.h` to feel a buzz on
press and release.

## Install

1. Download `ploopyco_pavonis_macos.uf2` from
   [Releases](../../releases), or [build it yourself](#build-it-yourself).
2. Put the trackpad in **bootloader mode**:
   - **Easy way:** rest **five fingers** on the pad and hold them there
     for about 1.5 seconds. It reboots (and buzzes, if the optional motor
     is fitted), and a drive called `RPI-RP2` appears in Finder. (The
     stock firmware reboots the moment it sees five fingers, with no hold.)
   - **If that doesn't work:** unplug it, remove the bootloader cap on the
     back, bridge the two gold holes with a paperclip, and plug it back in
     ([Ploopy's guide](https://ploopyco.github.io/trackpad/appendices/programming/)).
3. Wait a few seconds for Finder to finish mounting the drive, then drag
   the `.uf2` onto `RPI-RP2`. It reboots by itself when it's done. If
   the drive is still there after about 5 seconds, the copy didn't
   take: drag the file onto it again.
4. **System Settings → Mouse**:
   - **Tracking speed:** to taste. The Pavonis shows up as a mouse, not a trackpad.
   - **Scrolling speed:** set it to the middle, which is what the
     scroll tuning assumes, then adjust to taste.
   - **Natural scrolling:** leave it off. The firmware already scrolls in
     natural direction, so turning it on would reverse scrolling again.
5. **Space switching shortcuts:** check that **System Settings → Keyboard
   → Keyboard Shortcuts → Mission Control** has *Move left/right a space*
   (Ctrl ← / Ctrl →), *Mission Control* (Ctrl ↑) and *Application
   windows* (Ctrl ↓) enabled. These are on by default.

### Going back to stock

Use the same five-finger gesture, then drag
[`stock/ploopyco_pavonis_default.uf2`](stock/ploopyco_pavonis_default.uf2)
(Ploopy's shipping firmware) onto `RPI-RP2`.

## Gestures

| Gesture | Does |
|---|---|
| Firm press | left click |
| Firm press, then move | drag / select text |
| One-finger tap | left click |
| Two-finger tap | right click |
| Three-finger tap | middle click |
| Tap, then touch and hold | drag (release to drop) |
| Two-finger drag | scroll; flick for momentum |
| Two-finger pinch | zoom (Cmd + / Cmd −) |
| Three-finger swipe left/right | next/previous Space |
| Three-finger swipe up | Mission Control |
| Three-finger swipe down | App Exposé |
| Five fingers, held 1.5 s | reboot into bootloader (for flashing) |

## Tuning

Everything is in
[`keyboards/ploopyco/pavonis/keymaps/macos/config.h`](keyboards/ploopyco/pavonis/keymaps/macos/config.h),
with a comment on each setting. The ones you're most likely to change:

| Setting | Default | Change it when |
|---|---|---|
| `DIGITIZER_FORCE_CLICK_PCT` | 28 | press-click too hard → lower; light touches click → raise |
| `DIGITIZER_MOUSE_POINTER_SCALE_PCT` | 55 | pointer range too slow/fast even with the macOS slider |
| `ONE_EURO_MINCUTOFF` | 1.0 | cursor wiggles at rest → lower |
| `ONE_EURO_BETA` | 0.01 | cursor lags on fast moves → raise |
| `DIGITIZER_SCROLL_DIVISOR` | 27 | slow scrolling too fast → raise, too slow → lower |
| `DIGITIZER_SCROLL_ACCEL_MAX` | 4 | fast flicks go too far → lower; not far enough → raise (1 = off) |
| `DIGITIZER_COAST_FRICTION_PER_MS` | 3 | momentum runs too long → raise |
| `DIGITIZER_MOUSE_SWIPE_DISTANCE` | 300 | swipes fire by accident → raise |
| `MXT_GAIN` | 11 (Ploopy) | dry fingers miss, or two fingers read as one → raise by 1–2 |

## Build it yourself

You need git and Docker. Nothing else gets installed.

```sh
./build.sh              # -> build/ploopyco_pavonis_macos.uf2
./build.sh macos_debug  # same, with the gesture log on
```

The script clones the firmware fork pinned to a known commit, applies
`patches/`, and builds it
in QMK's official container (`ghcr.io/qmk/qmk_cli`). If you push this repo
to GitHub, the included workflow builds the `.uf2` and publishes it as a
release.

## Troubleshooting

- **Phantom jumps or clicks that come and go:** capacitive sensors are
  sensitive to electrical noise. Ungrounded third-party USB-C (GaN)
  chargers and phones charging nearby are known culprits. After removing
  the source, unplug the trackpad and plug it back in without touching
  it, so the sensor recalibrates.
- **Two close fingers act as one:** spread them a few millimetres apart.
  If that happens a lot, try a higher `MXT_GAIN`.

### Reporting a bug, or tuning for your fingers

Flash the diagnostic build (`./build.sh macos_debug`). It's the same
firmware with the gesture log turned on. Then record a log with
timestamps:

```sh
pip install hidapi
python3 tools/console.py session.log
```

Each touch, tap judgement and press shows up in the log (`FRC press ...
light=..`), so you can see your own light-touch level and press levels.
Switch back to the normal build afterwards, because logging costs a
little smoothness.

## Credits and licence

- [Ploopy](https://github.com/ploopyco/trackpad): the hardware and its
  open-source design.
- [George Norton](https://github.com/george-norton/qmk_firmware): QMK
  digitizer, maXTouch driver and Pavonis board support.
- [Vuk Vukovic](https://github.com/vukvukovich/qmk-trackpad-macos): the
  macOS mouse-fallback engine (filtering, pixel scroll, momentum, gestures).

Firmware is GPL-2.0-or-later, inherited from QMK. The stock firmware in
`stock/` is Ploopy's, redistributed under GPLv3.
