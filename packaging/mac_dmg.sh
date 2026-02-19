#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   mac_dmg.sh <path-to-app> <output-dmg-path> [optional-path-to-vst3]

if [[ $# -lt 2 ]]; then
  echo "Usage: $0 <path-to-app> <output-dmg-path> [optional-path-to-vst3]" >&2
  exit 1
fi

APP_PATH="$1"
OUTPUT_DMG="$2"
VST3_PATH="${3:-}"

if [[ ! -d "$APP_PATH" || "${APP_PATH##*.}" != "app" ]]; then
  echo "Expected a valid .app bundle, got: $APP_PATH" >&2
  exit 1
fi

mkdir -p "$(dirname "$OUTPUT_DMG")"

STAGE_DIR="$(mktemp -d)"
cleanup() {
  rm -rf "$STAGE_DIR"
}
trap cleanup EXIT

cp -R "$APP_PATH" "$STAGE_DIR/"
ln -s /Applications "$STAGE_DIR/Applications"

if [[ -n "$VST3_PATH" && -d "$VST3_PATH" ]]; then
  cp -R "$VST3_PATH" "$STAGE_DIR/"
fi

hdiutil create \
  -volname "AudioToMidiBeat" \
  -srcfolder "$STAGE_DIR" \
  -ov \
  -format UDZO \
  "$OUTPUT_DMG"

echo "Created $OUTPUT_DMG"
