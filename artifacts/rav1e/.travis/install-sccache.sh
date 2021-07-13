#!/bin/bash
set -ex

SCCACHE_VERSION="0.2.15"

export RUSTC_WRAPPER=sccache

if [ "$(sccache --version)" = "sccache $SCCACHE_VERSION" ]; then
  echo "Using cached directory."
elif [ "$ARCH" = "x86_64" ]; then
  curl -L "https://github.com/mozilla/sccache/releases/download/v$SCCACHE_VERSION/sccache-v$SCCACHE_VERSION-x86_64-unknown-linux-musl.tar.gz" | tar xz
  mv -f "sccache-v$SCCACHE_VERSION-x86_64-unknown-linux-musl/sccache" "$DEPS_DIR/bin/sccache"
  chmod +x "$DEPS_DIR/bin/sccache"
else
  RUSTC_WRAPPER='' cargo install --version "$SCCACHE_VERSION" --root "$DEPS_DIR" --no-default-features sccache
fi

set +ex
