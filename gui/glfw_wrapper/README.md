# glfw wrapper

The code in this directory provides a thin wrapper of the glfw library.
This is convenient because we use glfw in multiple "main files", so the goal is to reduce repetition of glfw library calls.

## CMake

### Targets

* `ben_bot::glfw_wrapper`: wrapper library
