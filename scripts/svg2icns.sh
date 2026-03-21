#!/bin/bash

# ======================================================================================
#
# ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
#
# ======================================================================================

# This script generates a MacOS icns file from an SVG.

svgfile="$1"
iconOut="$2"

filename=$(basename -- "$svgfile")
name="${filename%.*}"

echo "Processing $name ..."
set -e

iconset="$(mktemp -d)/$name.iconset"
mkdir -p "$iconset"

# run inkscape from the command line to generate the iconset formatted for icns
inkscape -o "$iconset/icon_16x16.png"      -w   16 -h   16 "$svgfile"
inkscape -o "$iconset/icon_16x16@2x.png"   -w   32 -h   32 "$svgfile"
inkscape -o "$iconset/icon_32x32.png"      -w   32 -h   32 "$svgfile"
inkscape -o "$iconset/icon_32x32@2x.png"   -w   64 -h   64 "$svgfile"
inkscape -o "$iconset/icon_128x128.png"    -w  128 -h  128 "$svgfile"
inkscape -o "$iconset/icon_128x128@2x.png" -w  256 -h  256 "$svgfile"
inkscape -o "$iconset/icon_256x256.png"    -w  256 -h  256 "$svgfile"
inkscape -o "$iconset/icon_256x256@2x.png" -w  512 -h  512 "$svgfile"
inkscape -o "$iconset/icon_512x512.png"    -w  512 -h  512 "$svgfile"
inkscape -o "$iconset/icon_512x512@2x.png" -w 1024 -h 1024 "$svgfile"

# run osx iconutil app to convert the iconset to icns format
iconutil --convert icns --output "$iconOut" "$iconset"

echo "Done."

exit 0
