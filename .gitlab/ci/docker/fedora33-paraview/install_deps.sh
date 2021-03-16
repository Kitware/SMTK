#!/bin/sh

# Install build requirements.
dnf install -y \
    zlib-devel libcurl-devel python-devel python-unversioned-command \
    freeglut-devel glew-devel graphviz-devel libpng-devel mesa-dri-drivers \
    libxcb libxcb-devel libXt-devel xcb-util xcb-util-devel mesa-libGL-devel \
    libxkbcommon-devel diffutils hostname file openssl-devel

# Install development tools
dnf install -y \
    gcc-c++ \
    cmake \
    git-core \
    git-lfs \
    ninja-build \
    make \
    chrpath \
    which

# Install memcheck tools
dnf install -y \
    libasan \
    libubsan \
    valgrind

dnf clean all
