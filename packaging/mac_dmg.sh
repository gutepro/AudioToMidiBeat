#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR=${1:-"$ROOT_DIR/build"}
OUT_DIR=${2:-"$ROOT_DIR/dist"}

mkdir -p "$OUT_DIR"

APP_PATH=$(find "$BUILD_DIR" -name "AudioToMidiBeatApp.app" -type d | head -n 1)
VST3_PATH=$(find "$BUILD_DIR" -name "AudioToMidiBeat.vst3" -type d | head -n 1)

if [[ -z "${APP_PATH:-}" || -z "${VST3_PATH:-}" ]]; then
  echo "Could not find built app or VST3 in $BUILD_DIR" >&2
  exit 1
fi

PKG_DIR=$(mktemp -d)
cp -R "$APP_PATH" "$PKG_DIR/"
cp -R "$VST3_PATH" "$PKG_DIR/"

DMG_PATH="$OUT_DIR/AudioToMidiBeat-macOS.dmg"
hdiutil create -volname "AudioToMidiBeat" -srcfolder "$PKG_DIR" -ov -format UDZO "$DMG_PATH"

rm -rf "$PKG_DIR"
echo "Created $DMG_PATH"
