#!/usr/bin/env bash
#
# Package a macOS 3dfier binary into a self-contained bundle with all its
# shared library dependencies (Homebrew dylibs) and a tarball.
#
# Usage:
#   package_macos.sh <path-to-3dfier-binary> <output-directory>
#
# The resulting archive contains:
#   3dfier-<version>-macos/
#     bin/3dfier
#     libs/*.dylib
#
set -euo pipefail

BIN="${1:?path to 3dfier binary required}"
OUT="${2:?output directory required}"

if ! command -v dylibbundler >/dev/null 2>&1; then
  echo "ERROR: dylibbundler is required. Install it with: brew install dylibbundler" >&2
  exit 1
fi

VERSION="$("$BIN" --version 2>&1 | awk '/^3dfier [0-9]/{print $2}')"
ARCH="$(uname -m)"
BUNDLE_DIR="3dfier-${VERSION}-macos"
BUNDLE="$OUT/$BUNDLE_DIR"

rm -rf "$BUNDLE"
mkdir -p "$BUNDLE/bin" "$BUNDLE/libs"

cp "$BIN" "$BUNDLE/bin/3dfier"

(
  cd "$BUNDLE/bin"
  dylibbundler -od -b -x ./3dfier -d ../libs -p @loader_path/../libs >/dev/null

  # dylibbundler sometimes leaves duplicate LC_RPATH entries in dylibs, which
  # makes dyld refuse to load them. Remove the duplicates and re-sign.
  for f in ../libs/*.dylib; do
    n=$(otool -l "$f" 2>/dev/null | grep -c 'path @loader_path/../libs/' || true)
    if [ "$n" -gt 1 ]; then
      install_name_tool -delete_rpath '@loader_path/../libs/' "$f" >/dev/null 2>&1
      codesign -f -s - "$f" >/dev/null 2>&1
    fi
  done
  codesign -f -s - ./3dfier >/dev/null 2>&1
)

(
  cd "$OUT"
  tar -czf "${BUNDLE_DIR}-${ARCH}.tar.gz" "$BUNDLE_DIR"
)

echo "Packaged $OUT/${BUNDLE_DIR}-${ARCH}.tar.gz"
