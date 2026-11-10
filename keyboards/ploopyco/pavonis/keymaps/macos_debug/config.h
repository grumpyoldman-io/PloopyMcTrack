// SPDX-License-Identifier: GPL-2.0-or-later
// Diagnostic variant of the macos keymap: identical tuning, gesture log
// ON at boot. Flash this only while capturing a log (tools/console.py);
// console writes on every sensor report cost some pointer smoothness.
#pragma once
#include "../macos/config.h"
#undef DIGITIZER_MOUSE_GESTURE_TRACE_OFF
