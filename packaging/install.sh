#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
# Installs the built binary, the icon and the .desktop file for the current user.
set -eu

project_root=$(cd "$(dirname "$0")/.." && pwd)
binary="$project_root/build/portails"
data_home="${XDG_DATA_HOME:-$HOME/.local/share}"
binary_dir="$HOME/.local/bin"
applications_dir="$data_home/applications"
icon_root="$data_home/icons/hicolor"

if [ ! -x "$binary" ]; then
    echo "Binary not found. Run: cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build" >&2
    exit 1
fi

mkdir -p "$binary_dir" "$applications_dir" "$icon_root/scalable/apps"
install -m 755 "$binary" "$binary_dir/portails"
for size in 16 32 48 64 128 256 512; do
    mkdir -p "$icon_root/${size}x${size}/apps"
    install -m 644 "$project_root/assets/icons/portails-$size.png" "$icon_root/${size}x${size}/apps/portails.png"
done
install -m 644 "$project_root/assets/icon.svg" "$icon_root/scalable/apps/portails.svg"
sed "s|^Exec=.*|Exec=$binary_dir/portails|" "$project_root/packaging/portails.desktop" > "$applications_dir/portails.desktop"
update-desktop-database "$applications_dir" 2>/dev/null || true

echo "Installed: $binary_dir/portails"
echo "Installed: $applications_dir/portails.desktop"
echo "Installed: icons in $icon_root (16 to 512 and scalable)"
