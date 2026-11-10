#!/usr/bin/env bash
# Builds the macOS firmware for the Ploopy Pavonis Trackpad inside QMK's
# official build container. Needs only git and Docker.
#
#   ./build.sh [keymap]   -> build/ploopyco_pavonis_<keymap>.uf2  (default: macos)
set -euo pipefail

# Firmware fork with the macOS mouse-fallback engine (qmk-trackpad-macos
# 1.2.1). Pinned to a commit so every build is reproducible.
QMK_REPO="https://github.com/vukvukovich/qmk_firmware.git"
QMK_REF="768c0d1f49"
QMK_BRANCH="qmk-trackpad-macos"
IMAGE="ghcr.io/qmk/qmk_cli:latest"

KEYMAP="${1:-macos}"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
QMK_DIR="${QMK_DIR:-$HERE/.qmk_firmware}"

if [ ! -d "$QMK_DIR/.git" ]; then
    echo "==> Cloning firmware ($QMK_BRANCH)"
    git clone --branch "$QMK_BRANCH" --single-branch "$QMK_REPO" "$QMK_DIR"
fi
git -C "$QMK_DIR" fetch --quiet origin "$QMK_BRANCH" || true
git -C "$QMK_DIR" checkout --quiet --force "$QMK_REF"
git -C "$QMK_DIR" submodule update --init --recursive --depth 1 --quiet

# Small engine fixes for the Pavonis, applied on top of the pinned commit.
# The forced checkout above restores the original files first, so this is
# safe to re-run.
for p in "$HERE"/patches/*.patch; do
    echo "==> Applying $(basename "$p")"
    git -C "$QMK_DIR" apply "$p"
done

mkdir -p "$HERE/build"
echo "==> Building ploopyco/pavonis:$KEYMAP"
docker run --rm \
    -v "$QMK_DIR":/qmk_firmware \
    -v "$HERE":/qmk_userspace \
    -e QMK_USERSPACE=/qmk_userspace \
    -w /qmk_firmware \
    "$IMAGE" \
    make ploopyco/pavonis:$KEYMAP QMK_USERSPACE=/qmk_userspace

# QMK drops the .uf2 in the userspace root when building from a userspace
for f in "$HERE"/ploopyco_pavonis_$KEYMAP.uf2 "$QMK_DIR"/ploopyco_pavonis_$KEYMAP.uf2; do
    [ -f "$f" ] && mv -f "$f" "$HERE/build/"
done
ls -l "$HERE/build/ploopyco_pavonis_$KEYMAP.uf2"
