#!/bin/sh

set -e

# Install build requirements.
dnf install -y --setopt=install_weak_deps=False \
    zlib-devel libcurl-devel python-devel python-unversioned-command \
    freeglut-devel glew-devel graphviz-devel libpng-devel mesa-dri-drivers \
    libxcb libxcb-devel libXt-devel xcb-util xcb-util-devel mesa-libGL-devel \
    libxkbcommon-devel diffutils hostname file fontconfig-devel libXext-devel

# Install development tools
dnf install -y --setopt=install_weak_deps=False \
    gcc-c++ \
    qt5-qtbase-devel \
    qt5-qtsvg-devel \
    qt5-qttools-devel \
    qt5-qtx11extras-devel \
    qt5-qtxmlpatterns-devel \
    cmake \
    git-core \
    git-lfs \
    ninja-build \
    make \
    chrpath

# Install static analysis tools
dnf install -y --setopt=install_weak_deps=False \
    clang-tools-extra

# Install memcheck tools
dnf install -y \
    libasan \
    libubsan \
    valgrind

dnf clean all
