# CMake toolchains

This directory contains CMake files that are "injected" via presets. The main CMake project itself declares only the information that is absolutely essential to build `ben_bot`; this directory's scripts add features such as warnings, sanitizers, coverage, etc.
