#!/bin/sh

# Install build requirements.
dnf install -y \
    openssl-devel

# Install development tools
dnf install -y \
    cargo

dnf clean all

cargo install --root /usr/local --no-default-features --features=redis sccache
