#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
# Renders icon.svg into the PNG sizes of the icon theme. Needs rsvg-convert (librsvg).
set -eu

assets_dir=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$assets_dir/icons"
for size in 16 32 48 64 128 256 512; do
    rsvg-convert -w "$size" -h "$size" "$assets_dir/icon.svg" -o "$assets_dir/icons/portails-$size.png"
done
cp "$assets_dir/icons/portails-512.png" "$assets_dir/icon.png"
echo "Rendered icon sizes: 16 32 48 64 128 256 512"
